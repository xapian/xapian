/** @file internaltypes.h
 * @brief Types used internally.
 */
/* Copyright (C) 2009,2010,2014 Olly Betts
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

#ifndef XAPIAN_INCLUDED_INTERNALTYPES_H
#define XAPIAN_INCLUDED_INTERNALTYPES_H

#include <climits>

/// Unsigned 8 bit type.
typedef unsigned char byte;

#ifndef SIZEOF_SHORT
# error SIZEOF_SHORT is not defined
#endif
#ifndef SIZEOF_INT
# error SIZEOF_INT is not defined
#endif
#ifndef SIZEOF_LONG
# error SIZEOF_LONG is not defined
#endif

#if SIZEOF_SHORT * CHAR_BIT == 16
typedef unsigned short uint2;
#else
# error Type short is more than 16 bits, which Xapian does not currently handle
#endif

#if SIZEOF_INT * CHAR_BIT >= 32
typedef unsigned int uint4;
#elif SIZEOF_LONG * CHAR_BIT >= 32
typedef unsigned long uint4;
#else
# error Type long is less than 32 bits, which ISO does not allow!
#endif

#if SIZEOF_LONG * CHAR_BIT >= 64
typedef unsigned long uint8;
#else
/* C99 and C++11 actually standardised "long long", but it seems to be widely
 * supported (e.g. GCC has supported it for ages, MSVC since at least 2003 it
 * appears) and we've not had any reports of compilation failing due to lack of
 * support for it.
 */
typedef unsigned long long uint8;
#endif

/// Integer type used to hold the total length of all documents in a database.
typedef uint8 totlen_t;

#endif // XAPIAN_INCLUDED_INTERNALTYPES_H
