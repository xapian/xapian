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

#include <config.h>

#include "estimateop.h"

#include "omassert.h"

#include <algorithm>
#include <memory>

using namespace std;

Estimates
EstimateOp::resolve(Xapian::doccount db_size)
{
    Estimates result;
    // LocalSubMatch::resolve() checks db_size == 0 so we should be able to
    // assume it is non-zero in all the cases below.
    Assert(db_size);
    switch (type) {
      case KNOWN:
	result = estimates;
	break;
      case AND: {
	result = resolve_next(db_size);
	double scale = 1.0 / db_size;
	double est = result.est;
	auto n = n_subqueries;
	while (--n) {
	    Estimates r = resolve_next(db_size);

	    // The number of matching documents is minimised when we have the
	    // minimum number of matching documents from each sub-postlist, and
	    // these are maximally disjoint.
	    auto old_min = result.min;
	    result.min += r.min;
	    // If result.min < old_min then the calculation overflowed and the
	    // true sum must be > db_size.
	    if (result.min >= old_min && result.min <= db_size) {
		// It's possible there's no overlap.
		result.min = 0;
	    } else {
		// The pair-wise overlap is (a + b - db_size) - this works even
		// if (a + b) overflows (a and b are each <= db_size so
		// subtracting db_size must un-overflow us).
		result.min -= db_size;
	    }

	    result.max = std::min(result.max, r.max);

	    // We calculate the estimate assuming independence.  With this
	    // assumption, the estimate is the product of the estimates for the
	    // sub-postlists divided by db_size (n_subqueries - 1) times.
	    est = est * r.est * scale;
	}

	result.est = static_cast<Xapian::doccount>(est + 0.5);
	break;
      }
      case AND_NOT: {
	AssertEq(n_subqueries, 2);
	// NB: The RHS comes first, then the LHS.
	Estimates r = resolve_next(db_size);
	result = resolve_next(db_size);

	if (result.min <= r.max) {
	    result.min = 0;
	} else {
	    result.min -= r.max;
	}

	// We can't match more documents than our left-side does.
	// We also can't more documents than our right-side *doesn't*.
	result.max = std::min(result.max, db_size - r.min);

	// We calculate the estimate assuming independence.  With this
	// assumption, the estimate is the product of the estimates for the
	// sub-postlists (for the right side this is inverted by subtracting
	// from db_size), divided by db_size.
	double est = result.est;
	est = (est * (db_size - r.est)) / db_size;
	result.est = static_cast<Xapian::doccount>(est + 0.5);
	break;
      }
      case OR: {
	result = resolve_next(db_size);
	double scale = 1.0 / db_size;
	double P_est = result.est * scale;
	auto n = n_subqueries;
	while (--n) {
	    Estimates r = resolve_next(db_size);

	    result.min = std::max(result.min, r.min);

	    // Sum(max) over subqueries but saturating at db_size.
	    if (db_size - result.max <= r.max) {
		result.max = db_size;
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

	result.est = static_cast<Xapian::doccount>(P_est * db_size + 0.5);
	break;
      }
      case XOR: {
	result = resolve_next(db_size);
	double scale = 1.0 / db_size;

	bool all_exact = (result.max == result.min);

	unsigned max_overflow = 0;
	Xapian::doccount max_sum = result.max;

	// We calculate the estimate assuming independence.  The simplest
	// way to calculate this seems to be a series of (n_subqueries - 1)
	// pairwise calculations, which gives the same answer regardless of the
	// order.
	double P_est = result.est * scale;

	// Need to buffer min and max values from subqueries so we can compute
	// our min as a second pass.
	unique_ptr<Estimates[]> min_and_max{new Estimates[n_subqueries]};
	min_and_max[0] = result;
	for (unsigned i = 1; i < n_subqueries; ++i) {
	    min_and_max[i] = resolve_next(db_size);
	    const Estimates& r = min_and_max[i];

	    // Maximum is if all sub-postlists are disjoint.
	    Xapian::doccount max_i = r.max;
	    Xapian::doccount old_max_sum = max_sum;
	    max_sum += max_i;
	    // Track how many times we overflow the type.
	    if (max_sum < old_max_sum)
		++max_overflow;
	    all_exact = all_exact && (max_i == r.min);

	    double P_i = r.est * scale;
	    P_est += P_i - 2.0 * P_est * P_i;
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

	result.est = static_cast<Xapian::doccount>(P_est * db_size + 0.5);

	// Calculate min.
	Xapian::doccount min = 0;
	if (max_overflow <= 1) {
	    // If min_i(i) > sum(j!=i)(max_i(j)) then all the other subqueries
	    // can't cancel out subquery i.  If we overflowed more than once,
	    // then the true value of max_sum is greater than the maximum
	    // possible tf, so there's no point checking.
	    for (unsigned i = 0; i < n_subqueries; ++i) {
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
	break;
      }
      case NEAR:
	result = resolve_next(db_size);
	result.min = 0;
	result.est /= 2;
	break;
      case PHRASE:
	result = resolve_next(db_size);
	result.min = 0;
	result.est /= 3;
	break;
      case EXACT_PHRASE:
	result = resolve_next(db_size);
	result.min = 0;
	result.est /= 4;
	break;
    }
    return result;
}
