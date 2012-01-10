/** @file length.cc
 * @brief length encoded as a string
 */
/* Copyright (C) 2006,2007,2008,2009,2010,2011,2012 Olly Betts
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
    Xapian_NetworkError(const char * msg_) : msg(msg_) { }

    const char * get_description() const { return msg; }
};

static void
throw_network_error(const char * msg)
{
    throw Xapian_NetworkError(msg);
}

#endif

size_t
decode_length(const char ** p, const char *end, bool check_remaining)
{
    if (*p == end) {
	throw_network_error("Bad encoded length: no data");
    }

    size_t len = static_cast<unsigned char>(*(*p)++);
    if (len == 0xff) {
	len = 0;
	unsigned char ch;
	int shift = 0;
	do {
	    if (*p == end || shift > 28)
		throw_network_error("Bad encoded length: insufficient data");
	    ch = *(*p)++;
	    len |= size_t(ch & 0x7f) << shift;
	    shift += 7;
	} while ((ch & 0x80) == 0);
	len += 255;
    }
    if (check_remaining && len > size_t(end - *p)) {
	throw_network_error("Bad encoded length: length greater than data");
    }
    return len;
}
