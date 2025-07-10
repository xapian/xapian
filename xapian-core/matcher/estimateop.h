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

#include "omassert.h"

struct Estimates {
    Xapian::doccount min, est, max;

    /** Lower bound on docids matched. */
    Xapian::docid first;

    /** Upper bound on docids matched. */
    Xapian::docid last;

    Estimates() { }

    Estimates(Xapian::doccount min_,
	      Xapian::doccount est_,
	      Xapian::doccount max_,
	      Xapian::doccount first_ = 1,
	      Xapian::doccount last_ = Xapian::docid(-1))
	: min(min_), est(est_), max(max_), first(first_), last(last_) { }
};

// Clean up Microsoft namespace pollution.
#ifdef NEAR
# undef NEAR
#endif

class EstimateOp {
  public:
    enum op_type {
	KNOWN,
	// In the absence of accept/reject counts we just scale the AND by
	// dividing by these values:
	DECIDER = 1,
	NEAR = 2, PHRASE = 3, EXACT_PHRASE = 4,
	POSTING_SOURCE,
	AND, AND_NOT, OR, XOR
    };

  private:
    EstimateOp* next;

    op_type type;

    /** Estimates.
     *
     *  * KNOWN: Already known exact leaf term frequency (in min/est/max)
     *    or value range min/est/max based on static information (which is
     *    adjusted by a call to report_range_ratio()).
     *  * DECIDER/NEAR/PHRASE/EXACT_PHRASE: Filled in on PostList deletion by
     *    calling report_ratio(): min = accepted, max = rejected
     *  * POSTING_SOURCE: Filled in on PostList construction with min/est/max
     *    from PostingSource object.
     */
    Estimates estimates;

    /** Used by get_subquery_count().
     *
     *  Set to zero for leaf operators.
     */
    unsigned n_subqueries = 0;

  public:
    /// Leaf term.
    EstimateOp(EstimateOp* next_, Xapian::doccount tf_,
	       Xapian::docid first, Xapian::docid last)
	: next(next_), type(KNOWN), estimates(tf_, tf_, tf_, first, last) { }

    /// PostingSource
    EstimateOp(EstimateOp* next_)
	: next(next_), type(POSTING_SOURCE) { }

    /// Value range.
    EstimateOp(EstimateOp* next_, Estimates estimates_)
	: next(next_), type(KNOWN), estimates(estimates_) { }

    /// Value range degenerate case.
    EstimateOp(EstimateOp* next_, Xapian::doccount tf_)
	: next(next_), type(KNOWN), estimates(tf_, tf_, tf_) { }

    /// AND, AND_NOT, OR or XOR.
    EstimateOp(EstimateOp* next_, op_type type_, unsigned n_subqueries_,
	       Xapian::docid first, Xapian::docid last)
	: next(next_), type(type_), n_subqueries(n_subqueries_) {
	estimates.first = first;
	estimates.last = last;
    }

    /** DECIDER, NEAR, PHRASE or EXACT_PHRASE.
     *
     *  These operate as filters so have a single subquery.
     */
    EstimateOp(EstimateOp* next_, op_type type_)
	: next(next_), type(type_), estimates(0, 0, 0), n_subqueries(1) { }

    void report_ratio(Xapian::doccount accepted, Xapian::doccount rejected) {
	Assert(type == DECIDER ||
	       type == NEAR ||
	       type == PHRASE ||
	       type == EXACT_PHRASE);
	// Store ratio to use later.
	estimates.min = accepted;
	estimates.max = rejected;
    }

    /** Adjust static estimates for value range. */
    void report_range_ratio(Xapian::doccount accepted,
			    Xapian::doccount rejected) {
	AssertEq(type, KNOWN);
	AssertEq(estimates.first, 1);
	AssertEq(estimates.last, Xapian::docid(-1));

	// The static min is 0.
	AssertEq(estimates.min, 0);
	estimates.min = accepted;

	// Combine the static estimate already in estimate.est with a dynamic
	// estimate based on accepted/rejected ratio using a weighted average
	// based on the proportion of value_freq actually looked at.
	auto est = double(estimates.max - accepted - rejected);
	est = est / estimates.max * estimates.est;
	estimates.est = accepted + Xapian::doccount(est + 0.5);

	// The static max is the value slot frequency, so every reject can
	// be removed from that.
	estimates.max -= rejected;

	// We shouldn't need to clamp here to ensure the invariant.
	AssertRel(estimates.min, <=, estimates.est);
	AssertRel(estimates.est, <=, estimates.max);
    }

    /** Fill in estimates for POSTING_SOURCE. */
    void report_termfreqs(Xapian::doccount min_,
			  Xapian::doccount est,
			  Xapian::doccount max_) {
	AssertEq(type, POSTING_SOURCE);
	estimates.min = min_;
	estimates.est = est;
	estimates.max = max_;
	estimates.first = 1;
	estimates.last = Xapian::docid(-1);
    }

    Estimates resolve(Xapian::doccount db_size,
		      Xapian::docid db_first,
		      Xapian::docid db_last);

    Estimates resolve_next(Xapian::doccount db_size,
			   Xapian::docid db_first,
			   Xapian::docid db_last) {
	Estimates result = next->resolve(db_size, db_first, db_last);
	EstimateOp* old_next = next;
	next = next->next;
	delete old_next;
	return result;
    }

    EstimateOp* get_next() const { return next; }

    unsigned get_subquery_count() const { return n_subqueries; }
};

#endif // XAPIAN_INCLUDED_ESTIMATEOP_H
