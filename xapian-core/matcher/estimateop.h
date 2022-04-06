/** @file
 *  @brief Calculated bounds on and estimate of number of matches
 */
/* Copyright (C) 2022 Olly Betts
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

#ifndef XAPIAN_INCLUDED_ESTIMATEOP_H
#define XAPIAN_INCLUDED_ESTIMATEOP_H

#include "xapian/types.h"

struct Estimates {
    Xapian::doccount min, est, max;

    Estimates() { }

    Estimates(Xapian::doccount min_,
	      Xapian::doccount est_,
	      Xapian::doccount max_)
	: min(min_), est(est_), max(max_) { }
};

class EstimateOp {
  public:
    enum op_type {
	KNOWN, AND, AND_NOT, OR, XOR, NEAR, PHRASE, EXACT_PHRASE
    };

  private:
    EstimateOp* next;

    op_type type;

    /** Used for KNOWN. */
    Estimates estimates;

    /** Used by get_subquery_count().
     *
     *  Set to a non-zero value for AND, AND_NOT, OR and XOR.
     */
    unsigned n_subqueries = 0;

  public:
    // Leaf term.
    EstimateOp(EstimateOp* next_, Xapian::doccount tf_)
	: next(next_), type(KNOWN), estimates(tf_, tf_, tf_) { }

    // Value range or PostingSource.
    EstimateOp(EstimateOp* next_,
	       Xapian::doccount min_,
	       Xapian::doccount est,
	       Xapian::doccount max_)
	: next(next_), type(KNOWN), estimates(min_, est, max_) { }

    // AND, AND_NOT, OR or XOR.
    EstimateOp(EstimateOp* next_, op_type type_, unsigned n_subqueries_)
	: next(next_), type(type_), n_subqueries(n_subqueries_) { }

    // NEAR, PHRASE or EXACT_PHRASE.
    EstimateOp(EstimateOp* next_, op_type type_)
	: next(next_), type(type_) { }

    Estimates resolve(Xapian::doccount db_size);

    Estimates resolve_next(Xapian::doccount db_size) {
	Estimates result = next->resolve(db_size);
	EstimateOp* old_next = next;
	next = next->next;
	delete old_next;
	return result;
    }

    EstimateOp* get_next() const { return next; }

    unsigned get_subquery_count() const { return n_subqueries; }
};

#endif // XAPIAN_INCLUDED_ESTIMATEOP_H
