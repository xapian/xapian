/** @file xslxparse.cc
 * @brief Extract fields from XLSX sheet*.xml.
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

#include "xlsxparse.h"

using namespace std;

bool
XlsxParser::opening_tag(const string &tag)
{
    if (tag == "c") {
	// Skip <v> tags which are inside <c t="s">, as these are numeric
	// references to shared strings, which we index separately.
	string type;
	index_content = (!get_parameter("t", type) || type != "s");
    } else if (tag == "v") {
	in_v = index_content;
    }
    return true;
}

bool
XlsxParser::closing_tag(const string &tag)
{
    if (tag == "v") {
	in_v = false;
    }
    return true;
}

void
XlsxParser::process_text(const string &text)
{
    if (in_v && !text.empty()) {
	if (!dump.empty()) dump += ' ';
	dump += text;
    }
}
