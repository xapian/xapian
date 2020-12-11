/** @file
 * @brief constants and functions for document value handling.
 */
/* Copyright (C) 2006,2010,2015,2019 Olly Betts
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

#ifndef OMEGA_INCLUDED_VALUES_H
#define OMEGA_INCLUDED_VALUES_H

#ifndef PACKAGE
# error config.h must be included first in each C++ source file
#endif

#include <cstring>
#include <string>

#include <cstdint>

enum value_slot {
    VALUE_LASTMOD = 0,	// 4 byte big endian value - seconds since 1970.
    VALUE_MD5 = 1,	// 16 byte MD5 checksum of original document.
    VALUE_SIZE = 2,	// sortable_serialise(<file size in bytes>).
    VALUE_CTIME = 3	// Like VALUE_LASTMOD, but for last metadata change.
};

#ifndef WORDS_BIGENDIAN
inline std::uint32_t bswap32(std::uint32_t v) {
# if HAVE_DECL___BUILTIN_BSWAP32
    return __builtin_bswap32(v);
# elif HAVE_DECL__BYTESWAP_ULONG
    return _byteswap_ulong(v);
# else
    return (v << 24) | ((v & 0xff00) << 8) | ((v >> 8) & 0xff00) | (v >> 24);
# endif
}
#endif

inline std::uint32_t binary_string_to_int(const std::string &s)
{
    if (s.size() != 4) return static_cast<std::uint32_t>(-1);
    std::uint32_t v;
    std::memcpy(&v, s.data(), 4);
#ifndef WORDS_BIGENDIAN
    v = bswap32(v);
#endif
    return v;
}

inline std::string int_to_binary_string(std::uint32_t v)
{
#ifndef WORDS_BIGENDIAN
    v = bswap32(v);
#endif
    return std::string(reinterpret_cast<const char*>(&v), 4);
}

#endif
