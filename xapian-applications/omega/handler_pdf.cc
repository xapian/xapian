/** @file handler_pdf.cc
 * @brief @brief Extract text and metadata using poopler.
 */
/* Copyright (C) 2011 Olly Betts
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
#include "handler_pdf.h"

#include <glib.h>
#include <glib-object.h>
#include <iostream>
#include <string.h>
#include <xapian.h>

#include <poppler/glib/poppler-document.h>
#include <poppler/glib/poppler-page.h>

using namespace std;

static string
clear_text(const gchar * x)
{
    if (!x) return string("");
    int i, sz = strlen(x);
    string text;
    text.reserve(sz);

    for (i = 0; i < sz; ++i)
	if (!isspace(x[i]) || (i && !isspace(x[i - 1])))
	    text.push_back(x[i]);

    return text;
}

bool
extract(const string & filename,
	string & dump,
	string & title,
	string & keywords,
	string & author,
	string & pages)
{

    try {
	gchar * uri = g_filename_to_uri(filename.c_str(), NULL, NULL);
	PopplerDocument * doc = poppler_document_new_from_file(uri, NULL, NULL);
	gchar * cstr;

	if (!doc) {
	    cerr << "Poppler Error: Failed to read pdf file" << endl;
	    return false;
	}
	g_free(uri);

	// Extracting PDF metadata
	int npages = poppler_document_get_n_pages(doc);
	if (0 < npages) {
	    pages = to_string(npages);
	}

	cstr = poppler_document_get_author(doc);
	if (cstr) {
	    author = clear_text(cstr);
	    g_free(cstr);
	}

	cstr = poppler_document_get_title(doc);
	if (cstr) {
	    title = clear_text(cstr);
	    g_free(cstr);
	}

	cstr = poppler_document_get_keywords(doc);
	if (cstr) {
	    keywords = clear_text(cstr);
	    g_free(cstr);
	}

	// Extracting text from PDF file
	for (int i = 0; i < npages; ++i) {
	    PopplerPage * pg = poppler_document_get_page(doc, i);
	    if (!pg) {
		cerr << "Poppler Error: Failed to create page" << endl;
		return false;
	    }
	    cstr = poppler_page_get_text(pg);
	    if (cstr) {
		dump += clear_text(cstr);
		g_free(cstr);
	    }
	    g_object_unref(pg);
	}
	g_object_unref(doc);
    } catch (...) {
	cerr << "Poppler threw an exception" << endl;
	return false;
    }

    return true;
}
