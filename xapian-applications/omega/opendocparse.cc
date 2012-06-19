/** @file opendocparse.cc
 * @brief Extract text from XML from an OpenDocument document.
 */
/* Copyright (C) 2012 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "opendocparse.h"

using namespace std;

bool
OpenDocParser::opening_tag(const string &tag)
{
    if (tag == "office:body") {
	indexing = true;
    } else if (tag == "style:style") {
	(void)get_parameter("style:master-page-name", master_page_name);
    } else if (tag == "style:master-page") {
	string n;
	if (get_parameter("style:name", n) && n == master_page_name)
	    indexing = true;
    }
    return true;
}

bool
OpenDocParser::closing_tag(const string &tag)
{
    if (!indexing)
	return true;

    if (tag == "text:p") {
	pending_space = true;
    } else if (tag == "office:body" || tag == "style:style") {
	indexing = false;
    }
    return true;
}

void
OpenDocParser::process_text(const string &text)
{
    if (indexing && !text.empty()) {
	if (pending_space) {
	    pending_space = false;
	    dump += ' ';
	}
	dump += text;
    }
}
