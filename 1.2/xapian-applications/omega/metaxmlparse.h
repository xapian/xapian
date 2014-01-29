/* metaxmlparse.h: subclass of HtmlParser for parsing OpenDocument's meta.xml.
 *
 * Copyright (C) 2006,2009,2010,2011 Olly Betts
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

#ifndef OMEGA_INCLUDED_METAXMLPARSE_H
#define OMEGA_INCLUDED_METAXMLPARSE_H

#include "htmlparse.h"

class MetaXmlParser : public HtmlParser {
    enum { NONE, KEYWORDS, TITLE, SAMPLE, AUTHOR } field;
  public:
    MetaXmlParser() : field(NONE) { }
    void process_text(const string &text);
    bool opening_tag(const string &tag);
    bool closing_tag(const string &tag);
    string title, keywords, sample, author;
};

#endif // OMEGA_INCLUDED_METAXMLPARSE_H
