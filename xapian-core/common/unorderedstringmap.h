/** @file
 * @brief std::unordered_map<std::string,T> with opportunistic transparent keys
 */
/* Copyright (C) 2024 Olly Betts
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

#ifndef XAPIAN_INCLUDED_UNORDEREDSTRINGMAP_H
#define XAPIAN_INCLUDED_UNORDEREDSTRINGMAP_H

#include <unordered_map>
#include <string>

#ifdef __cpp_lib_generic_unordered_lookup // C++20

#include <string_view>

struct string_hash {
    using is_transparent = void;

    // Support hashing anything that can be converted to a std::string_view.
    template<typename S>
    auto operator()(S&& s) const {
	return std::hash<std::string_view>{}(std::forward<S>(s));
    }
};

template<typename T>
using unordered_string_map =
    std::unordered_map<std::string, T, string_hash, std::equal_to<>>;

#define STRING_MAP_KEY(KEY) KEY

#else

template<typename T>
using unordered_string_map =
    std::unordered_map<std::string, T>;

#define STRING_MAP_KEY(KEY) std::string(KEY)

#endif

#endif // XAPIAN_INCLUDED_UNORDEREDSTRINGMAP_H
