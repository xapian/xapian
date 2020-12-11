/** @file
 * @brief Round a bounded estimate to an appropriate number of S.F.
 */
/* Copyright 2017,2019 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
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

#ifndef XAPIAN_INCLUDED_ROUNDESTIMATE_H
#define XAPIAN_INCLUDED_ROUNDESTIMATE_H

#include <algorithm>
#include <cmath>
#include "exp10.h"

/** Round a bounded estimate to an appropriate number of S.F.
 *
 *  The algorithm used looks at the lower and upper bound and where the
 *  estimate sits between them and picks a number of significant figures
 *  which doesn't suggest much more precision than there is, while still
 *  providing a useful estimate.
 */
template<typename T>
inline Xapian::doccount
round_estimate(T lb, T ub, T est)
{
    using namespace std;

    // We round based on the difference between the bounds, or the estimate if
    // that's smaller - for example, consider lb=11, est=24, ub=1234 where
    // rounding est to a multiple of 10 is reasonable but rounding it to a
    // multiple of 1000 isn't.
    T scale = min(ub - lb, est);
    if (scale <= 10) {
	// Estimate is either too close to exact or too small to round.
	return est;
    }

    // Set r to the largest power of 10 <= scale.
    T r = T(exp10(int(log10(scale))) + 0.5);

    // Set result to est with less significant digits truncated.
    T result = est / r * r;
    if (result < lb) {
	// We have to round up to be above the lower bound.
	result += r;
    } else if (result > ub - r) {
	// We can't round up as it would exceed the upper bound.
    } else {
	// We can choose which way to round so consider whether we're before or
	// after the mid-point of [result, result+r] and round to the nearer
	// end of the range.  If we're exactly on the middle, pick the rounding
	// direction which puts the rounded estimate closest to the mid-range
	// of the bounds.
	T d = 2 * (est - result);
	if (d > r || (d == r && result - lb <= ub - r - result)) {
	    result += r;
	}
    }

    return result;
}

#endif // XAPIAN_INCLUDED_ROUNDESTIMATE_H
