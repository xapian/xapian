/* flint_utils.h: Generic functions for flint
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2006,2008,2009 Olly Betts
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

#ifndef OM_HGUARD_FLINT_UTILS_H
#define OM_HGUARD_FLINT_UTILS_H

#include "omassert.h"

#include <xapian/types.h>

#include <string>

using namespace std;

typedef unsigned char       om_byte;
typedef unsigned int        om_uint32;
typedef int                 om_int32;

/** FIXME: the pack and unpack int methods store in low-byte-first order
 *  - it might be easier to implement efficient specialisations with
 *  high-byte-first order.
 */

/** Reads an unsigned integer from a string starting at a given position.
 *
 *  @param src       A pointer to a pointer to the data to read.  The
 *                   character pointer will be updated to point to the
 *                   next character to read, or 0 if the method ran out of
 *                   data.  (It is only set to 0 in case of an error).
 *  @param src_end   A pointer to the byte after the end of the data to
 *                   read the integer from.
 *  @param resultptr A pointer to a place to store the result.  If an
 *                   error occurs, the value stored in this location is
 *                   undefined.  If this pointer is 0, the result is not
 *                   stored, and the method simply skips over the result.
 *
 *  @result True if an integer was successfully read.  False if the read
 *          failed.  Failure may either be due to the data running out (in
 *          which case *src will equal 0), or due to the value read
 *          overflowing the size of result (in which case *src will point
 *          to wherever the value ends, despite the overflow).
 */
template<class T>
bool
F_unpack_uint(const char ** src,
	    const char * src_end,
	    T * resultptr)
{
    // Check unsigned
    STATIC_ASSERT_UNSIGNED_TYPE(T);

    // Check byte is what it's meant to be
    STATIC_ASSERT(sizeof(om_byte) == 1);

    unsigned int shift = 0;
    T result = 0;

    while (true) {
	if ((*src) == src_end) {
	    *src = 0;
	    return false;
	}

	om_byte part = static_cast<om_byte>(**src);
	(*src)++;

	// if new byte might cause overflow, and it does
	if (((shift > (sizeof(T) - 1) * 8 + 1) &&
	     ((part & 0x7f) << (shift % 8)) >= 0x100) ||
	    (shift >= sizeof(T) * 8)) {
	    // Overflowed - move to end of this integer
	    while (true) {
		if ((part & 0x80) == 0) return false;
		if ((*src) == src_end) {
		    *src = 0;
		    return false;
		}
		part = static_cast<om_byte>(**src);
		(*src)++;
	    }
	}

	result += T(part & 0x7f) << shift;
	shift += 7;

	if ((part & 0x80) == 0) {
	    if (resultptr) *resultptr = result;
	    return true;
	}
    }
}


/** Generates a packed representation of an integer.
 *
 *  @param value  The integer to represent.
 *
 *  @result       A string containing the representation of the integer.
 */
template<class T>
string
F_pack_uint(T value)
{
    // Check unsigned
    STATIC_ASSERT_UNSIGNED_TYPE(T);

    if (value == 0) return string(1, '\0');
    string result;

    while (value != 0) {
	om_byte part = static_cast<om_byte>(value & 0x7f);
	value = value >> 7;
	if (value) part |= 0x80;
	result.append(1u, char(part));
    }

    return result;
}

/** Generates a packed representation of a bool.
 *
 *  This is a specialisation of the template above.
 *
 *  @param value  The bool to represent.
 *
 *  @result       A string containing the representation of the bool.
 */
template<>
inline string
F_pack_uint<bool>(bool value)
{
    return string(1, static_cast<char>(value));
}

/** Reads an unsigned integer from a string starting at a given position.
 *  This encoding requires that we know the encoded length from out-of-band
 *  information (so is suitable when only one integer is encoded, or for
 *  the last integer encoded).
 *
 *  @param src       A pointer to a pointer to the data to read.
 *  @param src_end   A pointer to the byte after the end of the data to
 *                   read the integer from.
 *  @param resultptr A pointer to a place to store the result.  If an
 *                   error occurs, the value stored in this location is
 *                   undefined.  If this pointer is 0, the result is not
 *                   stored, and the method simply skips over the result.
 *
 *  @result True if an integer was successfully read.  False if the read
 *          failed.  Failure can hapen if the value read overflows
 *          the size of result.
 */
template<class T>
bool
F_unpack_uint_last(const char ** src, const char * src_end, T * resultptr)
{
    // Check unsigned
    STATIC_ASSERT_UNSIGNED_TYPE(T);
    // Check byte is what it's meant to be
    STATIC_ASSERT(sizeof(om_byte) == 1);

    if (src_end - *src > int(sizeof(T))) {
	// Would overflow
	*src = src_end;
	return false;
    }

    T result = 0;
    int shift = 0;
    while (*src != src_end) {
	result |= static_cast<T>(static_cast<om_byte>(**src)) << shift;
	++(*src);
	shift += 8;
    }
    *resultptr = result;
    return true;
}

/** Generates a packed representation of an integer.
 *  This encoding requires that we know the encoded length from out-of-band
 *  information (so is suitable when only one integer is encoded, or for
 *  the last integer encoded).
 *
 *  @param value  The integer to represent.
 *
 *  @result       A string containing the representation of the integer.
 */
template<class T>
string
F_pack_uint_last(T value)
{
    // Check unsigned
    STATIC_ASSERT_UNSIGNED_TYPE(T);

    string result;
    while (value) {
        result += char(value);
	value >>= 8;
    }
    return result;
}

/** Generate a packed representation of an integer, preserving sort order.
 *
 *  This representation is less compact than the usual one, and has a limit
 *  of 256 bytes on the length of the integer.  However, this is unlikely to
 *  ever be a problem.
 *
 *  @param value  The integer to represent.
 *
 *  @result       A string containing the representation of the integer.
 */
template<class T>
string
F_pack_uint_preserving_sort(T value)
{
    // Check unsigned
    STATIC_ASSERT_UNSIGNED_TYPE(T);

    string result;
    while (value != 0) {
	om_byte part = static_cast<om_byte>(value & 0xff);
	value = value >> 8;
	result.insert(string::size_type(0), 1u, char(part));
    }
    result.insert(string::size_type(0), 1u, char(result.size()));
    return result;
}

/** Unpack a unsigned integer, store in sort preserving order.
 *
 *  @param src       A pointer to a pointer to the data to read.  The
 *                   character pointer will be updated to point to the
 *                   next character to read, or 0 if the method ran out of
 *                   data.  (It is only set to 0 in case of an error).
 *  @param src_end   A pointer to the byte after the end of the data to
 *                   read the integer from.
 *  @param resultptr A pointer to a place to store the result.  If an
 *                   error occurs, the value stored in this location is
 *                   undefined.  If this pointer is 0, the result is not
 *                   stored, and the method simply skips over the result.
 *
 *  @result True if an integer was successfully read.  False if the read
 *          failed.  Failure may either be due to the data running out (in
 *          which case *src will equal 0), or due to the value read
 *          overflowing the size of result (in which case *src will point
 *          to wherever the value ends, despite the overflow).
 */
template<class T>
bool
F_unpack_uint_preserving_sort(const char ** src,
			    const char * src_end,
			    T * resultptr)
{
    if (*src == src_end) {
	*src = 0;
	return false;
    }

    unsigned int length = static_cast<om_byte>(**src);
    (*src)++;

    if (length > sizeof(T)) {
	*src += length;
	if (*src > src_end) {
	    *src = 0;
	}
	return false;
    }

    // Can't be overflow now.
    T result = 0;
    while (length > 0) {
	result = result << 8;
	result += static_cast<om_byte>(**src);
	(*src)++;
	length--;
    }
    *resultptr = result;

    return true;
}

inline bool
F_unpack_string(const char ** src,
	      const char * src_end,
	      string & result)
{
    string::size_type length;
    if (!F_unpack_uint(src, src_end, &length)) {
    	return false;
    }

    if (src_end - *src < 0 ||
	string::size_type(src_end - *src) < length) {
	src = 0;
	return false;
    }

    result.assign(*src, length);
    *src += length;
    return true;
}

inline string
F_pack_string(const string & value)
{
    return F_pack_uint(value.size()) + value;
}

/** Pack a string into a representation which preserves sort order.
 *  We do this by replacing zero bytes in the string with a zero byte
 *  followed by byte value 0xff, and then appending two zero bytes to
 *  the end.
 */
inline string
F_pack_string_preserving_sort(string value)
{
    string::size_type i = 0, j;
    while ((j = value.find('\0', i)) != string::npos) {
	value.replace(j, 1, "\0\xff", 2);
	i = j + 2;
    }
    value += '\0'; // FIXME temp...
    return value + '\0'; // Note - next byte mustn't be '\xff'...
}

inline bool
F_unpack_string_preserving_sort(const char ** src,
			      const char * src_end,
			      string & result)
{
    result.resize(0);
    while (*src < src_end) {
	const char *begin = *src;
	while (**src) {
	    ++(*src);
	    if (*src == src_end) return false;
	}
	result += string(begin, *src - begin);
	++(*src);
	if (*src == src_end) return false;
	if (**src != '\xff') {
	    ++(*src); // FIXME temp
	    return true;
	}
	result += '\0';
	++(*src);
    }
    return false;
}

inline bool
F_unpack_bool(const char ** src,
	    const char * src_end,
	    bool * resultptr)
{
    if (*src == src_end) {
	*src = 0;
	return false;
    }
    switch (*((*src)++)) {
	case '0':
	    if (resultptr) *resultptr = false;
	    return true;
	case '1':
	    if (resultptr) *resultptr = true;
	    return true;
    }
    *src = 0;
    return false;
}

inline string
F_pack_bool(bool value)
{
    return value ? "1" : "0";
}

/** Convert a document id to a key (suitable when the docid is the only
 *  component of the key).
 */
inline string
flint_docid_to_key(Xapian::docid did)
{
    return F_pack_uint_preserving_sort(did);
}

#endif /* OM_HGUARD_FLINT_UTILS_H */
