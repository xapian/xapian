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
void skip_prefix(const char *& ch);
string next_tag(const char *& ch, string & cur_tag);
// TODO temporary usage of static. This will be either moved to function
// arguments or plan to implement parser class where this data can be stored.
static bool prefix = false;

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

void
skip_prefix(const char *& ch)
{
    while (*ch != '\0' && *ch != ':')
	++ch;
    if (*ch == '\0')
	return;
    ++ch;
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
	    ch = ch + 2;
	    if (prefix) skip_prefix(ch);
	    if (strncmp(ch, "math>", 5) == 0)
		return false;
	}
	++ch;
    }

    if (*ch == '\0')
	return false;

    if (prefix)
	skip_prefix(ch);
    tag.clear();
    while (*ch != '\0' && *ch != ' ' &&  *ch != '>') {
	tag.push_back(*ch);
	++ch;
    }

    if (*ch == ' ') {
	while (*ch != '>')
	    ++ch;
    }
    // TODO Handle EOF.
    ++ch;
    return true;
}

string
next_tag(const char *& ch, string & cur_tag)
{
    string tag;
    while (*ch != '\0') {
	if (ch[0] == '<') {
	    ++ch;
	    if (ch[0] == '/')
		++ch;
	    while (*ch != '\0' && *ch != '>') {
		tag.push_back(*ch);
		++ch;
	    }
	    ++ch;
	    if (tag.compare(cur_tag) == 0) {
		tag.clear();
		continue;
	    } else
		return tag;
	} else
	    ++ch;
    }
    return tag;
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
	// TODO Handling prefix this way is totally safe. Will fix it later.
	// Detect '<math ' or '<math>' or ':math ' or ':math>'
	if ((ch[0] == '<' || ch[0] == ':') && strncmp(ch + 1, "math", 4) == 0 &&
	    (ch[5] == ' ' || ch[5] == '>')) {
	    if (ch[0] == ':')
		prefix = true;
	    ch += 6;
	    // Parse elements until '</math' found.
	    while (move_to_next_open_tag(ch, tag)) {
		if (tag.compare("mrow") == 0 || tag.compare("annotation") == 0
			|| tag.compare("semantics") == 0) {
		    continue;
		} else if (tag.compare("mfrac") == 0) {
		    // Add fraction symbol.
		    mrow.emplace_back("F", NEXT);
		    // Parse numerator.
		    if (move_to_next_open_tag(ch, tag) &&
			    tag.compare("mrow") == 0) {
			if (move_to_next_open_tag(ch, tag))
			    mrow.back().trow.emplace_back(get_label(ch, tag),
				    ABOVE);
			while (next_tag(ch, tag).compare("mrow") != 0) {
			    if (move_to_next_open_tag(ch, tag))
				// TODO This only works if all the elemments
				// are token elements.
				mrow.back().trow.emplace_back(
					get_label(ch, tag), NEXT);
			}
		    } else {
			mrow.back().trow.emplace_back(get_label(ch, tag),
				ABOVE);
		    }
		    // Parse denominator.
		    if (move_to_next_open_tag(ch, tag) &&
			    tag.compare("mrow") == 0) {
			if (move_to_next_open_tag(ch, tag))
			    mrow.back().brow.emplace_back(get_label(ch, tag),
				    BELOW);
			while (next_tag(ch, tag).compare("mrow") != 0) {
			    if (move_to_next_open_tag(ch, tag))
				// TODO This only works if all the elemments are
				// token elements.
				mrow.back().brow.emplace_back(get_label(ch,
					    tag), NEXT);
			}
		    } else {
			mrow.back().brow.emplace_back(get_label(ch, tag),
				BELOW);
		    }
		} else if (tag.compare("mroot") == 0) {
		    // Add root symbol.
		    mrow.emplace_back("R", NEXT);
		    // Parse base.
		    if (move_to_next_open_tag(ch, tag))
			mrow.emplace_back(get_label(ch, tag), WITHIN);
		    // Parse index.
		    if (move_to_next_open_tag(ch, tag))
			mrow.back().trow.emplace_back(get_label(ch, tag),
				ABOVE);
		} else if (tag.compare("msqrt") == 0) {
		    // Add root symbol.
		    mrow.emplace_back("R", NEXT);
		    // Parse base.
		    if (move_to_next_open_tag(ch, tag) &&
			    tag.compare("mrow") == 0) {
			if (move_to_next_open_tag(ch, tag))
			    mrow.back().trow.emplace_back(
				    get_label(ch, tag), WITHIN);
			while (next_tag(ch, tag).compare("mrow") != 0) {
			    if (move_to_next_open_tag(ch, tag))
				// TODO This only works if all the elemments are
				// token elements.
				mrow.back().trow.emplace_back(
					get_label(ch, tag), NEXT);
			}
		    } else {
			mrow.back().trow.emplace_back(get_label(ch, tag),
				WITHIN);
		    }
		} else if (tag.compare("msup") == 0) {
		    // Parse base.
		    if (move_to_next_open_tag(ch, tag))
			mrow.emplace_back(get_label(ch, tag), NEXT);
		    // Parse superscript.
		    if (move_to_next_open_tag(ch, tag))
			mrow.back().trow.emplace_back(get_label(ch, tag),
				ABOVE);
		} else if (tag.compare("msub") == 0) {
		    // Parse base.
		    if (move_to_next_open_tag(ch, tag))
			mrow.emplace_back(get_label(ch, tag), NEXT);
		    // Parse subscript.
		    if (move_to_next_open_tag(ch, tag))
			mrow.back().brow.emplace_back(get_label(ch, tag),
				BELOW);
		} else if (tag.compare("msubsup") == 0) {
		    // Parse base.
		    if (move_to_next_open_tag(ch, tag))
			mrow.emplace_back(get_label(ch, tag), NEXT);
		    // Parse subscript.
		    if (move_to_next_open_tag(ch, tag))
			mrow.back().brow.emplace_back(get_label(ch, tag),
				BELOW);
		    // Parse superscript.
		    if (move_to_next_open_tag(ch, tag))
			mrow.back().trow.emplace_back(get_label(ch, tag),
				ABOVE);
		} else if (tag.compare("munder") == 0) {
		    // Parse base.
		    if (move_to_next_open_tag(ch, tag))
			mrow.emplace_back(get_label(ch, tag), NEXT);
		    // Parse underscript.
		    if (move_to_next_open_tag(ch, tag))
			mrow.back().brow.emplace_back(get_label(ch, tag),
				UNDER);
		} else if (tag.compare("mover") == 0) {
		    // Parse base.
		    if (move_to_next_open_tag(ch, tag))
			mrow.emplace_back(get_label(ch, tag), NEXT);
		    // Parse overscript.
		    if (move_to_next_open_tag(ch, tag))
			mrow.back().trow.emplace_back(get_label(ch, tag),
				OVER);
		} else if (tag.compare("munderover") == 0) {
		    // Parse base.
		    if (move_to_next_open_tag(ch, tag))
			mrow.emplace_back(get_label(ch, tag), NEXT);
		    // Parse underscript.
		    if (move_to_next_open_tag(ch, tag))
			mrow.back().brow.emplace_back(get_label(ch, tag),
				UNDER);
		    // Parse overscript.
		    if (move_to_next_open_tag(ch, tag))
			mrow.back().trow.emplace_back(get_label(ch, tag),
				OVER);

		} else {
		    // Parse token element.
		    mrow.emplace_back(get_label(ch, tag), NEXT);
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

#ifdef DEBUGGING_NO
    // Debug prints
    cout << "\n===============\n";
    cout << "Main line:";
    for (auto & sym : mrow) {
	cout << ">>>>>" << "(" << sym.label << ", " << sym.edge << ")";
	if (!sym.trow.empty()) {
	    cout << "\nTop line for " << sym.label << ':';
	    for (auto & tsym : sym.trow)
		cout << "--->" << '(' << tsym.label << ", " << tsym.edge << ')';
	    cout << '\n';
	}
	if (!sym.brow.empty()) {
	    cout << "\nBottom line for " << sym.label << ':';
	    for (auto & bsym : sym.brow)
		cout << "--->" << '(' << bsym.label << ", " << bsym.edge << ')';
	    cout << '\n';
	}
    }
#endif
    generate_symbol_pair_list();

    // Index symbol-pair string in db.
    for (const auto & sp : symbol_pairs)
	doc.add_term(sp);
}

const unsigned MAX_PATH_LEN = 3;

void
MathTermGenerator::Internal::generate_symbol_pair_list()
{
    // If main row has size n, then number of tuples along main row is
    // T = (n * (n+1))/ 2. if main row has trow and brow of each size 2, then
    // total pairs is T + 4 * T. This computation is not accurate, an attempt to
    // avoid reallocation.
    unsigned long approx_pairs_count = (mrow.size() * (mrow.size() + 1)) * 3;
    symbol_pairs.reserve(approx_pairs_count);
    string pair;
    string path;
    for (vector<Symbol>::size_type i = 0; i < mrow.size(); ++i) {
	path.clear();
	auto j = i;
	while (j < mrow.size() && path.size() <= MAX_PATH_LEN) {
	    if (i != j) {
		path.push_back(mrow[j].edge);
		pair.reserve(mrow[i].label.size() + mrow[j].label.size() +
			path.size());
		pair.clear();
		pair.append(mrow[i].label);
		pair.append(mrow[j].label);
		pair.append(path);
		symbol_pairs.push_back(pair);
	    }
	    // Get subpath along trow.
	    if (!mrow[j].trow.empty()) {
		unsigned offset = 0;
		auto & subrow = mrow[j].trow;
		string subpath(path);
		while (offset < subrow.size() && path.size() <= MAX_PATH_LEN) {
		    subpath.push_back(subrow[offset].edge);
		    pair.reserve(mrow[i].label.size() +
			    subrow[offset].label.size() + path.size());
		    pair.clear();
		    pair.append(mrow[i].label);
		    pair.append(subrow[offset].label);
		    pair.append(subpath);
		    symbol_pairs.push_back(pair);
		    ++offset;
		}
	    }
	    // Get subpath along brow.
	    if (!mrow[j].brow.empty()) {
		unsigned offset = 0;
		auto & subrow = mrow[j].brow;
		auto subpath(path);
		while (offset < subrow.size() && path.size() <= MAX_PATH_LEN) {
		    subpath.push_back(subrow[offset].edge);
		    pair.reserve(mrow[i].label.size() +
			    subrow[offset].label.size() + path.size());
		    pair.clear();
		    pair.append(mrow[i].label);
		    pair.append(subrow[offset].label);
		    pair.append(subpath);
		    symbol_pairs.push_back(pair);
		    ++offset;
		}
	    }
	    ++j;
	}
	// TODO currently source symbol is taken only from mrow
	// include trow and brow in the future.
	// TODO There is a pattern, refactor possible? If so, worth
	// doing it, becoz for me this looks very clear.
    }
#ifdef DEBUGGING
    // Debug prints
    cout << "\n===Symbol pairs========\n";
    for (auto & p : symbol_pairs)
	cout << p << '\n';
    cout << "#symbols on main line : " << mrow.size() << '\n';
    cout << "total pairs = " << symbol_pairs.size();
#endif
}
}
