/** @file roundestimate.h
 * @brief Round a bounded estimate to an appropriate number of S.F.
 */
/* Copyright 2017 Olly Betts
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
round_estimate(T m, T M, T e)
{
    using namespace std;

    T D = M - m;
    if (D == 0 || e == 0) {
	// Estimate is exact or zero.  A zero but non-exact estimate can happen
	// with get_mset(0, 0).
	return e;
    }

    T r = T(exp10(int(log10(D))) + 0.5);
    while (r > e) r /= 10;

    T R = e / r * r;
    if (R < m) {
	R += r;
    } else if (R > M) {
	R -= r;
    } else if (R < e && r % 2 == 0 && e - R == r / 2) {
	// Round towards the centre of the range.
	if (e - m < M - e) {
	    R += r;
	}
    }

    // If it all goes pear-shaped, just stick to the original estimate.
    if (R < m || R > M) R = e;
    return R;
}

#endif // XAPIAN_INCLUDED_ROUNDESTIMATE_H
