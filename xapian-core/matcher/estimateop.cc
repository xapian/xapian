/** @file
 *  @brief Calculated bounds on and estimate of number of matches
 */
/* Copyright (C) 2022,2026 Olly Betts
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

#include "estimateop.h"

#include "omassert.h"
#include "overflow.h"

#include <algorithm>
#include <memory>

using namespace std;

Estimates
EstimateOp::resolve(Xapian::doccount db_size,
		    Xapian::docid db_first,
		    Xapian::docid db_last)
{
    // Clamp first and last to those from the DB - we lazily set first=1 and/or
    // last=Xapian::docid(-1) when we don't have better bounds.
    estimates.first = max(estimates.first, db_first);
    estimates.last = min(estimates.last, db_last);

    // If there are gaps in the docid numbering then max_range could be greater
    // than db_size.  It might seem tempting to clamping max_range to db_size
    // but the gaps will tend to inflate all the range lengths proportionally
    // so the effect should tend to cancel out if we leave max_range alone.
    Xapian::doccount max_range = estimates.last - estimates.first + 1;

    Estimates result;
    // LocalSubMatch::resolve() checks db_size == 0 so we should be able to
    // assume it is non-zero in all the cases below.
    Assert(db_size);
    switch (type) {
      case KNOWN:
      case POSTING_SOURCE:
	result = estimates;
	break;
      case AND: {
	result.min = max_range;
	result.max = max_range;
	double est = max_range;
	auto n_subqueries = sub_estimates.size();
	for (unsigned n = 0; n < n_subqueries; ++n) {
	    Estimates r = sub_estimates[n]->resolve(db_size, db_first, db_last);

	    // The number of matching documents is minimised when we have the
	    // minimum number of matching documents from each sub-postlist, and
	    // these are maximally disjoint.
	    auto range_i = r.last - r.first + 1;
	    // min_overlap is the minimum number of documents from this subquery
	    // which have to be in the range where all the subqueries overlap.
	    Xapian::doccount min_overlap;
	    if (!sub_overflows(r.min, range_i - max_range, min_overlap)) {
		if (!add_overflows(result.min, min_overlap, result.min) &&
		    result.min <= max_range) {
		    // It's possible there's no overlap.
		    result.min = 0;
		} else {
		    // The pair-wise overlap is (a + b - max_range) - this works
		    // even if (a + b) overflows (a and b are each <= max_range
		    // so subtracting max_range must un-overflow us).
		    result.min -= max_range;
		}
	    } else {
		// This child could have nothing in the overlap.
		result.min = 0;
	    }

	    result.max = std::min(result.max, r.max);

	    // We calculate the estimate assuming independence - it's the
	    // product of (est/range) for the children, multiplied by the
	    // range for the AND.
	    if (usual(range_i > 0)) {
		est = est * r.est / range_i;
	    }
	}

	result.est = static_cast<Xapian::doccount>(est + 0.5);
	result.est = std::clamp(result.est, result.min, result.max);
	break;
      }
      case AND_NOT: {
	AssertEq(sub_estimates.size(), 2);
	result = sub_estimates[0]->resolve(db_size, db_first, db_last);
	Estimates r = sub_estimates[1]->resolve(db_size, db_first, db_last);

	// An AND_NOT with no overlap gets optimised away before now, so we
	// know the ranges overlap.
	Xapian::doccount overlap =
	    min(result.last, r.last) - max(result.first, r.first) + 1;

	// The maximum number of documents that could be in common.
	auto max_actual_overlap = min(overlap, r.max);

	if (sub_overflows(result.min, max_actual_overlap, result.min)) {
	    // This AND_NOT could match no documents.
	    result.min = 0;
	}

	Xapian::doccount range_l = result.last - result.first + 1;
	Xapian::doccount range_r = r.last - r.first + 1;
	// We can't match more documents than our left-side does.
	// We also can't more documents than when the non-overlaps are
	// full and the overlap as segregated as possible with the LHS
	// achieving its maximum and the RHS its minimum.
	//
	// Note that the second expression can't overflow the type.
	result.max = std::min(result.max, range_l - overlap + range_r - r.min);

	// Max is when:
	// <---- LHS ------->
	// [LLLLLLL|llllbbrr|RRRRRR]
	//         <-------RHS----->
	//
	// #L = range_l - overlap
	// #R = range_r - overlap
	// #r+#b = r_min - #R
	// #l+#b = l_max - #L
	// #b = rmin - #R + l_max - #L - overlap
	// result.max = #L + #l = #L + l_max - #L -
	//           - (rmin - #R + l_max - #L - overlap)
	//            = - rmin + #R + #L + overlap)
	//            = #L + #R + overlap - rmin
	//            = range_l + range_r - overlap - rmin

	// We calculate the estimate assuming independence, but taking into
	// account the range information.
	double est = result.est * (1.0 - double(overlap) * r.est /
		     (double(range_l) * range_r));

	result.est = static_cast<Xapian::doccount>(est + 0.5);
	break;
      }
      case OR: {
	// FIXME: We don't currently make full use of the range information
	// here.  Trivial testing suggested it doesn't make much difference
	// for the estimate but we should investigate more deeply as it may
	// be useful for min and max, and perhaps for the estimate in cases
	// we didn't try.
	//
	// We also only tried applying the range version of
	// estimate_or_assuming_indep() from orpostlist.cc which works on
	// pairswise OR multiple times.  A fuller algorithm would work
	// through the range starts and ends something like:
	//
	// * Sort start and ends into order and work through (at most 2*N-1
	//   regions)
	// * For min allocate to where there's most overlap, e.g.
	//
	//     AAAAAAAAAAAAAAAAAAAAAAA
	// |   |      | aaaaaaa|aaaaa|        |       |       |
	// |   |      BBBBBBBBBBBBBBBBBBBBBBBBB       |       |
	// |   |      | bbbbbbb|bbbbb|bbbbbb  |       |       |
	// |   |      |        CCCCCCCCCCCCCCCCCCCCCCCC       |
	// |   |      |        |ccccc|ccc     |       |       |
	//
	// * For max try to allocate where there's least overlap
	// * For est assume each range splits proportionally over the regions
	//   it participates (potentially O(n*n) in number of subqueries)
	result = sub_estimates[0]->resolve(db_size, db_first, db_last);
	double scale = max_range == 0.0 ? 1.0 : 1.0 / max_range;
	double P_est = result.est * scale;
	auto n_subqueries = sub_estimates.size();
	for (unsigned i = 1; i != n_subqueries; ++i) {
	    Estimates r = sub_estimates[i]->resolve(db_size, db_first, db_last);

	    result.min = std::max(result.min, r.min);

	    // Sum(max) over subqueries but saturating at max_range.
	    if (max_range - result.max <= r.max) {
		result.max = max_range;
	    } else {
		result.max += r.max;
	    }

	    // We calculate the estimate assuming independence.  The simplest
	    // way to calculate this seems to be a series of (n_subqueries - 1)
	    // pairwise calculations, which gives the same answer regardless of
	    // the order.
	    double P_i = r.est * scale;
	    P_est += P_i - P_est * P_i;
	}

	result.est = static_cast<Xapian::doccount>(P_est * max_range + 0.5);
	break;
      }
      case XOR: {
	// FIXME: We don't currently make full use of the range information
	// here.
	double scale = max_range == 0.0 ? 1.0 : 1.0 / max_range;

	bool all_exact = true;
	bool invert = false;

	unsigned max_overflow = 0;
	Xapian::doccount max_sum = 0;

	// We calculate the estimate assuming independence.  The simplest
	// way to calculate this seems to be a series of (n_subqueries - 1)
	// pairwise calculations, which gives the same answer regardless of the
	// order.
	double P_est = 0.0;

	unsigned j = 0;

	// Need to buffer min and max values from subqueries so we can compute
	// our min as a second pass.
	auto n_subqueries = sub_estimates.size();
	unique_ptr<Estimates[]> min_and_max{new Estimates[n_subqueries]};
	for (unsigned i = 0; i < n_subqueries; ++i) {
	    min_and_max[j] =
		sub_estimates[i]->resolve(db_size, db_first, db_last);
	    const Estimates& r = min_and_max[j];

	    if (r.min == db_size) {
		// A subquery matches all documents which just inverts which
		// documents match - note that but otherwise ignore this
		// subquery.
		invert = !invert;
	    } else {
		// Maximum is if all sub-postlists are disjoint.
		Xapian::doccount max_i = r.max;
		if (add_overflows(max_sum, max_i, max_sum)) {
		    // Track how many times we overflow the type.
		    ++max_overflow;
		}
		all_exact = all_exact && (max_i == r.min);

		double P_i = r.est * scale;
		P_est += P_i - 2.0 * P_est * P_i;

		++j;
	    }
	}

	if (max_overflow || max_sum > db_size) {
	    if (all_exact) {
		// If the sub-postlist tfs are all exact, then if the sum of
		// them has a different odd/even-ness to db_size then max tf of
		// the XOR can't achieve db_size.
		result.max = db_size - ((max_sum & 1) != (db_size & 1));
	    } else {
		result.max = db_size;
	    }
	} else {
	    result.max = max_sum;
	}

	result.est = static_cast<Xapian::doccount>(P_est * max_range + 0.5);

	// Calculate min.
	Xapian::doccount min = 0;
	if (max_overflow <= 1) {
	    // If min_i(i) > sum(j!=i)(max_i(j)) then all the other subqueries
	    // can't cancel out subquery i.  If we overflowed more than once,
	    // then the true value of max_sum is greater than the maximum
	    // possible tf, so there's no point checking.
	    for (unsigned i = 0; i < j; ++i) {
		const Estimates& r = min_and_max[i];
		Xapian::doccount all_the_rest = max_sum - r.max;
		// If no overflow, or we un-overflowed again...
		if (max_overflow == 0 || all_the_rest > max_sum) {
		    if (r.min > all_the_rest) {
			min = std::max(min, r.min - all_the_rest);
		    }
		}
	    }
	}
	if (all_exact && min == 0) {
	    // If all the subqueries termfreqs are exact then the parity of
	    // result.min must match the parity of max_sum.  That's already
	    // ensured unless min was clamped to zero in which case result.min
	    // should be 1 if max_sum is odd.
	    result.min = max_sum & 1;
	} else {
	    result.min = min;
	}

	if (invert) {
	    auto old_min = result.min;
	    result.min = db_size - result.max;
	    result.est = db_size - result.est;
	    result.max = db_size - old_min;
	}
	break;
      }
      case DECIDER:
      case NEAR:
      case PHRASE:
      case EXACT_PHRASE: {
	result = sub_estimates[0]->resolve(db_size, db_first, db_last);
	if (estimates.min == 0 && estimates.max == 0) {
	    result.min = 0;
	    // We've arranged for type's value to be the factor we want to
	    // scale by in the absence of accept/reject counts.
	    result.est /= unsigned(type);
	    break;
	}
	result.min = estimates.min;
	result.max -= estimates.max;
	double scale = double(estimates.min) / (estimates.min + estimates.max);
	result.est = Xapian::doccount(result.est * scale + 0.5);
	result.est = std::clamp(result.est, result.min, result.max);
	break;
      }
    }
    AssertRel(result.min, <=, result.est);
    AssertRel(result.est, <=, result.max);

    result.first = estimates.first;
    result.last = estimates.last;

    // The range size is an upper bound for max and est.  This is redundant in
    // some cases (for example it isn't needed for AND) but it's very cheap and
    // simpler to always ensure this.
    if (max_range < result.max) {
	result.max = max_range;
	if (max_range < result.est) {
	    result.est = max_range;
	}
    }
    return result;
}
