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

    /// Set the current document.
    void set_document(const Xapian::Document & doc);

    /// Get the current document.
    const Xapian::Document & get_document() const;

    /** Index math expression in std::string.
     *
     * Currently there is only support for indexing math expression in
     * Presentation MathML format.
     *
     * @param expr	The expression to index.
     * @param prefix	The term prefix to use (default is no prefix).
     */
    void index_math(const std::string & expr,
		    const std::string & prefix = std::string());

    /** Generate and return symbol pair list from math expression in string.
     *
     * This method creates list of symbol pair tuple from the symbol layout
     * tree.
     *
     * consider Symbol layout tree with spatial relationship annotated on the
     * edges (B - Below, N - Next):
     *
     * 		(V!a)--N--(O+)--N--(V!b)
     * 		 |
     * 		 B
     * 		 |
     * 		(V!x)
     *
     * 	Generated Symbol pair list: [(V!aV!xB), (V!aO+N), (V!aV!bNN), (O+V!bN)]
     *
     * @param expr	The expression to generate symbol pair. The expression
     * 			must be in presentation mathml format.
     */
    std::vector<std::string> get_symbol_pair_list(const std::string & expr);

    /** Get the math symbols list in a given symbol layout tree.
     *
     * This method can be used to investigate the math symbols generated from
     * mathml expression. Each math symbol represents a node in symbol layout
     * tree.
     *
     * For example, consider expression:
     * 	<math>
     * 		<mi> a </mi>
     * 		<mo> + </mo>
     * 		<mi> b </mi>
     * 	</math>
     *
     * 	Symbol list: [ (V!a), (O+), (V!b) ]
     */
    std::vector<std::string> get_labels_list();

    /** Set the condition to generate unified terms during symbol pair
     * generation.
     */
    void set_unification_op(const bool unify);

    /** Returns true if formula encountered is invalid.
     *
     * MathML parser generates math terms while parsing along the expression.
     * If at any point invalid structure (for ex. missing closed tag) encounted,
     * error flag is set, but this doesn't throw any exception, instead indexing
     * operation terminated silently. With this method, user can verfiy indexing
     * of complete expression is performed or not.
     */
    bool parse_error();

    /// Return a string describing this object.
    std::string get_description() const;
};

}

#endif // XAPIAN_INCLUDED_MATHTERMGENERATOR_H
