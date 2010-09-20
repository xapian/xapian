/** @file svgparse.cc
 * @brief Extract text from an SVG file.
 */
/* Copyright (C) 2010 Olly Betts
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

#include "svgparse.h"

void
SvgParser::process_text(const string &text)
{
    if (in_text) {
	if (!dump.empty())
	    dump += ' ';
	dump += text;
    }
}

void
SvgParser::opening_tag(const string &tag)
{
    if (tag == "text")
	in_text = true;
    // FIXME: handle <metadata>
}

void
SvgParser::closing_tag(const string &tag)
{
    if (tag == "text")
	in_text = false;
}
