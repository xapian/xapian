/** @file exp10.h
 * @brief Define exp10() if not provided by <cmath>
 */
/* Copyright (C) 2017 Olly Betts
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

#ifndef XAPIAN_INCLUDED_EXP10_H
#define XAPIAN_INCLUDED_EXP10_H

#ifndef PACKAGE
# error config.h must be included first in each C++ source file
#endif

#include <cmath>
#ifndef HAVE_EXP10
# ifdef HAVE___EXP10
inline double exp10(double x) { return __exp10(x); }
# elif defined HAVE___BUILTIN_EXP10
inline double exp10(double x) { return __builtin_exp10(x); }
# else
inline double exp10(double x) { return std::pow(10.0, x); }
#endif
#endif

#endif // XAPIAN_INCLUDED_EXP10_H
