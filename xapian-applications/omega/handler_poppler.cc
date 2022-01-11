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

static inline void
assign_if_nonnull(string& v, gchar* s)
{
    if (s) v = s;
}

bool
extract(const string& filename,
	const string& mimetype,
	string& dump,
	string& title,
	string& keywords,
	string& author,
	string& pages,
	string& error)
{
    try {
	GError* e;
	gchar* abs_filename = g_canonicalize_filename(filename.c_str(), NULL);
	gchar* uri = g_filename_to_uri(abs_filename, NULL, &e);
	g_free(abs_filename);
	if (!uri) {
	    error = "Poppler Error: g_filename_to_uri() failed: ";
	    error += e->message;
	    g_error_free(e);
	    return false;
	}

	PopplerDocument* doc = poppler_document_new_from_file(uri, NULL, &e);
	g_free(uri);
	if (!doc) {
	    error = "Poppler Error: Failed to read pdf file: ";
	    error += e->message;
	    g_error_free(e);
	    return false;
	}

	// Extracting PDF metadata
	assign_if_nonnull(author, poppler_document_get_author(doc));
	assign_if_nonnull(title, poppler_document_get_title(doc));
	assign_if_nonnull(keywords, poppler_document_get_keywords(doc));
	time_t datetime = poppler_document_get_creation_date(doc);
	if (datetime != time_t(-1)) {
	    // FIXME: Add support for this to the worker protocol.
	}
	int npages = poppler_document_get_n_pages(doc);
	pages = str(npages);
	// Extracting text from PDF file
	for (int i = 0; i < npages; ++i) {
	    PopplerPage* page = poppler_document_get_page(doc, i);
	    if (!page) {
		error = "Poppler Error: Failed to create page " + str(i);
		g_object_unref(page);
		g_object_unref(doc);
		return false;
	    }
	    dump += poppler_page_get_text(page);
	    g_object_unref(page);
	}
	g_object_unref(doc);
    } catch (...) {
	error = "Poppler threw an exception";
	return false;
    }

    return true;
}
