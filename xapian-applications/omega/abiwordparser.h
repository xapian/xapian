/** @file
 * @brief Extract text from Abiword documents.
 */
/* Copyright (C) 2020 Olly Betts
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

#ifndef OMEGA_INCLUDED_ABIWORDPARSER_H
#define OMEGA_INCLUDED_ABIWORDPARSER_H

#include "xmlparser.h"

class AbiwordParser : public XmlParser {
  public:
    std::string dump;

    AbiwordParser() { }

    void process_content(const std::string& content);

    void parse(const std::string& text) {
	charset = "utf-8";
	XmlParser::parse(text);
    }
};

#endif // OMEGA_INCLUDED_ABIWORDPARSER_H
