/** @file
 *  @brief Calculated bounds on and estimate of number of matches
 */
/* Copyright (C) 2022,2025,2026 Olly Betts
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

#include <memory>

#include "xapian/types.h"

#include "api/smallvector.h"
#include "omassert.h"
#include "backends/postlist.h"

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

/** Class for estimating the total number of matching documents.
 *
 *  We build a tree of EstimateOp objects which largely follows the Query
 *  tree/PostList tree, but there are some differences.  Here we only care
 *  about the number of matches, so operations which only affect ordering are
 *  equivalent, so for example, we use OR here for any of OP_OR, OP_SYNONYM
 *  and OP_MAX; OP_AND_MAYBE and its RHS are omitted.
 */
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

    Xapian::VecUniquePtr<EstimateOp> sub_estimates;

  public:
    /// Leaf term.
    EstimateOp(Xapian::doccount tf_,
	       Xapian::docid first, Xapian::docid last)
	: type(KNOWN), estimates(tf_, tf_, tf_, first, last) { }

    /// PostingSource
    EstimateOp()
	: type(POSTING_SOURCE) { }

    /// Value range.
    EstimateOp(Estimates estimates_)
	: type(KNOWN), estimates(estimates_) { }

    /// Value range degenerate case.
    EstimateOp(Xapian::doccount tf_)
	: type(KNOWN), estimates(tf_, tf_, tf_) { }

    /// AND, AND_NOT, OR or XOR.
    EstimateOp(op_type type_, Xapian::docid first, Xapian::docid last,
	       Xapian::VecUniquePtr<EstimateOp>&& sub_estimates_)
	: type(type_), sub_estimates(std::move(sub_estimates_)) {
	estimates.first = first;
	estimates.last = last;
    }

    /// AND, AND_NOT, OR or XOR (pair-wise).
    EstimateOp(op_type type_, Xapian::docid first, Xapian::docid last,
	       std::unique_ptr<EstimateOp>&& est1,
	       std::unique_ptr<EstimateOp>&& est2)
	: type(type_) {
	sub_estimates.push_back(est1.release());
	sub_estimates.push_back(est2.release());
	estimates.first = first;
	estimates.last = last;
    }

    /** DECIDER, NEAR, PHRASE or EXACT_PHRASE.
     *
     *  These operate as filters so have a single subquery.
     */
    EstimateOp(op_type type_, EstimateOp* sub_estimate)
	: type(type_), estimates(0, 0, 0) {
	sub_estimates.push_back(sub_estimate);
    }

    /** Report the first docid indexed.
     *
     *  Called by ValueRangePostList if it starts with next().
     */
    void report_first(Xapian::docid first) {
	estimates.first = first;
    }

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

	// Degenerate range case.
	if (estimates.min == estimates.max) return;

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

    unsigned get_subquery_count() const { return sub_estimates.size(); }
};

namespace Xapian::Internal {

struct PostListAndEstimate {
    // Note that pl may hold a pointer to est and pl's destructor may try to
    // report stats via that pointer, so the order of destruction of pl and
    // est matters.  We don't currently enforce that here, but instead the
    // stats reporting is skipped if no matching has yet occurred, which is
    // enough to avoid calls to a deleted est (after matching we carefully
    // arrange to destroy the postlist tree before the estimates).
    //
    // FIXME: It would be cleaner to enforce that pl gets destroyed first here,
    // but that seems to require that this struct owns the pl which requires a
    // significant refactor.
    PostList* pl = nullptr;

    std::unique_ptr<EstimateOp> est;

    PostListAndEstimate() { }

    PostListAndEstimate(PostList* pl_, EstimateOp* est_)
	: pl(pl_), est(est_) { }

    PostListAndEstimate(PostList* pl_, std::unique_ptr<EstimateOp>&& est_)
	: pl(pl_), est(std::move(est_)) { }
};

}

using Xapian::Internal::PostListAndEstimate;

#endif // XAPIAN_INCLUDED_ESTIMATEOP_H
