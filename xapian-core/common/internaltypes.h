/** @file
 * @brief Types used internally.
 */
/* Copyright (C) 2009,2010,2014,2016 Olly Betts
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

#include <cstdint>

// The standard marks these types as optional, as an implementation may not
// directly support a type of the appropriate width.  If there are platforms
// we care about which lack them, we could use wider types with some care
// around where we read and write them.

typedef uint32_t uint4;

#endif // XAPIAN_INCLUDED_INTERNALTYPES_H
