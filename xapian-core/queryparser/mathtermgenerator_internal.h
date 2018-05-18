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
#include <xapian/database.h>
#include <xapian/document.h>
#include <xapian/mathtermgenerator.h>

#include <string>
#include <vector>

namespace Xapian {

struct Symbol;

class MathTermGenerator::Internal : public Xapian::Internal::intrusive_base {
    friend class MathTermGenerator;
    std::vector<Symbol> mrow;
    void parse_mathml(const char *& ch);
  public:
    Internal() { }

    void index_math(const char * ch);

    // For debugging purpose.
    std::size_t symbol_count() {
	return mrow.size();
    }
};

enum edge {
    ADJACENT,
    ABOVE,
    BELOW,
    OVER,
    UNDER,
    WITHIN,
    ELEMENT
};

struct Symbol {
    std::string label;
    edge e;
    // Symbols like fraction, matrix are represented over multiple lines.
    // For example, consider a / b. a goes to top row, b goes to bottom
    // row and fraction symbol is added in middle row.
    // TODO Think of better way
    std::vector<Symbol> trow;	// Top row.
    std::vector<Symbol> brow;	// Bottom row.
    explicit Symbol(std::string l, edge e_) : label(l), e(e_) { }
};
}

#endif // XAPIAN_INCLUDED_MATHTERMGENERATOR_INTERNAL_H
