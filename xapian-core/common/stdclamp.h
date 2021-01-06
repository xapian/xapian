/** @file
 * @brief Defines STD_CLAMP like C++17's std::clamp
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

#ifndef XAPIAN_INCLUDED_STDCLAMP_H
#define XAPIAN_INCLUDED_STDCLAMP_H

#include <algorithm>
#include <functional>

#if __cplusplus >= 201703L
# define STD_CLAMP std::clamp
#else
template<typename T, typename CMP>
constexpr const T&
STD_CLAMP(const T& value, const T& lower, const T& upper, CMP cmp)
{
    return cmp(value, lower) ? lower : (cmp(upper, value) ? upper : value);
}

template<typename T>
constexpr const T&
STD_CLAMP(const T& value, const T& lower, const T& upper)
{
    return STD_CLAMP(value, lower, upper, std::less<T>());
}
#endif

#endif // XAPIAN_INCLUDED_STDCLAMP_H
