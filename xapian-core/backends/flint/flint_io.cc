/** @file flint_io.cc
 * @brief Wrappers for low-level POSIX I/O routines.
 */
/* Copyright (C) 2006,2007,2009 Olly Betts
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

#include "safeerrno.h"
#include "safeunistd.h"

#include <xapian/error.h>

#include "flint_io.h"

size_t flint_io_read(int fd, char * p, size_t n, size_t min)
{
    size_t total = 0;
    while (n) {
	ssize_t c = read(fd, p, n);
	if (c <= 0) {
	    if (c == 0) {
		if (total >= min) break;
		throw Xapian::DatabaseError("Couldn't read enough (EOF)");
	    }
	    if (errno == EINTR) continue;
	    throw Xapian::DatabaseError("Error reading from file", errno);
	}
	p += c;
	total += c;
	n -= c;
    }
    return total;
}

/** Write n bytes from block pointed to by p to file descriptor fd. */
void flint_io_write(int fd, const char * p, size_t n)
{
    while (n) {
	ssize_t c = write(fd, p, n);
	if (c < 0) {
	    if (errno == EINTR) continue;
	    throw Xapian::DatabaseError("Error writing to file", errno);
	}
	p += c;
	n -= c;
    }
}
