/* quartz_utils.h: Generic functions for quartz
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#ifndef OM_HGUARD_QUARTZ_UTILS_H
#define OM_HGUARD_QUARTZ_UTILS_H

#include "config.h"
#include <string>

/// Compile time assert a condition
#define CASSERT(a) {char assert[(a) ? 1 : -1];(void)assert;}

typedef unsigned char       om_byte;
typedef unsigned int        om_uint32;
typedef unsigned long long  om_uint64;
typedef int                 om_int32;
typedef long long           om_int64;

/** Reads an integer from a string starting at a given position.
 *
 *  @param src       A pointer to a pointer to the data to read.  The
 *                   character pointer will be updated to point to the
 *                   next character to read, or 0 if no more data is to be
 *                   read.
 *  @param src_end   A pointer to the byte after the end of the data to
 *                   read the integer from.
 *  @param result    A pointer to a place to store the result.  If an
 *                   error occurs, the value stored in this location is
 *                   undefined.
 *
 *  @result True if an integer was successfully read.  False if the read
 *          failed.  Failure may either be due to the data running out (in
 *          which case *src will equal 0), or due to the value read
 *          overflowing the size of result (in which case *src will point
 *          to wherever the value ends, despite the overflow).
 */

template<class T>
bool
unpack_uint(const char ** src,
	    const char * src_end,
	    T * result)
{
    // Check unsigned
    CASSERT((T)(-1) > 0);

    // Check byte is what it's meant to be
    CASSERT(sizeof(om_byte) == 1);

    unsigned int shift = 0;
    *result = 0;

    while(1) {
	if ((*src) == src_end) {
	    *src = 0;
	    return false;
	}

	om_byte part = static_cast<om_byte> (**src);
	(*src)++;

	// if new byte might cause overflow, and it does
	if (((shift > (sizeof(*result) - 1) * 8 + 1) &&
	     ((part & 0x7f) << (shift % 8)) >= 0x100) ||
	    (shift >= sizeof(*result) * 8))  {
	    // Overflowed - move to end of this integer
	    while(1) {
		if ((part & 0x80) == 0) return false;
		if ((*src) == src_end) {
		    *src = 0;
		    return false;
		}
		part = static_cast<const om_byte> (**src);
		(*src)++;
	    }
	}

	*result += (part & 0x7f) << shift;
	shift += 7;

	if ((part & 0x80) == 0) {
	    return true;;
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
std::string
pack_uint(T value)
{
    // Check unsigned
    CASSERT((T)(-1) > 0);

    if (value == 0) return std::string("\000", 1);
    std::string result;

    while(value != 0) {
	om_byte part = value & 0x7f;
	value = value >> 7;
	if (value) part |= 0x80;
	result.append(1, (char) part);
    }

    return result;
}

inline bool
unpack_string(const char ** src,
	      const char * src_end,
	      std::string & result)
{
    std::string::size_type length;
    if (!unpack_uint(src, src_end, &length)) {
	return false;
    }

    if (src_end - *src < 0 ||
	(std::string::size_type)(src_end - *src) < length) {
	src = 0;
	return false;
    }

    result = string(*src, length);
    *src += length;
    return true;
}

inline std::string
pack_string(std::string value)
{
    return pack_uint(value.size()) + value;
}

inline bool
unpack_bool(const char ** src,
	    const char * src_end,
	    bool * result)
{
    if (*src == src_end) {
	*src = 0;
	return false;
    }
    switch (*((*src)++)) {
	case '0': *result = false; return true;
	case '1': *result = true; return true;
    }
    *src = 0;
    return false;
}

inline std::string
pack_bool(bool value)
{
    return value ? "1" : "0";
}

#include "quartz_table_entries.h"
#include "om/omtypes.h"

/** Convert a document id to an OmKey.
 */
QuartzDbKey quartz_docid_to_key(om_docid did);

#endif /* OM_HGUARD_QUARTZ_UTILS_H */
