/** @file cosine_sim.cc
 *  @brief Cosine similarity calculation between documents
 */
/* Copyright (C) 2016 Richhiey Thomas
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include "xapian/cluster.h"

#include "debuglog.h"

#include <cmath>

using namespace std;
using namespace Xapian;

string
CosineDistance::get_description() const
{
    return "CosineDistance()";
}

double
CosineDistance::similarity(const PointType &a, const PointType &b) const
{
    LOGCALL(API, double, "CosineDistance::similarity", a | b);
    double denom_a = a.get_magnitude();
    double denom_b = b.get_magnitude();
    double inner_product = 0;

    if (denom_a == 0 || denom_b == 0)
	return 0.0;

    for (TermIterator it = a.termlist_begin(); it != a.termlist_end(); ++it) {
	const string &term = *it;
	double a_weight = a.get_weight(term);
	if (a_weight == 0)
	    continue;
	double b_weight = b.get_weight(term);
	if (b_weight == 0)
	    continue;
	inner_product += a_weight * b_weight;
    }

    return 1 - (inner_product / (sqrt(denom_a * denom_b)));
}
