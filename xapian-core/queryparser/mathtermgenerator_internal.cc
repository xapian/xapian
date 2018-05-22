/** @file mathtermgenerator_internal.cc
 * @brief MathTermGenerator class internals
 */
/* Copyright (C) 2018 Guruprasad Hegde
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

#include "mathtermgenerator_internal.h"

/* #define DEBUGGING */

#include <xapian/document.h>
#include <xapian/unicode.h>

#include "stringutils.h"

#include <cstring>
#ifdef DEBUGGING
#include <iostream>
#endif
#include <string>
#include <vector>


using namespace std;

namespace Xapian {

bool move_to_next_open_tag(const char *& ch, string & tag);
string get_element_value(const char *& ch);
string get_label(const char *& ch, string & tag);

string
get_element_value(const char *& ch)
{
    // Skip leading whitespace.
    while (*ch != '\0' && *ch == ' ') {
	++ch;
    }

    // TODO Handle EOF.

    string value;
    while (*ch != '\0' && *ch != '<') {
	value.push_back(*ch);
	++ch;
    }

    // TODO Handle EOF.

    // Skip trailing whitespace.
    return value.substr(0, value.find_last_not_of(' ') + 1);
}

bool
move_to_next_open_tag(const char *& ch, string & tag)
{
    while (*ch != '\0') {
	if (ch[0] == '<') {
	    if (ch[1] != '/') {
		++ch;
		break;
	    }
	    if (strncmp(ch + 2, "math>", 5) == 0)
		return false;
	}
	++ch;
    }

    if (*ch == '\0')
	return false;

    tag.clear();
    while (*ch != '\0' && *ch != '>') {
	tag.push_back(*ch);
	++ch;
    }
    // TODO Handle EOF.
    ++ch;
    return true;
}

string
get_label(const char *& ch, string & tag) {
    if (tag.compare("mi") == 0)
	return string("V!" + get_element_value(ch));
    else if (tag.compare("mn") == 0)
	return string("N!" + get_element_value(ch));
    else if (tag.compare("mo") == 0)
	return string("O" + get_element_value(ch));
    else if (tag.compare("mtext") == 0)
	return string("T!" + get_element_value(ch));
    else {
	// Not supported tag, mark as unknown for now.
	return string("U!" + get_element_value(ch));
    }
}

void
MathTermGenerator::Internal::parse_mathml(const char *& ch)
{
    // Clear mrow contents.
    mrow.clear();
    string tag;
    while (*ch != '\0') {
	// Detect '<math ' or '<math>'.
	if ((strncmp(ch, "<math", 5) == 0) && (ch[5] == ' ' || ch[5] == '>')) {
	    ch += 6;
	    // Parse elements until '</math' found.
	    while (move_to_next_open_tag(ch, tag)) {
		if (tag.compare("mfrac") == 0) {
		    // Add fraction symbol.
		    mrow.emplace_back("F", ADJACENT);
		    // Parse numerator.
		    if (move_to_next_open_tag(ch, tag))
			mrow.back().trow.emplace_back(get_label(ch, tag),
					              ABOVE);
		    // Parse denominator.
		    if (move_to_next_open_tag(ch, tag))
			mrow.back().brow.emplace_back(get_label(ch, tag),
					              BELOW);
		} else if (tag.compare("mroot") == 0) {
		    // Add root symbol.
		    mrow.emplace_back("R", ADJACENT);
		    // Parse base.
		    if (move_to_next_open_tag(ch, tag))
			mrow.emplace_back(get_label(ch, tag), WITHIN);
		    // Parse index.
		    if (move_to_next_open_tag(ch, tag))
			mrow.back().trow.emplace_back(get_label(ch, tag),
					              ABOVE);
		} else if (tag.compare("msqrt") == 0) {
		    // Add root symbol.
		    mrow.emplace_back("R", ADJACENT);
		    // Parse base.
		    if (move_to_next_open_tag(ch, tag))
			mrow.emplace_back(get_label(ch, tag), WITHIN);
		} else {
		    // Parse token element.
		    mrow.emplace_back(get_label(ch, tag), ADJACENT);
		}
	    }
	} else {
	    // <math> tag not found yet, move to next char.
	    ++ch;
	}
    }
}

void
MathTermGenerator::Internal::index_math(const char * ch)
{
    parse_mathml(ch);

#ifdef DEBUGGING
    // Debug prints
    cout << "\n===============\n";
    cout << "#symbols on main line : " << mrow.size() << '\n';
    cout << "Main line:";
    for (auto & sym : mrow)
	cout << "---->" << "(" << sym.label << ")";
#endif
}
}
