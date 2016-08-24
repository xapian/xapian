/** @file euclidian_sim.cc
 *  @brief Euclidian similarity calculation between documents
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

#include <debuglog.h>

#include "points.h"
#include "similarity.h"

#include <cmath>

using namespace std;
using namespace Xapian;

string
EuclidianDistance::get_description() const {
    LOGCALL(API, string, "EuclidianDistance::get_description()", NO_ARGS);
    return "Euclidian Distance metric";
}

double
EuclidianDistance::similarity(PointType &a, PointType &b) const {
    LOGCALL(API, double, "EuclidianDistance::similarity()", a | b);
    double sum = 0;
    TermIterator it = a.termlist_begin();
    for (; it != a.termlist_end(); it++) {
	if (a.contains(*it) && b.contains(*it)) {
	    double a_val = a.get_value(*it);
	    double b_val = b.get_value(*it);
	    sum += (a_val - b_val)*(a_val - b_val);
	}
	else
	    sum += a.get_value(*it)*a.get_value(*it);
    }
    it = b.termlist_begin();
    for (; it != b.termlist_end(); it++) {
	if (!a.contains(*it))
	    sum += b.get_value(*it)*b.get_value(*it);
    }
    return sqrt(sum);
}
