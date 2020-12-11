/** @file
 * @brief Parser for Microsoft XML formats (.docx, etc).
 */
/* Copyright (C) 2006,2009,2011,2012,2013,2020 Olly Betts
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

#include "msxmlparser.h"

using namespace std;

void
MSXmlParser::process_content(const string& content)
{
    dump += content;
}

bool
MSXmlParser::closing_tag(const string& tag)
{
    // For .docx and .pptx respectively.
    if (tag == "w:t" || tag == "a:t") {
	if (!dump.empty())
	    dump += ' ';
    }
    return true;
}
