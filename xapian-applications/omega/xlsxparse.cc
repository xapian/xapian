/** @file xslxparse.cc
 * @brief Extract fields from XLSX sheet*.xml.
 */
/* Copyright (C) 2012,2013 Olly Betts
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
#include <cstring>
#include <ctime>

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
	    if (get_parameter("s", type)) {
		unsigned long style_id = strtoul(type.c_str(), NULL, 10);
		if (date_style.find(style_id) != date_style.end()) {
		    mode = MODE_C_DATE;
		}
	    }
	}
    } else if (tag == "v") {
	if (mode == MODE_C_LITERAL) {
	    mode = MODE_V_LITERAL;
	} else if (mode == MODE_C_STRING) {
	    mode = MODE_V_STRING;
	} else if (mode == MODE_C_DATE) {
	    mode = MODE_V_DATE;
	}
    } else if (tag == "si") {
	mode = MODE_SI;
    } else if (tag == "sst") {
	string unique_count;
	if (get_parameter("uniquecount", unique_count)) {
	    unsigned long c = strtoul(unique_count.c_str(), NULL, 10);
	    // This reserving is just a performance tweak, so don't go
	    // reserving ludicrous amounts of space just because an XML
	    // attribute told us to.
	    sst.reserve(std::min(c, 1000000ul));
	}
    } else if (tag == "workbookpr") {
	string v;
	if (get_parameter("date1904", v)) {
	    date1904 = (v == "true" || v == "1");
	}
    } else if (tag == "numfmt") {
	string formatcode;
	if (get_parameter("formatcode", formatcode)) {
	    // Heuristic for "date format" (FIXME: implement properly)
	    if (strchr(formatcode.c_str(), 'd') &&
		strchr(formatcode.c_str(), 'm') &&
		strchr(formatcode.c_str(), 'y')) {
		string v;
		if (get_parameter("numfmtid", v)) {
		    unsigned long id = strtoul(v.c_str(), NULL, 10);
		    date_format.insert(id);
		}
	    }
	}
    } else if (tag == "cellxfs") {
	mode = MODE_CELLXFS;
    } else if (tag == "xf") {
	if (mode == MODE_CELLXFS) {
	    string v;
	    if (get_parameter("applynumberformat", v)) {
		if (v == "true" || v == "1") {
		    if (get_parameter("numfmtid", v)) {
			unsigned long id = strtoul(v.c_str(), NULL, 10);
			if ((id >= 14 && id <= 17) ||
			    date_format.find(id) != date_format.end()) {
			    date_style.insert(style_index);
			}
		    }
		}
	    }
	    ++style_index;
	}
    }
    return true;
}

void
XlsxParser::process_text(const string &text)
{
    switch (mode) {
	case MODE_V_DATE: {
	    // Date field.
	    unsigned long c = strtoul(text.c_str(), NULL, 10);
	    if (date1904) {
		c -= 24107;
	    } else {
		// The spec insists we treat 1900 as a leap year!
		if (c > 60) --c;
		c -= 25568;
	    }
	    time_t t = c * 86400 + 43200;
	    struct tm * tm = gmtime(&t);
	    if (tm) {
		char buf[32];
		size_t res = strftime(buf, sizeof(buf), "%Y-%m-%d", tm);
		if (res)
		    append_field(string(buf, res));
	    }
	    mode = MODE_NONE;
	    return;
	}
	case MODE_V_STRING: {
	    // Shared string use.
	    unsigned long c = strtoul(text.c_str(), NULL, 10);
	    if (c < sst.size()) {
		append_field(sst[c]);
	    }
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
