/** @file win32_uuid.cc
 * @brief Provide UUID functions compatible with libuuid from util-linux-ng.
 */
/* Copyright (C) 2008 Lemur Consulting Ltd
 * Copyright (C) 2013,2015,2016,2017 Olly Betts
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

#include "win32_uuid.h"

#include "xapian/error.h"

#include <cstring>
#include "stringutils.h"

#ifdef __WIN32__
# include "safewinsock2.h" // For htonl() and htons().
#else
// Cygwin:
# include <arpa/inet.h> // For htonl() and htons().
#endif

using namespace std;

/// The size of a UUID in bytes.
const size_t UUID_SIZE = 16;

void
uuid_generate(uuid_t uu)
{
    UUID uuid;
    if (rare(UuidCreate(&uuid) != RPC_S_OK)) {
	// Throw a DatabaseCreateError, since we can't make a UUID.  The
	// windows API documentation is a bit unclear about the situations in
	// which this can happen, but if this behaviour causes a problem, an
	// alternative would be to create a UUID ourselves somehow in this
	// situation.
	throw Xapian::DatabaseCreateError("Cannot create UUID");
    }
    uuid.Data1 = htonl(uuid.Data1);
    uuid.Data2 = htons(uuid.Data2);
    uuid.Data3 = htons(uuid.Data3);
    memcpy(uu, &uuid, UUID_SIZE);
}

int
uuid_parse(const char * in, uuid_t uu)
{
    for (unsigned i = 0; i != UUID_SIZE; ++i) {
	uu[i] = hex_digit(in[0]) << 4 | hex_digit(in[1]);
	in += ((0x2a8 >> i) & 1) | 2;
    }
    return 0;
}

void uuid_unparse_lower(const uuid_t uu, char * out)
{
    for (unsigned i = 0; i != UUID_SIZE; ++i) {
	unsigned char ch = uu[i];
	*out++ = "0123456789abcdef"[ch >> 4];
	*out++ = "0123456789abcdef"[ch & 0x0f];
	if ((0x2a8 >> i) & 1)
	   *out++ = '-';
    }
    *out = '\0';
}

void uuid_clear(uuid_t uu)
{
    memset(uu, 0, UUID_SIZE);
}

int uuid_is_null(const uuid_t uu)
{
    unsigned i = 0;
    while (i < UUID_SIZE) {
	if (uu[i++])
	    return 0;
    }
    return 1;
}
