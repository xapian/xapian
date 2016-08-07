/** @file euclidian_sim.cc
 *  @brief Document similarity calculation
 */
/* Copyright (C) 2010 Richard Boulton
 * Copyright (C) 2016 Richhiey Thomas
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
#include <api/termlist.h>

#include <vector>
#include <map>
#include <cmath>

using namespace std;
using namespace Xapian;

string
EuclidianDistance::get_description() {
    LOGCALL(API, string, "EuclidianDistance::get_description()", "");
    return "Euclidian Distance metric";
}

/** TODO
 *  Implement the euclidian distance metric for the current API
 */
double
EuclidianDistance::similarity(Point a, Point b) {
    return 0.0;
}
