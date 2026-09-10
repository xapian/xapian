/** @file
 * @brief Various handy string-related helpers
 */
/* Copyright (C) 2004-2023 Olly Betts
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
 * along with this program; if not, see
 * <https://www.gnu.org/licenses/>.
 */

#ifndef XAPIAN_INCLUDED_STRINGUTILS_H
#define XAPIAN_INCLUDED_STRINGUTILS_H

#include <algorithm>
#include <string>
#include <string_view>
#include <cstring>

/** Helper macro for STRINGIZE - the nested call is required because of how
 *  # works in macros.
 */
#define STRINGIZE_(X) #X

/// The STRINGIZE macro converts its parameter into a string constant.
#define STRINGIZE(X) STRINGIZE_(X)

/** Returns the length of a string constant.
 *
 *  We rely on concatenation of string literals to produce an error if this
 *  macro is applied to something other than a string literal.
 */
#define CONST_STRLEN(S) (sizeof(S"") - 1)

/* C++20 added starts_with(), ends_with() and contains() methods to std::string
 * and std::string_view which provide this functionality, but we don't yet
 * require C++20.
 */

inline bool
startswith(std::string_view s, char pfx)
{
    return !s.empty() && s[0] == pfx;
}

inline bool
startswith(std::string_view s, const char* pfx, size_t len)
{
    return s.size() >= len && (std::memcmp(s.data(), pfx, len) == 0);
}

inline bool
startswith(std::string_view s, const char* pfx)
{
    return startswith(s, pfx, std::strlen(pfx));
}

inline bool
startswith(std::string_view s, std::string_view pfx)
{
    return startswith(s, pfx.data(), pfx.size());
}

inline bool
endswith(std::string_view s, char sfx)
{
    return !s.empty() && s[s.size() - 1] == sfx;
}

inline bool
endswith(std::string_view s, const char* sfx, size_t len)
{
    return s.size() >= len && (std::memcmp(s.data() + s.size() - len, sfx, len) == 0);
}

inline bool
endswith(std::string_view s, const char* sfx)
{
    return endswith(s, sfx, std::strlen(sfx));
}

inline bool
endswith(std::string_view s, std::string_view sfx)
{
    return endswith(s, sfx.data(), sfx.size());
}

inline bool
contains(std::string_view s, char substring)
{
    return s.find(substring) != s.npos;
}

inline bool
contains(std::string_view s, const char* substring, size_t len)
{
    return s.find(substring, 0, len) != s.npos;
}

inline bool
contains(std::string_view s, const char* substring)
{
    return s.find(substring) != s.npos;
}

inline bool
contains(std::string_view s, std::string_view substring)
{
    return s.find(substring) != s.npos;
}

inline std::string::size_type
common_prefix_length(std::string_view a, std::string_view b)
{
    std::string::size_type minlen = std::min(a.size(), b.size());
    std::string::size_type common;
    for (common = 0; common < minlen; ++common) {
        if (a[common] != b[common]) break;
    }
    return common;
}

inline std::string::size_type
common_prefix_length(std::string_view a, std::string_view b,
                     std::string::size_type max_prefix_len)
{
    std::string::size_type minlen = std::min({a.size(),
                                              b.size(),
                                              max_prefix_len});
    std::string::size_type common;
    for (common = 0; common < minlen; ++common) {
        if (a[common] != b[common]) break;
    }
    return common;
}

#endif // XAPIAN_INCLUDED_STRINGUTILS_H
