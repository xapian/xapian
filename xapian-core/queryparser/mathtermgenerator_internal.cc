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

#include "str.h"
#include <xapian/document.h>
#include <xapian/unicode.h>

#include "mathml.h"

#include <algorithm>
#include <cstring>
#ifdef DEBUGGING
#include <iostream>
#endif
#include <string>
#include <vector>


using namespace std;

namespace Xapian {

string
MathMLParser::get_element_value()
{
    // Skip leading whitespace.
    while (it != end && *it == ' ') {
	++it;
    }

    // TODO Handle EOF.

    string value;
    while (it != end && *it != '<') {
	value.push_back(*it);
	++it;
    }

    // TODO Handle EOF.

    // Skip trailing whitespace.
    return value.substr(0, value.find_last_not_of(' ') + 1);
}

void
MathMLParser::skip_xml_prefix()
{
    while (it != end && *it != ':')
	++it;
    if (it == end)
	return;
    }
}

bool
MathMLParser::skip_close_tag()
{
    it = find(it, end, '<');
    if (it == end || it[1] != '/') {
	end_math = true;
	end_mrow = true;
	return false;
    }

    auto start_it = it;
    it = find(it, end, '>');

    if (it == end) {
	end_math = true;
	end_mrow = true;
	return false;
    }

    auto close_id = mathmlid_table[string(start_it + 2, it)];

    if (close_id == MATH) {
	end_math = true;
	level--;
    } else if (close_id == MROW) {
	end_mrow = true;
	level--;
    } else if (close_id == MTABLE) {
	end_mtable = true;
    }

    it++;
    return true;
}

bool
MathMLParser::get_open_tag()
{
    bool closed = false;
    do {
	auto start_it = std::find(it, end, '<');

	it = start_it;
	if (it == end) {
	    end_math = true;
	    end_mrow = true;
	    return false;
	}

	if (xml_prefix)
	    skip_xml_prefix();

	it++;

	if (it[0] == '/') {
	    closed = true;
	    it = start_it;
	    skip_close_tag();
	} else
	    closed = false;
    } while (closed);

    cur_tag.clear();
    while (it != end && *it != ' ' && *it != '>') {
	cur_tag.push_back(*it);
	++it;
    }

    // Skip attribute values
    if (*it == ' ') {
	it = std::find(it, end, '>');
	if (it == end) {
	    end_math = true;
	    end_mrow = true;
	    return false;
	}
    }

    ++it;
    return true;
}

void
MathMLParser::parse_expression(vector<Symbol> & crow, char relation)
{
    if (!get_open_tag())
	return;
    auto cur_id = mathmlid_table[cur_tag];
    switch (cur_id) {
	case MATH:
	    end_math = false;
	    do {
		parse_expression(crow, relation);
	    } while (!end_math);
	    end_math = false;
	    return;
	case MROW:
	    level++;
	    end_mrow = false;
	    do {
		parse_expression(crow, relation);
		relation = NEXT;
	    } while (!end_mrow);
	    end_mrow = false;
	    return;
	case MTABLE:
	    nrows = 0;
	    end_mtable = false;
	    while (!end_mtable && get_open_tag()) {
		if (mathmlid_table[cur_tag] == MTR)
		    nrows++;
		skip_close_tag();
	    }
	    crow.emplace_back(tag_values[MTABLE] + str(nrows), NEXT);
	    break;
	case MANNOTATION:
	case MSEMANTICS:
	    break;
	case MFRAC:
	    // Add fraction symbol.
	    crow.emplace_back(tag_values[cur_id], NEXT);
	    // Numerator
	    parse_expression(crow.back().trow, ABOVE);
	    // Denominator
	    parse_expression(crow.back().brow, BELOW);
	    skip_close_tag();
	    break;
	case MROOT:
	    // Add root symbol.
	    crow.emplace_back(tag_values[cur_id], NEXT);
	    // Parse base.
	    parse_expression(crow, WITHIN);
	    // Parse index.
	    parse_expression(crow.back().trow, ABOVE);
	    break;
	case MSQRT:
	    // Add root symbol.
	    crow.emplace_back(tag_values[cur_id], NEXT);
	    // Parse base.
	    parse_expression(crow.back().trow, ABOVE);
	    skip_close_tag();
	    break;
	case MSUP:
	    // Parse base.
	    parse_expression(crow, NEXT);
	    // Parse superscript.
	    parse_expression(crow.back().trow, ABOVE);
	    break;
	case MSUB:
	    // Parse base.
	    parse_expression(crow, NEXT);
	    // Parse subscript.
	    parse_expression(crow.back().brow, BELOW);
	    break;
	case MSUBSUP:
	    // Parse base.
	    parse_expression(crow, NEXT);
	    // Parse subscript.
	    parse_expression(crow.back().brow, BELOW);
	    // Parse superscript.
	    parse_expression(crow.back().trow, ABOVE);
	    break;
	case MUNDER:
	    // Parse base.
	    parse_expression(crow, NEXT);
	    // Parse underscript.
	    parse_expression(crow.back().brow, UNDER);
	    break;
	case MOVER:
	    // Parse base.
	    parse_expression(crow, NEXT);
	    // Parse overscript.
	    parse_expression(crow.back().trow, OVER);
	    break;
	case MUNDEROVER:
	    // Parse base.
	    parse_expression(crow, NEXT);
	    // Parse underscript.
	    parse_expression(crow.back().brow, UNDER);
	    // Parse overscript.
	    parse_expression(crow.back().trow, OVER);
	    break;
	case MI:
	case MN:
	case MO:
	case MTEXT:
	    if (!(relation != NEXT && crow.empty()))
		relation = NEXT;
	    crow.emplace_back(tag_values[cur_id] + get_element_value(),
				relation);
	    skip_close_tag();
	    return;
	default:
	    // Unrecognized token. Skip for now.
	    break;
    } // End switch statement.
}

vector<Symbol>
MathMLParser::parse(const string & text) {
    it = text.begin();
    end = text.end();
    vector<Symbol> mrow;
    mrow.reserve(EST_SYMBOLS_COUNT);
    while (it != end) {
	// TODO Handling xml_prefix this way is totally safe. Will fix it later.
	// Detect '<math ' or '<math>' or ':math ' or ':math>'
	if ((it[0] == '<' || it[0] == ':') &&
		it[1] == 'm' &&
		it[2] == 'a' &&
		it[3] == 't' &&
		it[4] == 'h' &&
		(it[5] == ' ' || it[5] == '>')) {
	    if (it[0] == ':') {
		xml_prefix = true;
		do {
		    --it;
		} while (it[0] != '<');
	    }
	    // Parse elements until '</math' found.
	    level = 0;
	    error = false;
	    parse_expression(mrow, NEXT);
	} else {
	    // <math> tag not found yet, move to next char.
	    ++it;
	}
    }
    return mrow;
}

void
MathTermGenerator::Internal::index_math(const std::string & text,
					const string & prefix)
{
    mrow = mlp.parse(text);

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
    auto symbol_pairs = generate_symbol_pair_list();

    // Index symbol-pair string in db.
    for (const auto & sp : symbol_pairs)
	doc.add_term(prefix + sp);
}

const unsigned MAX_PATH_LEN = 3;

vector<string>
MathTermGenerator::Internal::generate_symbol_pair_list()
{
    // If main row has size n, then number of tuples along main row is
    // T = (n * (n+1))/ 2. if main row has trow and brow of each size 2, then
    // total pairs is T + 4 * T. This computation is not accurate, an attempt to
    // avoid reallocation.
    unsigned long approx_pairs_count = (mrow.size() * (mrow.size() + 1)) * 3;
    vector<string> symbol_pairs;
    symbol_pairs.reserve(approx_pairs_count);
    string pair;
    string path;
    string unified_pair;
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
		if (unification && (mrow[i].label[0] == 'V' ||
			    mrow[j].label[0] == 'V')) {
		    unified_pair.clear();
		    unified_pair.push_back(mrow[i].label[0]);
		    unified_pair.push_back(mrow[j].label[0]);
		    unified_pair.append(path);
		    symbol_pairs.push_back(unified_pair);
		}
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
		    if (unification && (mrow[i].label[0] == 'V' ||
				subrow[offset].label[0] == 'V')) {
			unified_pair.clear();
			unified_pair.push_back(mrow[i].label[0]);
			unified_pair.push_back(subrow[offset].label[0]);
			unified_pair.append(subpath);
			symbol_pairs.push_back(unified_pair);
		    }
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
		    if (unification && (mrow[i].label[0] == 'V' ||
				subrow[offset].label[0] == 'V')) {
			unified_pair.clear();
			unified_pair.push_back(mrow[i].label[0]);
			unified_pair.push_back(subrow[offset].label[0]);
			unified_pair.append(subpath);
			symbol_pairs.push_back(unified_pair);
		    }
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

    return symbol_pairs;
}

void
MathTermGenerator::Internal::set_unification(const bool unify)
{
    unification = unify;
}

vector<string>
MathTermGenerator::Internal::get_symbol_pair_list(const std::string & text)
{
    mrow = mlp.parse(text);
    return generate_symbol_pair_list();
}
}
