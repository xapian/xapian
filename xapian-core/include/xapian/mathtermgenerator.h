/** @file mathtermgenerator.h
 * @brief parse math expression and generate terms
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

#ifndef XAPIAN_INCLUDED_MATHTERMGENERATOR_H
#define XAPIAN_INCLUDED_MATHTERMGENERATOR_H

#if !defined XAPIAN_IN_XAPIAN_H && !defined XAPIAN_LIB_BUILD
# error "Never use <xapian/mathtermgenerator.h> directly; include <xapian.h> instead."
#endif

#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/unicode.h>
#include <xapian/visibility.h>

#include <string>
#include <vector>

namespace Xapian {

class Document;
class WritableDatabase;

/** Parses a math expression and generate terms.
 *
 * This module takes a math expression and parses it to produce layout tree
 * which are then used to generate suitable terms for indexing. The terms
 * generated are represented by SymbolPair class. These terms are suitable for
 * use with Query objects produced by QueryParser class for math expression.
 */
class XAPIAN_VISIBILITY_DEFAULT MathTermGenerator {
  public:
    /// @private @internal Class representing the MathTermGenerator internals.
    class Internal;
    /// @private @internal Reference counted internals.
    Xapian::Internal::intrusive_ptr_nonnull<Internal> internal;

    /// Copy constructor.
    MathTermGenerator(const MathTermGenerator & o);

    /// Assignment.
    MathTermGenerator & operator=(const MathTermGenerator & o);

    /// Move constructor.
    MathTermGenerator(MathTermGenerator && o);

    /// Move assignment operator.
    MathTermGenerator & operator=(MathTermGenerator && o);

    /// Default constructor.
    MathTermGenerator();

    /// Destructor.
    ~MathTermGenerator();

    /** Index some math expression in null-terminated string.
     *
     * @param expr	The expression to index.
     */
    void index_math(const char * expr);

    /// For debugging purpose.
    std::size_t symbol_count();

    /// Return a string describing this object.
    std::string get_description() const;
};

}

#endif // XAPIAN_INCLUDED_MATHTERMGENERATOR_H
