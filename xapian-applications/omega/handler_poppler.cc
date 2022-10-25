/** @file
 * @brief Extract text and metadata using poppler.
 */
/* Copyright (C) 2019 Bruno Baruffaldi
 * Copyright (C) 2022 Olly Betts
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

static inline size_t
strlen_if_nonnull(const gchar* s)
{
    return s ? strlen(s) : 0;
}

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

void
extract(const string& filename,
	const string& mimetype)
{
    GError* e;
    gchar* uri = convert_to_uri(filename, &e);
    if (!uri) {
	string error = "Poppler Error: g_filename_to_uri() failed: ";
	error += e->message;
	g_error_free(e);
	fail(error);
	return;
    }

    PopplerDocument* doc = poppler_document_new_from_file(uri, NULL, &e);
    g_free(uri);
    if (!doc) {
	string error = "Poppler Error: Failed to read pdf file: ";
	error += e->message;
	g_error_free(e);
	fail(error);
	return;
    }

    try {
	string dump;
	int pages = poppler_document_get_n_pages(doc);
	// Extracting text from PDF file
	for (int i = 0; i < pages; ++i) {
	    PopplerPage* page = poppler_document_get_page(doc, i);
	    if (!page) {
		g_object_unref(doc);
		fail("Poppler Error: Failed to create page " + str(i));
		return;
	    }
	    dump += poppler_page_get_text(page);
	    g_object_unref(page);
	}

	// Extract PDF metadata.
	gchar* author = poppler_document_get_author(doc);
	gchar* title = poppler_document_get_title(doc);
	gchar* keywords = poppler_document_get_keywords(doc);
	time_t created = poppler_document_get_creation_date(doc);

	response(dump.data(), dump.size(),
		 title, strlen_if_nonnull(title),
		 nullptr, 0,
		 author, strlen_if_nonnull(author),
		 pages,
		 created);
	g_free(author);
	g_free(title);
	g_free(keywords);
	g_object_unref(doc);
    } catch (...) {
	fail("Poppler threw an exception");
    }
}
