/** @file proc_uuid.cc
 * @brief Generate UUIDs by reading from a pseudo-file under /proc
 *
 * Especially useful when building for Android, as it avoids having to
 * cross-build a UUID library.
 */
/* Copyright (C) 2008 Lemur Consulting Ltd
 * Copyright (C) 2013,2015,2016 Olly Betts
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

#include "proc_uuid.h"

#include "xapian/error.h"

#include <cstring>
#include "stringutils.h"

#include <sys/types.h>
#include "safesysstat.h"
#include "safeerrno.h"
#include "safefcntl.h"
#include "safeunistd.h"

using namespace std;

/// The size of a UUID in bytes.
const size_t UUID_SIZE = 16;

/// The size of a UUID string in bytes (not including trailing '\0').
const size_t UUID_STRING_SIZE = 36;

void
uuid_generate(uuid_t uu)
{
    char buf[37];
    int fd = open("/proc/sys/kernel/random/uuid", O_RDONLY);
    if (rare(fd == -1)) {
	throw Xapian::DatabaseCreateError("Opening UUID generator failed", errno);
    }
    if (read(fd, buf, 36) != 36) {
	close(fd);
	throw Xapian::DatabaseCreateError("Generating UUID failed");
    }
    close(fd);
    buf[36] = '\0';
    uuid_parse(buf, uu);
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
	sprintf(out, "%02x", uu[i]);
	out += 2;
	if ((0x2a8 >> i) & 1)
	   *out++ = '-';
    }
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
