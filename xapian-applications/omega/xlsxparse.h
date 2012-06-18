/** @file xslxparse.h
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

#ifndef OMEGA_INCLUDED_XLSXPARSE_H
#define OMEGA_INCLUDED_XLSXPARSE_H

#include "myhtmlparse.h"

class XlsxParser : public MyHtmlParser {
    bool index_content, in_v;
  public:
    XlsxParser() : MyHtmlParser(), index_content(false), in_v(false) { }
    bool opening_tag(const std::string &tag);
    bool closing_tag(const std::string &tag);
    void process_text(const std::string &text);

    void parse_html(const std::string &text) {
	// Ignore overriding charsets in meta tags.
	MyHtmlParser::parse_html(text, "utf-8", true);
    }
};

#endif // OMEGA_INCLUDED_XLSXPARSE_H
