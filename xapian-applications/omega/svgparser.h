/** @file
 * @brief Extract text from an SVG file.
 */
/* Copyright (C) 2010,2011,2019 Olly Betts
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

#ifndef OMEGA_INCLUDED_SVGPARSER_H
#define OMEGA_INCLUDED_SVGPARSER_H

#include "xmlparser.h"

class SvgParser : public XmlParser {
    enum { OTHER, TEXT, METADATA, TITLE, KEYWORDS, AUTHOR } state = OTHER;
    std::string dc_tag;

  public:
    SvgParser() { }
    void process_content(const std::string& content);
    bool opening_tag(const std::string& tag);
    bool closing_tag(const std::string& tag);
    std::string title, keywords, dump, author;
};

#endif // OMEGA_INCLUDED_SVGPARSER_H
