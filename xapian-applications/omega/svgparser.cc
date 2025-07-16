/** @file
 * @brief Extract text from an SVG file.
 */
/* Copyright (C) 2010-2022 Olly Betts
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

#include "svgparser.h"
#include "stringutils.h"

using namespace std;

void
SvgParser::process_content(const string& content)
{
    string * target = NULL;
    switch (state) {
	case TEXT:
	    target = &dump;
	    break;
	case TITLE:
	    target = &title;
	    title.resize(0);
	    break;
	case DC_TITLE:
	    // Prefer <title> to <dc:title>.
	    if (!title.empty()) return;
	    target = &title;
	    break;
	case KEYWORDS:
	    target = &keywords;
	    break;
	case AUTHOR:
	    target = &author;
	    break;
	case METADATA: case OTHER:
	    // Ignore context in other places.
	    return;
    }
    if (!target->empty())
	*target += ' ';
    *target += content;
}

bool
SvgParser::opening_tag(const string& tag)
{
    switch (state) {
	case OTHER:
	    if (tag == "text" || tag == "svg:text")
		state = TEXT;
	    else if (tag == "metadata" || tag == "svg:metadata")
		state = METADATA;
	    else if (tag == "title")
		state = TITLE;
	    break;
	case METADATA:
	    // Ignore nested "dc:" tags - for example dc:title is also used to
	    // specify the creator's name inside dc:creator.
	    if (dc_tag.empty() && startswith(tag, "dc:")) {
		dc_tag = tag;
		if (tag == "dc:title")
		    state = DC_TITLE;
		else if (tag == "dc:subject")
		    state = KEYWORDS;
		else if (tag == "dc:creator")
		    state = AUTHOR;
	    }
	    break;
	case DC_TITLE: case KEYWORDS: case TEXT: case TITLE: case AUTHOR:
	    // Avoid compiler warnings.
	    break;
    }
    return true;
}

bool
SvgParser::closing_tag(const string& tag)
{
    if (tag == "text" || tag == "svg:text" ||
	tag == "title" ||
	tag == "metadata" || tag == "svg:metadata") {
	state = OTHER;
    } else if (tag == dc_tag) {
	dc_tag.resize(0);
	state = METADATA;
    }
    return true;
}
