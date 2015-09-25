/** @file length.cc
 * @brief length encoded as a string
 */
/* Copyright (C) 2006,2007,2008,2009,2010,2011,2012,2015 Olly Betts
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

#include <config.h>

#include "length.h"

#include "noreturn.h"

XAPIAN_NORETURN(static void throw_network_error(const char * msg));

#ifndef XAPIAN_UNITTEST

#include "xapian/error.h"

static void
throw_network_error(const char * msg)
{
    throw Xapian::NetworkError(msg);
}

#else

class Xapian_NetworkError {
    const char * msg;

  public:
    explicit Xapian_NetworkError(const char * msg_) : msg(msg_) { }

    const char * get_description() const { return msg; }
};

static void
throw_network_error(const char * msg)
{
    throw Xapian_NetworkError(msg);
}

#endif

template<typename T>
inline void
decode_length_(const char ** p, const char *end, T & out)
{
    if (*p == end) {
	throw_network_error("Bad encoded length: no data");
    }

    T len = static_cast<unsigned char>(*(*p)++);
    if (len == 0xff) {
	len = 0;
	unsigned char ch;
	unsigned shift = 0;
	do {
	    if (*p == end || shift > (sizeof(T) * 8 / 7 * 7))
		throw_network_error("Bad encoded length: insufficient data");
	    ch = *(*p)++;
	    len |= T(ch & 0x7f) << shift;
	    shift += 7;
	} while ((ch & 0x80) == 0);
	len += 255;
    }
    out = len;
}

template<typename T>
inline void
decode_length_and_check_(const char ** p, const char *end, T & out)
{
    decode_length(p, end, out);
    if (out > T(end - *p)) {
	throw_network_error("Bad encoded length: length greater than data");
    }
}

void
decode_length(const char ** p, const char *end, unsigned & out)
{
    decode_length_(p, end, out);
}

void
decode_length(const char ** p, const char *end, unsigned long & out)
{
    decode_length_(p, end, out);
}

void
decode_length(const char ** p, const char *end, unsigned long long & out)
{
    decode_length_(p, end, out);
}

void
decode_length_and_check(const char ** p, const char *end, unsigned & out)
{
    decode_length_and_check_(p, end, out);
}

void
decode_length_and_check(const char ** p, const char *end, unsigned long & out)
{
    decode_length_and_check_(p, end, out);
}

void
decode_length_and_check(const char ** p, const char *end,
			unsigned long long & out)
{
    decode_length_and_check_(p, end, out);
}
