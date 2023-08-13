/** @file
 * @brief Count the number of set bits in an integer type
 */
/* Copyright (C) 2014-2020 Olly Betts
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

#ifndef XAPIAN_INCLUDED_POPCOUNT_H
#define XAPIAN_INCLUDED_POPCOUNT_H

#ifndef PACKAGE
# error config.h must be included first in each C++ source file
#endif

#if !HAVE_DECL___BUILTIN_POPCOUNT
// Only include <intrin.h> if we have to as it can result in warnings about
// duplicate declarations of builtin functions under mingw.
# if HAVE_DECL___POPCNT || HAVE_DECL___POPCNT64
#  include <intrin.h>
# endif
#endif

/// Add the number of set bits in value to accumulator.
template<typename A, typename V>
static inline void
add_popcount(A& accumulator, V value)
{
    if (false) {
#if HAVE_DECL___BUILTIN_POPCOUNT
    } else if constexpr(sizeof(V) == sizeof(unsigned)) {
	accumulator += __builtin_popcount(value);
#elif HAVE_DECL___POPCNT
    } else if constexpr(sizeof(V) == sizeof(unsigned)) {
	accumulator += static_cast<A>(__popcnt(value));
#endif
#if HAVE_DECL___BUILTIN_POPCOUNTL
    } else if constexpr(sizeof(V) == sizeof(unsigned long)) {
	accumulator += __builtin_popcountl(value);
#endif
#if HAVE_DECL___BUILTIN_POPCOUNTLL
    } else if constexpr(sizeof(V) == sizeof(unsigned long long)) {
	accumulator += __builtin_popcountll(value);
#elif HAVE_DECL___POPCNT64
    } else if constexpr(sizeof(V) == sizeof(unsigned long long)) {
	accumulator += static_cast<A>(__popcnt64(value));
#endif
    } else {
	auto u = static_cast<std::make_unsigned<V>>(value);
	while (u) {
	    ++accumulator;
	    // Bit twiddling trick to unset the lowest set bit of u.
	    u &= u - 1;
	}
    }
}

/// Count the number of set bits in value.
template<typename V>
static unsigned
popcount(V value)
{
    unsigned accumulator = 0;
    add_popcount(accumulator, value);
    return accumulator;
}

#endif // XAPIAN_INCLUDED_POPCOUNT_H
