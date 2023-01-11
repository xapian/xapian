/** @file
 * @brief Pack types into strings and unpack them again.
 */
/* Copyright (C) 2009,2015,2016,2017 Olly Betts
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

#ifndef PACKAGE
# error config.h must be included first in each C++ source file
#endif

#include <cstring>
#include <string>
#include <type_traits>

#include "omassert.h"

#include "xapian/types.h"

/** How many bits to store the length of a sortable uint in.
 *
 *  Setting this to 2 limits us to 2**32 documents in the database.  If set
 *  to 3, then 2**64 documents are possible, but the database format isn't
 *  compatible.
 */
const unsigned int SORTABLE_UINT_LOG2_MAX_BYTES = 2;

/// Calculated value used below.
const unsigned int SORTABLE_UINT_MAX_BYTES = 1 << SORTABLE_UINT_LOG2_MAX_BYTES;

/// Calculated value used below.
const unsigned int SORTABLE_UINT_1ST_BYTE_MASK =
	(0xffu >> SORTABLE_UINT_LOG2_MAX_BYTES);

/** Append an encoded bool to a string.
 *
 *  @param s		The string to append to.
 *  @param value	The bool to encode.
 */
inline void
pack_bool(std::string& s, bool value)
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
unpack_bool(const char** p, const char* end, bool* result)
{
    Assert(result);
    const char*& ptr = *p;
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
pack_uint_last(std::string& s, U value)
{
    static_assert(std::is_unsigned<U>::value, "Unsigned type required");

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
unpack_uint_last(const char** p, const char* end, U* result)
{
    static_assert(std::is_unsigned<U>::value, "Unsigned type required");
    Assert(result);

    const char* ptr = *p;
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
 *  [Chert variant]
 *
 *  The appended string data will sort in the same order as the unsigned
 *  integer being encoded.
 *
 *  Note that the first byte of the encoding will never be \xff, so it is
 *  safe to store the result of this function immediately after the result of
 *  pack_string_preserving_sort().
 *
 *  @param s		The string to append to.
 *  @param value	The unsigned integer to encode.
 */
template<class U>
inline void
C_pack_uint_preserving_sort(std::string & s, U value)
{
    static_assert(std::is_unsigned<U>::value, "Unsigned type required");
#if 0
    // FIXME: Doesn't work with 64-bit Xapian::docid, etc.
    static_assert(sizeof(U) <= SORTABLE_UINT_MAX_BYTES,
		  "Template type U too wide for database format");
#endif

    char tmp[sizeof(U) + 1];
    char * p = tmp + sizeof(tmp);

    do {
	*--p = char(value & 0xff);
	value >>= 8;
    } while (value &~ SORTABLE_UINT_1ST_BYTE_MASK);

    unsigned char len = static_cast<unsigned char>(tmp + sizeof(tmp) - p);
    *--p = char((len - 1) << (8 - SORTABLE_UINT_LOG2_MAX_BYTES) | value);
    s.append(p, len + 1);
    Assert(s[0] != '\xff');
}

/** Decode an "sort preserved" unsigned integer from a string.
 *
 *  [Chert variant]
 *
 *  The unsigned integer must have been encoded with
 *  C_pack_uint_preserving_sort().
 *
 *  @param p	    Pointer to pointer to the current position in the string.
 *  @param end	    Pointer to the end of the string.
 *  @param result   Where to store the result.
 */
template<class U>
inline bool
C_unpack_uint_preserving_sort(const char ** p, const char * end, U * result)
{
    static_assert(std::is_unsigned<U>::value, "Unsigned type required");
    static_assert(sizeof(U) < 256,
		  "Template type U too wide for database format");
    Assert(result);

    const char * ptr = *p;
    Assert(ptr);

    if (rare(ptr == end)) {
	return false;
    }

    unsigned char len_byte = static_cast<unsigned char>(*ptr++);
    *result = len_byte & SORTABLE_UINT_1ST_BYTE_MASK;
    size_t len = (len_byte >> (8 - SORTABLE_UINT_LOG2_MAX_BYTES)) + 1;

    if (rare(size_t(end - ptr) < len)) {
	return false;
    }

    end = ptr + len;
    *p = end;

    // Check for overflow.
    if (rare(len > int(sizeof(U)))) {
	return false;
    }

    while (ptr != end) {
	*result = (*result << 8) | U(static_cast<unsigned char>(*ptr++));
    }

    return true;
}

#if HAVE_DECL___BUILTIN_CLZ && \
    HAVE_DECL___BUILTIN_CLZL && \
    HAVE_DECL___BUILTIN_CLZLL
template<typename T>
inline int
do_clz(T value) {
    extern int no_clz_builtin_for_this_type(T);
    return no_clz_builtin_for_this_type(value);
}

template<>
inline int
do_clz(unsigned value) {
    return __builtin_clz(value);
}

template<>
inline int
do_clz(unsigned long value) {
    return __builtin_clzl(value);
}

template<>
inline int
do_clz(unsigned long long value) {
    return __builtin_clzll(value);
}

# define HAVE_DO_CLZ
#endif

/** Append an encoded unsigned integer to a string, preserving the sort order.
 *
 *  [Glass and newer variant]
 *
 *  The appended string data will sort in the same order as the unsigned
 *  integer being encoded.
 *
 *  Note that the first byte of the encoding will never be \xff, so it is
 *  safe to store the result of this function immediately after the result of
 *  pack_string_preserving_sort().
 *
 *  @param s		The string to append to.
 *  @param value	The unsigned integer to encode.
 */
template<class U>
inline void
pack_uint_preserving_sort(std::string& s, U value)
{
    static_assert(std::is_unsigned<U>::value, "Unsigned type required");
    static_assert(sizeof(U) <= 8,
		  "Template type U too wide for database format");
    // The clz() functions are undefined for 0, so handle the smallest band
    // as a special case.
    if (value < 0x8000) {
	s.resize(s.size() + 2);
	s[s.size() - 2] = static_cast<unsigned char>(value >> 8);
	Assert(s[s.size() - 2] != '\xff');
	s[s.size() - 1] = static_cast<unsigned char>(value);
	return;
    }

#ifdef HAVE_DO_CLZ
    size_t len = ((sizeof(U) * 8 + 5) - do_clz(value)) / 7;
#else
    size_t len = 3;
    for (auto x = value >> 22; x; x >>= 7) ++len;
#endif
    unsigned mask = 0xff << (10 - len);

    s.resize(s.size() + len);
    for (size_t i = 1; i != len; ++i) {
	s[s.size() - i] = static_cast<unsigned char>(value);
	value >>= 8;
    }

    s[s.size() - len] = static_cast<unsigned char>(value | mask);
    Assert(s[s.size() - len] != '\xff');

    AssertRel(len, >, 2);
    AssertRel(len, <=, 9);
}

/** Decode a "sort preserved" unsigned integer from a string.
 *
 *  [Glass and newer variant]
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
unpack_uint_preserving_sort(const char** p, const char* end, U* result)
{
    static_assert(std::is_unsigned<U>::value, "Unsigned type required");
    static_assert(sizeof(U) <= 8,
		  "Template type U too wide for database format");
    Assert(result);

    const char* ptr = *p;
    Assert(ptr);

    if (rare(ptr == end)) {
	return false;
    }

    unsigned char len_byte = static_cast<unsigned char>(*ptr++);
    if (len_byte < 0x80) {
	*result = (U(len_byte) << 8) | static_cast<unsigned char>(*ptr++);
	*p = ptr;
	return true;
    }

    if (len_byte == 0xff) {
	return false;
    }

    // len is how many bytes there are after the length byte.
#ifdef HAVE_DO_CLZ
    size_t len = do_clz(len_byte ^ 0xffu) + 9 - sizeof(unsigned) * 8;
#else
    size_t len = 2;
    for (unsigned char m = 0x40; len_byte & m; m >>= 1) ++len;
#endif
    if (rare(size_t(end - ptr) < len)) {
	return false;
    }
    unsigned mask = 0xff << (9 - len);
    len_byte &= ~mask;

    // Check for overflow.
    if (rare(len > int(sizeof(U)))) return false;
    if (sizeof(U) != 8) {
	// Need to check the top byte too.
	if (rare(len == int(sizeof(U)) && len_byte != 0)) return false;
    }

    end = ptr + len;
    *p = end;

    U r = len_byte;
    while (ptr != end) {
	r = (r << 8) | U(static_cast<unsigned char>(*ptr++));
    }
    *result = r;

    return true;
}

/** Append an encoded unsigned integer to a string.
 *
 *  @param s		The string to append to.
 *  @param value	The unsigned integer to encode.
 */
template<class U>
inline void
pack_uint(std::string& s, U value)
{
    static_assert(std::is_unsigned<U>::value, "Unsigned type required");

    while (value >= 128) {
	s += static_cast<char>(static_cast<unsigned char>(value) | 0x80);
	value >>= 7;
    }
    s += static_cast<char>(value);
}

/** Append an encoded unsigned integer (bool type) to a string.
 *
 *  @param s		The string to append to.
 *  @param value	The unsigned integer to encode.
 */
template<>
inline void
pack_uint(std::string& s, bool value)
{
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
unpack_uint(const char** p, const char* end, U* result)
{
    static_assert(std::is_unsigned<U>::value, "Unsigned type required");

    const char* ptr = *p;
    Assert(ptr);
    const char* start = ptr;

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
pack_string(std::string& s, const std::string& value)
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
pack_string(std::string& s, const char* ptr)
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
unpack_string(const char** p, const char* end, std::string& result)
{
    size_t len;
    if (rare(!unpack_uint(p, end, &len))) {
	return false;
    }

    const char*& ptr = *p;
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
 *  The byte which follows this encoded value *must not* be \xff, or the sort
 *  order won't be correct.  You may need to store a padding byte (\0 say) to
 *  ensure this.  Note that pack_uint_preserving_sort() can never produce
 *  \xff as its first byte so is safe to use immediately afterwards.
 *
 *  @param s		The string to append to.
 *  @param value	The std::string to encode.
 *  @param last		If true, this is the last thing to be encoded in this
 *			string - see note below (default: false)
 *
 *  It doesn't make sense to use pack_string_preserving_sort() if nothing can
 *  ever follow, but if optional items can, you can set last=true in cases
 *  where nothing does and get a shorter encoding in those cases.
 */
inline void
pack_string_preserving_sort(std::string& s, const std::string& value,
			    bool last = false)
{
    std::string::size_type b = 0, e;
    while ((e = value.find('\0', b)) != std::string::npos) {
	++e;
	s.append(value, b, e - b);
	s += '\xff';
	b = e;
    }
    s.append(value, b, std::string::npos);
    if (!last) s += '\0';
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
unpack_string_preserving_sort(const char** p, const char* end,
			      std::string& result)
{
    result.resize(0);

    const char* ptr = *p;
    Assert(ptr);

    while (ptr != end) {
	char ch = *ptr++;
	if (rare(ch == '\0')) {
	    if (usual(ptr == end || *ptr != '\xff')) {
		break;
	    }
	    ++ptr;
	}
	result += ch;
    }
    *p = ptr;
    return true;
}

inline std::string
pack_chert_postlist_key(const std::string& term)
{
    // Special case for doclen lists.
    if (term.empty())
	return std::string("\x00\xe0", 2);

    std::string key;
    pack_string_preserving_sort(key, term, true);
    return key;
}

inline std::string
pack_chert_postlist_key(const std::string& term, Xapian::docid did)
{
    // Special case for doclen lists.
    if (term.empty()) {
	std::string key("\x00\xe0", 2);
	C_pack_uint_preserving_sort(key, did);
	return key;
    }

    std::string key;
    pack_string_preserving_sort(key, term);
    C_pack_uint_preserving_sort(key, did);
    return key;
}

inline std::string
pack_glass_postlist_key(const std::string& term)
{
    // Special case for doclen lists.
    if (term.empty())
	return std::string("\x00\xe0", 2);

    std::string key;
    pack_string_preserving_sort(key, term, true);
    return key;
}

inline std::string
pack_glass_postlist_key(const std::string& term, Xapian::docid did)
{
    // Special case for doclen lists.
    if (term.empty()) {
	std::string key("\x00\xe0", 2);
	pack_uint_preserving_sort(key, did);
	return key;
    }

    std::string key;
    pack_string_preserving_sort(key, term);
    pack_uint_preserving_sort(key, did);
    return key;
}

#endif // XAPIAN_INCLUDED_PACK_H
