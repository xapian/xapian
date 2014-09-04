/** @file io_utils.cc
 * @brief Wrappers for low-level POSIX I/O routines.
 */
/* Copyright (C) 2004,2006,2007,2008,2009,2011,2014 Olly Betts
 * Copyright (C) 2010 Richard Boulton
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

#include "io_utils.h"
#include "posixy_wrapper.h"

#include "safeerrno.h"
#include "safeunistd.h"

#include <string>

#include <xapian/error.h>

#include "noreturn.h"
#include "str.h"

// Trying to include the correct headers with the correct defines set to
// get pread() and pwrite() prototyped on every platform without breaking any
// other platform is a real can of worms.  So instead we probe for what
// prototypes (if any) are required in configure and put them into
// PREAD_PROTOTYPE and PWRITE_PROTOTYPE.
#if defined HAVE_PREAD && defined PREAD_PROTOTYPE
PREAD_PROTOTYPE
#endif
#if defined HAVE_PWRITE && defined PWRITE_PROTOTYPE
PWRITE_PROTOTYPE
#endif

bool
io_unlink(const std::string & filename)
{
    if (posixy_unlink(filename.c_str()) == 0) {
	return true;
    }
    if (errno != ENOENT) {
	throw Xapian::DatabaseError(filename + ": delete failed", errno);
    }
    return false;
}

size_t
io_read(int fd, char * p, size_t n, size_t min)
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
void
io_write(int fd, const char * p, size_t n)
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

XAPIAN_NORETURN(
	static void throw_block_error(const char * s, off_t b, int e = 0));
static void
throw_block_error(const char * s, off_t b, int e)
{
    std::string m = s;
    m += str(b);
    throw Xapian::DatabaseError(m, e);
}

void
io_read_block(int fd, char * p, size_t n, off_t b)
{
    off_t o = b * n;
    // Prefer pread if available since it's typically implemented as a
    // separate syscall, and that eliminates the overhead of an extra syscall
    // per block read.
#ifdef HAVE_PREAD
    while (true) {
	ssize_t c = pread(fd, p, n, o);
	// We should get a full read most of the time, so streamline that case.
	if (usual(c == ssize_t(n)))
	    return;
	// -1 is error, 0 is EOF
	if (c <= 0) {
	    // We get EINTR if the syscall was interrupted by a signal.
	    // In this case we should retry the read.
	    if (errno == EINTR) continue;
	    if (c == 0)
		throw_block_error("EOF reading block ", b);
	    throw_block_error("Error reading block ", b, errno);
	}
	p += c;
	n -= c;
	o += c;
    }
#else
    if (rare(lseek(fd, o, SEEK_SET) == off_t(-1)))
	throw_block_error("Error seeking to block ", b, errno);
    while (true) {
	ssize_t c = read(fd, p, n);
	// We should get a full read most of the time, so streamline that case.
	if (usual(c == ssize_t(n)))
	    return;
	if (c <= 0) {
	    // We get EINTR if the syscall was interrupted by a signal.
	    // In this case we should retry the read.
	    if (errno == EINTR) continue;
	    if (c == 0)
		throw_block_error("EOF reading block ", b);
	    throw_block_error("Error reading block ", b, errno);
	}
	p += c;
	n -= c;
    }
#endif
}

void
io_write_block(int fd, const char * p, size_t n, off_t b)
{
    off_t o = b * n;
    // Prefer pwrite if available since it's typically implemented as a
    // separate syscall, and that eliminates the overhead of an extra syscall
    // per block write.
#ifdef HAVE_PWRITE
    while (true) {
	ssize_t c = pwrite(fd, p, n, o);
	// We should get a full write most of the time, so streamline that case.
	if (usual(c == ssize_t(n)))
	    return;
	if (c < 0) {
	    // We get EINTR if the syscall was interrupted by a signal.
	    // In this case we should retry the write.
	    if (errno == EINTR) continue;
	    throw_block_error("Error writing block ", b, errno);
	}
	p += c;
	n -= c;
	o += c;
    }
#else
    if (rare(lseek(fd, o, SEEK_SET) == off_t(-1)))
	throw_block_error("Error seeking to block ", b, errno);
    while (true) {
	ssize_t c = write(fd, p, n);
	// We should get a full write most of the time, so streamline that case.
	if (usual(c == ssize_t(n)))
	    return;
	if (c < 0) {
	    // We get EINTR if the syscall was interrupted by a signal.
	    // In this case we should retry the write.
	    if (errno == EINTR) continue;
	    throw_block_error("Error writing block ", b, errno);
	}
	p += c;
	n -= c;
    }
#endif
}
