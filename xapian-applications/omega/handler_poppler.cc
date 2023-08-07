/** @file
 * @brief Extract text and metadata using poppler.
 */
/* Copyright (C) 2019 Bruno Baruffaldi
 * Copyright (C) 2022,2023 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>
#include "handler.h"
#include "str.h"

#include <poppler/glib/poppler-document.h>
#include <poppler/glib/poppler-page.h>

using namespace std;

static gchar*
convert_to_uri(const string& filename, GError** e)
{
#if GLIB_CHECK_VERSION(2,58,0)
    gchar* abs_filename = g_canonicalize_filename(filename.c_str(), NULL);
#else
    gchar* abs_filename;
    if (g_path_is_absolute(filename.c_str())) {
	abs_filename = g_strdup(filename.c_str());
    } else {
	gchar* cwd = g_get_current_dir();
	abs_filename = g_build_filename(cwd, filename.c_str(), NULL);
	g_free(cwd);
    }
#endif
    gchar* uri = g_filename_to_uri(abs_filename, NULL, e);
    g_free(abs_filename);
    return uri;
}

static void
send_glib_field(Field field, gchar* data)
{
    if (data) {
	send_field(field, data);
	g_free(data);
    }
}

bool
initialise()
{
    return true;
}

void
extract(const string& filename, const string&)
{
    GError* e;
    gchar* uri = convert_to_uri(filename, &e);
    if (!uri) {
	send_field(FIELD_ERROR, "g_filename_to_uri() failed: ");
	send_field(FIELD_ERROR, e->message);
	g_error_free(e);
	return;
    }

    PopplerDocument* doc = poppler_document_new_from_file(uri, NULL, &e);
    g_free(uri);
    if (!doc) {
	send_field(FIELD_ERROR, "poppler_document_new_from_file() failed: ");
	send_field(FIELD_ERROR, e->message);
	g_error_free(e);
	return;
    }

    int pages = poppler_document_get_n_pages(doc);
    send_field_page_count(pages);
    // Extracting text from PDF file
    for (int i = 0; i < pages; ++i) {
	PopplerPage* page = poppler_document_get_page(doc, i);
	if (!page) {
	    g_object_unref(doc);
	    send_field(FIELD_ERROR, "Failed to get page " + str(i));
	    return;
	}
	send_field(FIELD_BODY, poppler_page_get_text(page));
	g_object_unref(page);
    }

    // Extract PDF metadata.
    send_glib_field(FIELD_AUTHOR, poppler_document_get_author(doc));
    send_glib_field(FIELD_TITLE, poppler_document_get_title(doc));
    send_glib_field(FIELD_KEYWORDS, poppler_document_get_keywords(doc));
    send_field_created_date(poppler_document_get_creation_date(doc));

    g_object_unref(doc);
}
