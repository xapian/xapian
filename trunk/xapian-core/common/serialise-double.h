/* @file serialise-double.h
 * @brief functions to serialise and unserialise a double
 *
 * Copyright (C) 2006 Olly Betts
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

#ifndef XAPIAN_INCLUDED_SERIALISE_DOUBLE_H
#define XAPIAN_INCLUDED_SERIALISE_DOUBLE_H

#include <xapian/visibility.h>

#include <string>

/** Serialise a double to a string.
 *
 *  @param v	The double to serialise.
 *
 *  @return	Serialisation of @a v.
 */
XAPIAN_VISIBILITY_DEFAULT
std::string serialise_double(double v);

/** Unserialise a double serialised by serialise_double.
 *
 *  @param p	Pointer to a pointer to the string, which will be advanced past
 *		the serialised double.
 *  @param end	Pointer to the end of the string.
 *
 *  @return	The unserialised double.
 */
XAPIAN_VISIBILITY_DEFAULT
double unserialise_double(const char ** p, const char *end);

#endif
