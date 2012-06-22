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

#include <cstdlib>

using namespace std;

bool
XlsxParser::opening_tag(const string &tag)
{
    if (tag == "c") {
	// We need to distinguish <v> tags which are inside <c t="s">, as these
	// are numeric references to shared strings.
	string type;
	if (get_parameter("t", type) && type == "s") {
	    mode = MODE_C_STRING;
	} else {
	    mode = MODE_C_LITERAL;
	}
    } else if (tag == "v") {
	if (mode == MODE_C_LITERAL) {
	    mode = MODE_V_LITERAL;
	} else if (mode == MODE_C_STRING) {
	    mode = MODE_V_STRING;
	}
    } else if (tag == "si") {
	mode = MODE_SI;
    } else if (tag == "sst") {
	string unique_count;
	if (get_parameter("uniquecount", unique_count)) {
	    unsigned long c = strtoul(unique_count.c_str(), NULL, 10);
	    // This reserving is just a performance tweak, so don't go reserving
	    // ludicrous amounts of space just because an XML attribute told us to.
	    sst.reserve(std::max(c, 1000000ul));
	}
    }
    return true;
}

void
XlsxParser::process_text(const string &text)
{
    switch (mode) {
	case MODE_V_STRING: {
	    // Shared string use.
	    unsigned long c = strtoul(text.c_str(), NULL, 10);
	    if (c < sst.size())
		append_field(sst[c]);
	    mode = MODE_NONE;
	    return;
	}
	case MODE_V_LITERAL:
	    // Literal (possibly calculated) field value.
	    append_field(text);
	    mode = MODE_NONE;
	    return;
	case MODE_SI:
	    // Shared string definition.
	    sst.push_back(text);
	    mode = MODE_NONE;
	    return;
	default:
	    return;
    }
}
