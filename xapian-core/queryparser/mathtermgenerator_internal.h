/** @file mathtermgenerator_internal.h
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

#ifndef XAPIAN_INCLUDED_MATHTERMGENERATOR_INTERNAL_H
#define XAPIAN_INCLUDED_MATHTERMGENERATOR_INTERNAL_H

#include "xapian/intrusive_ptr.h"
#include <xapian/document.h>
#include <xapian/mathtermgenerator.h>

#include <string>
#include <vector>

namespace Xapian {

const unsigned EST_SYMBOLS_COUNT = 32;
const unsigned EST_SUB_SYMBOLS_COUNT = 8;

// Spatial relationship between symbols.
const char NEXT = 'N';
const char ABOVE = 'A';
const char BELOW = 'B';
const char OVER = 'O';
const char UNDER = 'U';
const char WITHIN = 'W';
const char ELEMENT = 'E';

struct Symbol {
    std::string label;
    char edge;
    // Symbols like fraction, matrix are represented over multiple lines.
    // For example, consider a / b. a goes to top row, b goes to bottom
    // row and fraction symbol is added in middle row.
    std::vector<Symbol> trow;	// Top row.
    std::vector<Symbol> brow;	// Bottom row.
    Symbol() = default;
    explicit Symbol(std::string l, char e) : label(l), edge(e) {
	trow.reserve(EST_SUB_SYMBOLS_COUNT);
	brow.reserve(EST_SUB_SYMBOLS_COUNT);
    }
};

class MathMLParser {
    bool xml_prefix = false;
    bool end_math = false;
    bool end_mrow = false;
    bool end_mtable = false;
    bool error = false;
    unsigned nrows = 0;
    unsigned level = 0;
    std::string cur_tag = "";
    std::string::const_iterator it;
    std::string::const_iterator end;
    /* helper methods */
    bool get_open_tag();
    std::string get_element_value();
    void skip_xml_prefix();
    bool skip_close_tag();
    void parse_expression(std::vector<Symbol> & mrow, char relation = NEXT);
  public:
    std::vector<Symbol> parse(const std::string & text);
    bool parse_error() {
	return error;
    }
};

class MathTermGenerator::Internal : public Xapian::Internal::intrusive_base {
    friend class MathTermGenerator;
    bool unification = true;
    Document doc;
    MathMLParser mlp;
    std::vector<Symbol> mrow;
    std::vector<std::string> generate_symbol_pair_list();
  public:
    Internal() : mrow(EST_SYMBOLS_COUNT) { }

    void index_math(const std::string & text, const std::string & prefix);

    std::vector<std::string> get_symbol_pair_list(const std::string & text);

    void set_unification(const bool unify);

    bool parse_error() {
	return mlp.parse_error();
    }

    // For debugging purpose.
    std::vector<std::string> get_labels_list() {
	std::vector<std::string> labels;
	for (const auto & sym : mrow)
	    labels.push_back(sym.label);
	return labels;
    }
};

}

#endif // XAPIAN_INCLUDED_MATHTERMGENERATOR_INTERNAL_H
