/** @file stringutils.h
 * @brief Various handy helpers which std::string really should provide.
 */
/* Copyright (C) 2007 Olly Betts
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

#ifndef XAPIAN_INCLUDED_STRINGUTILS_H
#define XAPIAN_INCLUDED_STRINGUTILS_H

#include <algorithm>
#include <string>
#include <string.h>

inline bool
startswith(const std::string & s, char pfx)
{
    return !s.empty() && s[0] == pfx;
}

inline bool
startswith(const std::string & s, const char * pfx, size_t len)
{
    return s.size() >= len && (memcmp(s.data(), pfx, len) == 0);
}

inline bool
startswith(const std::string & s, const char * pfx)
{
    return startswith(s, pfx, strlen(pfx));
}

inline bool
startswith(const std::string & s, const std::string & pfx)
{
    return startswith(s, pfx.data(), pfx.size());
}

inline bool
endswith(const std::string & s, char sfx)
{
    return !s.empty() && s[s.size() - 1] == sfx;
}

inline bool
endswith(const std::string & s, const char * sfx, size_t len)
{
    return s.size() >= len && (memcmp(s.data() + s.size() - len, sfx, len) == 0);
}

inline bool
endswith(const std::string & s, const char * sfx)
{
    return endswith(s, sfx, strlen(sfx));
}

inline bool
endswith(const std::string & s, const std::string & sfx)
{
    return endswith(s, sfx.data(), sfx.size());
}

inline std::string::size_type
common_prefix_length(const std::string &a, const std::string &b)
{
    std::string::size_type minlen = std::min(a.size(), b.size());
    std::string::size_type common;
    for (common = 0; common < minlen; ++common) {
	if (a[common] != b[common]) break;
    }
    return common;
}

#endif // XAPIAN_INCLUDED_STRINGUTILS_H
