/** @file handler_poppler.cc
 * @brief Extract text and metadata using poppler.
 */
/* Copyright (C) 2019 Bruno Baruffaldi
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

#include <iostream>

#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>

using namespace std;
using namespace poppler;

static string
text_to_utf8(const ustring& x)
{
    byte_array buf = x.to_utf8();
    string text(buf.data(), buf.size());
    return text;
}

static void
clear_text(string& text)
{
    int i, j, sz = text.length();
    for (j = i = 0; i < sz; ++i)
	if (!isspace(text[i]) || (i && !isspace(text[i - 1])))
	    text[j++] = text[i];
    text.resize(j);
}

bool
extract(const string& filename,
	string& dump,
	string& title,
	string& keywords,
	string& author,
	string& pages)
{
    try {
	document * doc = document::load_from_file(filename);

	if (!doc) {
	    cerr << "Poppler Error: Failed to read pdf file" << endl;
	    return false;
	}

	// Extracting PDF metadata
	author = text_to_utf8(doc->info_key("Author"));
	title = text_to_utf8(doc->info_key("Title"));
	keywords = text_to_utf8(doc->info_key("Keywords"));
	int npages = doc->pages();
	pages = str(npages);
	// Extracting text from PDF file
	for (int i = 0; i < npages; ++i) {
	    page *p(doc->create_page(i));
	    if (!p) {
		cerr << "Poppler Error: Failed to create page " << i << endl;
		return false;
	    }
	    dump += text_to_utf8(p->text());
	}
	clear_text(dump);
    } catch (...) {
	cerr << "Poppler threw an exception" << endl;
	return false;
    }

    return true;
}
