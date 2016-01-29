/** @file log2.h
 * @brief Defines a log2() function to find the logarithm to base 2 if not already defined in the library.
 */
/* Copyright (C) 2013 Aarsh Shah
 * Copyright (C) 2014,2015 Olly Betts
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

#ifndef XAPIAN_INCLUDED_LOG2_H
#define XAPIAN_INCLUDED_LOG2_H

#ifndef PACKAGE
# error config.h must be included first in each C++ source file
#endif

#include <cmath>
#if !HAVE_DECL_LOG2
inline double log2(double x) { return std::log(x) / std::log(2.0); }
#endif

#endif // XAPIAN_INCLUDED_LOG2_H
