/** @file atomparse.cc
 * @brief Extract text from an RSS atom file.
 */
/* Copyright (C) 2010,2011,2012 Olly Betts
 * Copyright (C) 2012 Mihai Bivol
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

#include "atomparse.h"
#include "myhtmlparse.h"
#include "stringutils.h"

using namespace std;

void
AtomParser::process_text(const string &text)
{
    if (is_ignored)
	return;

    string * target = NULL;

    switch (state) {
	case TEXT:
	    target = &dump;
	    break;
	case TITLE:
	    target = &title;
	    break;
	case KEYWORDS:
	    target = &keywords;
	    break;
	case AUTHOR:
	    target = &author;
	    break;
	case OTHER:
	    // Ignore context in other places.
	    return;
    }

    if (!target->empty())
	*target += ' ';

    if (type == "html") {
	MyHtmlParser p;
	p.parse_html(text, charset, true);
	*target += p.dump;
    } else {
	*target += text;
    }
}

bool
AtomParser::opening_tag(const string &tag)
{
    if (state == OTHER) {
	if (tag == "title")
	    state = in_entry ? KEYWORDS : TITLE;
	else if (tag == "summary" || tag == "subtitle" || tag == "content")
	    state = TEXT;
	else if (tag == "author")
	    state = AUTHOR;
	else if (tag == "entry")
	    in_entry = true;
	else if (tag == "category") {
	    // Handle category term separately.
	    string new_keyword;
	    get_parameter("term", new_keyword);
	    if (!keywords.empty())
		keywords += ' ';
	    keywords += new_keyword;
	}
    } else if (state == AUTHOR) {
	if (tag == "uri")
	    is_ignored = true;
    }

    if (!get_parameter("type", type))
	type = "text";
    return true;
}

bool
AtomParser::closing_tag(const string &tag)
{
    if (tag == "entry")
	in_entry = false;
    else if (tag == "uri")
	is_ignored = false;
    else if (tag == "title" || tag == "summary" || tag == "subtitle" ||
	     tag == "author" || tag == "content")
	state = OTHER;
    return true;
}
