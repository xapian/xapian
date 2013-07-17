/** @file xslxparse.h
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

#ifndef OMEGA_INCLUDED_XLSXPARSE_H
#define OMEGA_INCLUDED_XLSXPARSE_H

#include "htmlparse.h"

#include <vector>
#include <set>

class XlsxParser : public HtmlParser {
    std::vector<std::string> sst;
    std::set<unsigned> date_style;
    std::set<unsigned long> date_format;

    enum {
	MODE_NONE,
	MODE_SI,
	MODE_C_STRING,
	MODE_V_STRING,
	MODE_C_LITERAL,
	MODE_V_LITERAL,
	MODE_C_DATE,
	MODE_V_DATE,
	MODE_CELLXFS
    } mode;

    bool date1904;
    unsigned style_index;

    void append_field(const std::string &text) {
	if (!text.empty()) {
	    if (!dump.empty()) dump += ' ';
	    dump += text;
	}
    }

  public:
    std::string dump;

    XlsxParser()
	: HtmlParser(), mode(MODE_NONE), date1904(false), style_index(0) { }
    bool opening_tag(const std::string &tag);
    void process_text(const std::string &text);
};

#endif // OMEGA_INCLUDED_XLSXPARSE_H
