/** @file pack.h
 * @brief Pack types into strings and unpack them again.
 */
/* Copyright (C) 2009 Olly Betts
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

#ifndef XAPIAN_INCLUDED_PACK_H
#define XAPIAN_INCLUDED_PACK_H

#include <cstring>
#include <string>

#include "omassert.h"

/** Append an encoded bool to a string.
 *
 *  @param s		The string to append to.
 *  @param value	The bool to encode.
 */
inline void
pack_bool(std::string & s, bool value)
{
    s += char('0' | static_cast<char>(value));
}

/** Decode a bool from a string.
 *
 *  @param p	    Pointer to pointer to the current position in the string.
 *  @param end	    Pointer to the end of the string.
 *  @param result   Where to store the result.
 */
inline bool
unpack_bool(const char ** p, const char * end, bool * result)
{
    Assert(result);
    const char * & ptr = *p;
    Assert(ptr);
    char ch;
    if (rare(ptr == end || ((ch = *ptr++ - '0') &~ 1))) {
	ptr = NULL;
	return false;
    }
    *result = static_cast<bool>(ch);
    return true;
}

/** Append an encoded unsigned integer to a string as the last item.
 *
 *  This encoding is only suitable when this is the last thing encoded as
 *  the encoding used doesn't contain its own length.
 *
 *  @param s		The string to append to.
 *  @param value	The unsigned integer to encode.
 */
template<class U>
inline void
pack_uint_last(std::string & s, U value)
{
    // Check U is an unsigned type.
    STATIC_ASSERT_UNSIGNED_TYPE(U);

    while (value) {
	s += char(value & 0xff);
	value >>= 8;
    }
}

/** Decode an unsigned integer as the last item in a string.
 *
 *  @param p	    Pointer to pointer to the current position in the string.
 *  @param end	    Pointer to the end of the string.
 *  @param result   Where to store the result.
 */
template<class U>
inline bool
unpack_uint_last(const char ** p, const char * end, U * result)
{
    // Check U is an unsigned type.
    STATIC_ASSERT_UNSIGNED_TYPE(U);
    Assert(result);

    const char * ptr = *p;
    Assert(ptr);
    *p = end;

    // Check for overflow.
    if (rare(end - ptr > int(sizeof(U)))) {
	return false;
    }

    *result = 0;
    while (end != ptr) {
	*result = (*result << 8) | U(static_cast<unsigned char>(*--end));
    }

    return true;
}

/** Append an encoded unsigned integer to a string, preserving the sort order.
 *
 *  The appended string data will sort in the same order as the unsigned
 *  integer being encoded.
 *
 *  @param s		The string to append to.
 *  @param value	The unsigned integer to encode.
 */
template<class U>
inline void
pack_uint_preserving_sort(std::string & s, U value)
{
    // Check U is an unsigned type.
    STATIC_ASSERT_UNSIGNED_TYPE(U);
    STATIC_ASSERT(sizeof(U) < 256);

    char tmp[sizeof(U) + 1];
    char * p = tmp + sizeof(tmp);

    while (value) {
	*--p = char(value & 0xff);
	value >>= 8;
    }

    char len = static_cast<char>(tmp + sizeof(tmp) - p);
    *--p = len;
    s.append(p, len + 1);
}

/** Decode an "sort preserved" unsigned integer from a string.
 *
 *  The unsigned integer must have been encoded with
 *  pack_uint_preserving_sort().
 *
 *  @param p	    Pointer to pointer to the current position in the string.
 *  @param end	    Pointer to the end of the string.
 *  @param result   Where to store the result.
 */
template<class U>
inline bool
unpack_uint_preserving_sort(const char ** p, const char * end, U * result)
{
    // Check U is an unsigned type.
    STATIC_ASSERT_UNSIGNED_TYPE(U);
    STATIC_ASSERT(sizeof(U) < 256);
    Assert(result);

    const char * ptr = *p;
    Assert(ptr);

    if (rare(ptr == end)) {
	return false;
    }

    size_t len = size_t(static_cast<unsigned char>(*ptr++));

    if (rare(size_t(end - ptr) < len)) {
	return false;
    }

    end = ptr + len;
    *p = end;

    // Check for overflow.
    if (rare(len > int(sizeof(U)))) {
	return false;
    }

    *result = 0;
    while (ptr != end) {
	*result = (*result << 8) | U(static_cast<unsigned char>(*ptr++));
    }

    return true;
}

/** Append an encoded unsigned integer to a string.
 *
 *  @param s		The string to append to.
 *  @param value	The unsigned integer to encode.
 */
template<class U>
inline void
pack_uint(std::string & s, U value)
{
    // Check U is an unsigned type.
    STATIC_ASSERT_UNSIGNED_TYPE(U);

    while (value >= 128) {
	s += static_cast<char>(static_cast<unsigned char>(value) | 0x80);
	value >>= 7;
    }
    s += static_cast<char>(value);
}

/** Decode an unsigned integer from a string.
 *
 *  @param p	    Pointer to pointer to the current position in the string.
 *  @param end	    Pointer to the end of the string.
 *  @param result   Where to store the result (or NULL to just skip it).
 */
template<class U>
inline bool
unpack_uint(const char ** p, const char * end, U * result)
{
    // Check U is an unsigned type.
    STATIC_ASSERT_UNSIGNED_TYPE(U);

    const char * ptr = *p;
    Assert(ptr);
    const char * start = ptr;

    // Check the length of the encoded integer first.
    do {
	if (rare(ptr == end)) {
	    // Out of data.
	    *p = NULL;
	    return false;
	}
    } while (static_cast<unsigned char>(*ptr++) >= 128);

    *p = ptr;

    if (!result) return true;

    *result = U(*--ptr);
    if (ptr == start) {
	// Special case for small values.
	return true;
    }

    size_t maxbits = size_t(ptr - start) * 7;
    if (maxbits <= sizeof(U) * 8) {
	// No possibility of overflow.
	do {
	    unsigned char chunk = static_cast<unsigned char>(*--ptr) & 0x7f;
	    *result = (*result << 7) | U(chunk);
	} while (ptr != start);
	return true;
    }

    size_t minbits = maxbits - 6;
    if (rare(minbits > sizeof(U) * 8)) {
	// Overflow.
	return false;
    }

    while (--ptr != start) {
	unsigned char chunk = static_cast<unsigned char>(*--ptr) & 0x7f;
	*result = (*result << 7) | U(chunk);
    }

    U tmp = *result;
    *result <<= 7;
    if (rare(*result < tmp)) {
	// Overflow.
	return false;
    }
    *result |= U(static_cast<unsigned char>(*ptr) & 0x7f);
    return true;
}

/** Append an encoded std::string to a string.
 *
 *  @param s		The string to append to.
 *  @param value	The std::string to encode.
 */
inline void
pack_string(std::string & s, const std::string & value)
{
    pack_uint(s, value.size());
    s += value;
}

/** Append an encoded C-style string to a string.
 *
 *  @param s		The string to append to.
 *  @param ptr		The C-style string to encode.
 */
inline void
pack_string(std::string & s, const char * ptr)
{
    Assert(ptr);
    size_t len = std::strlen(ptr);
    pack_uint(s, len);
    s.append(ptr, len);
}

/** Decode a std::string from a string.
 *
 *  @param p	    Pointer to pointer to the current position in the string.
 *  @param end	    Pointer to the end of the string.
 *  @param result   Where to store the result.
 */
inline bool
unpack_string(const char ** p, const char * end, std::string & result)
{
    size_t len;
    if (rare(!unpack_uint(p, end, &len))) {
	return false;
    }

    const char * & ptr = *p;
    if (rare(len > size_t(end - ptr))) {
	ptr = NULL;
	return false;
    }

    result.assign(ptr, len);
    ptr += len;
    return true;
}

/** Append an encoded std::string to a string, preserving the sort order.
 *
 *  @param s		The string to append to.
 *  @param value	The std::string to encode.
 */
inline void
pack_string_preserving_sort(std::string & s, const std::string & value)
{
    std::string::size_type b = 0, e;
    while ((e = value.find('\0', b)) != std::string::npos) {
	++e;
	s.append(value, b, e - b);
	s += '\xff';
	b = e;
    }
    s.append(value, b, std::string::npos);
    s.append("\0", 2);
}

/** Decode a "sort preserved" std::string from a string.
 *
 *  The std::string must have been encoded with pack_string_preserving_sort().
 *
 *  @param p	    Pointer to pointer to the current position in the string.
 *  @param end	    Pointer to the end of the string.
 *  @param result   Where to store the result.
 */
inline bool
unpack_string_preserving_sort(const char ** p, const char * end,
			      std::string & result)
{
    result.resize(0);

    const char *ptr = *p;
    Assert(ptr);

    while (ptr != end) {
	char ch = *ptr++;
	if (rare(ch == '\0')) {
	    if (rare(ptr == end)) {
		// Ran out of data expecting '\0' or '\xff'.
		break;
	    }

	    char ch2 = *ptr++;
	    if (!ch2) {
		*p = ptr;
		return true;
	    }

	    if (rare(ch2 != '\xff')) {
		// Expected '\0' or '\xff'.
		break;
	    }
	}
	result += ch;
    }

    *p = NULL;
    return false;
}

#endif // XAPIAN_INCLUDED_PACK_H
