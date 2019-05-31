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

#include <xapian.h>
#include "handler_pdf.h"
#include <iostream>
#include <poppler-document.h>
#include <poppler-page.h>

using namespace std;
using namespace poppler;

static string
clear_text(const ustring & x)
{
    byte_array buf = x.to_utf8();
    string text, tmp(buf.data(), buf.size());
    int sz = tmp.length();

    for (int i = 0; i < sz; ++i)
	if (!isspace(tmp[i]) || (i && !isspace(tmp[i - 1])))
	    text.push_back(tmp[i]);

    return text;
}

bool
extract(const string & filename,
	string & dump,
	string & title,
	string & keywords,
	string & author)
{

    try {

	document * doc = document::load_from_file(filename, "", "");

	if (!doc) {
	    cerr << "Poppler Error: Failed to read pdf file" << endl;
	    return false;
	}

	author = clear_text(doc->get_author());
	title = clear_text(doc->get_title());
	keywords = clear_text(doc->get_keywords());

	// Extracting text from PDF file
	for (int i = 0; i < doc->pages(); ++i) {
	    page *p(doc->create_page(i));
	    if (!p) {
		cerr << "Poppler Error: Failed to create page" << endl;
		return false;
	    }
	    dump += clear_text(p->text());
	}
    } catch (...) {
	cerr << "Poppler threw an exception" << endl;
	return false;
    }

    return true;
}
