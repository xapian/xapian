/** @file
 * @brief Generic parser for XML - just replaces tags with spaces
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

#include <config.h>

#include "genericxmlparser.h"

#include "stringutils.h"

using namespace std;

void
GenericXmlParser::process_content(const string& content)
{
    auto first_non_space = find_if_not(content.begin(), content.end(),
				       C_isspace);
    if (first_non_space == content.end()) {
	// Ignore content which is empty or all whitespace - we'll add a space
	// before we append any further content anyway.
	return;
    }
    if (!dump.empty())
	dump += ' ';
    auto first = first_non_space - content.begin();
    auto last_non_space = find_if_not(content.rbegin(), content.rend(),
				      C_isspace);
    auto last = content.size() - (last_non_space - content.rbegin());
    dump.append(content, first, last - first);
}
