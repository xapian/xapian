/** @file omassert.cc
 * @brief Helper functions for omassert.h
 */
/* Copyright (C) 2007,2012 Olly Betts
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

#ifdef XAPIAN_ASSERTIONS

#include "omassert.h"

#include <cfloat>
#include <cmath>

using namespace std;

namespace Xapian {
namespace Internal {

bool within_DBL_EPSILON(double a, double b) {
    return fabs(a - b) < DBL_EPSILON;
}

}
}

#endif
