/* xmlparse.h: subclass of HtmlParser for parsing XML.
 *
 * Copyright (C) 2006,2008,2009,2011 Olly Betts
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

#ifndef OMEGA_INCLUDED_XMLPARSE_H
#define OMEGA_INCLUDED_XMLPARSE_H

#include "myhtmlparse.h"

class XmlParser : public MyHtmlParser {
  public:
    XmlParser() : MyHtmlParser() { }
    bool opening_tag(const string &tag);
    bool closing_tag(const string &tag);
    void parse_html(const string &text) {
	// Ignore overriding charsets in meta tags.
	MyHtmlParser::parse_html(text, "utf-8", true);
    }
};

#endif // OMEGA_INCLUDED_XMLPARSE_H
