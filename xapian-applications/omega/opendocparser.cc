/** @file
 * @brief Extract text from OpenDocument XML.
 */
/* Copyright (C) 2012-2022 Olly Betts
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

#include "opendocparser.h"

#include <cstring>

#include "stringutils.h"

using namespace std;

bool
OpenDocParser::opening_tag(const string& tag)
{
    if (startswith(tag, "text:")) {
	const char* tail = tag.c_str() + 5;
	if (strcmp(tail, "p") == 0 ||
	    strcmp(tail, "h") == 0 ||
	    strcmp(tail, "line-break") == 0 ||
	    strcmp(tail, "tab") == 0) {
	    pending_space = true;
	}
    } else if (tag == "office:body") {
	indexing = true;
    } else if (tag == "style:style") {
	(void)get_attribute("style:master-page-name", master_page_name);
    } else if (tag == "style:master-page") {
	string n;
	if (get_attribute("style:name", n) && n == master_page_name)
	    indexing = true;
    }
    return true;
}

bool
OpenDocParser::closing_tag(const string& tag)
{
    if (!indexing)
	return true;

    if (tag == "text:p" || tag == "text:h") {
	pending_space = true;
    } else if (tag == "office:body" || tag == "style:style") {
	indexing = false;
    }
    return true;
}

void
OpenDocParser::process_content(const string& content)
{
    if (indexing && !content.empty()) {
	if (pending_space) {
	    pending_space = false;
	    if (!content.empty()) dump += ' ';
	}
	dump += content;
    }
}
