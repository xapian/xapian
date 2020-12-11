/** @file
 * @brief Extract text from an RSS atom file.
 */
/* Copyright (C) 2010,2011,2012,2020 Olly Betts
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

#include "atomparser.h"
#include "htmlparser.h"
#include "stringutils.h"

using namespace std;

void
AtomParser::process_content(const string& content)
{
    if (!target)
	return;

    if (!target->empty())
	*target += ' ';

    if (html_content) {
	HtmlParser p;
	p.parse(content, charset, true);
	*target += p.dump;
    } else {
	*target += content;
    }
}

bool
AtomParser::opening_tag(const string& tag)
{
    if (state == INACTIVE) {
	if (tag == "title") {
	    if (in_entry) {
		// Treat <title> inside <entry> as more keywords.
		target = &keywords;
	    } else {
		target = &title;
	    }
	    goto check_type_attribute;
	} else if (tag == "summary" || tag == "subtitle" || tag == "content") {
	    target = &dump;
check_type_attribute:
	    string type;
	    html_content = (get_attribute("type", type) &&
			    (type == "html" || type == "xhtml"));
	    state = OTHER;
	} else if (tag == "author") {
	    target = &author;
	    html_content = false;
	    state = AUTHOR;
	} else if (tag == "entry") {
	    in_entry = true;
	} else if (tag == "category") {
	    // Handle category term separately.
	    string new_keyword;
	    get_attribute("term", new_keyword);
	    if (!keywords.empty())
		keywords += ' ';
	    keywords += new_keyword;
	}
	if (state != INACTIVE) {
	    active_tag = tag;
	}
    } else if (state == AUTHOR) {
	if (tag == "uri")
	    target = NULL;
    }

    return true;
}

bool
AtomParser::closing_tag(const string& tag)
{
    if (state != INACTIVE && tag == active_tag) {
	active_tag = string();
	state = INACTIVE;
	target = NULL;
    } else if (in_entry && tag == "entry") {
	in_entry = false;
    } else if (state == AUTHOR && tag == "uri") {
	target = &author;
    }
    return true;
}
