/** @file exp10.h
 * @brief Defines an exp10() function depending on how the system defines it.
 */
/* Copyright (C) 2017 Sébastien Le Callonnec
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

#include <cmath>
#ifdef HAVE_DECL_GNU_EXP10
#define EXP10(X) exp10(X)
#elif defined HAVE_DECL_APPLE_EXP10
#define EXP10(X) __exp10(X)
#else
#define EXP10(X) pow(10, X)
#endif

#endif // XAPIAN_INCLUDED_EXP10_H
