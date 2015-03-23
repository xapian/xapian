/** @file io_utils.cc
 * @brief Wrappers for low-level POSIX I/O routines.
 */
/* Copyright (C) 2006,2007,2008,2009,2011,2015 Olly Betts
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

#ifdef __WIN32__
# include "msvc_posix_wrapper.h"
#endif

#include "safeerrno.h"
#include "safeunistd.h"

#include <string>

#include <xapian/error.h>

#include "noreturn.h"
#include "omassert.h"
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
#ifdef __WIN32__
    if (msvc_posix_unlink(filename.c_str()) == 0) {
#else
    if (unlink(filename.c_str()) == 0) {
#endif
	return true;
    }
    if (errno != ENOENT) {
	throw Xapian::DatabaseError(filename + ": delete failed", errno);
    }
    return false;
}

// The smallest fd we want to use for a writable handle.
const int MIN_WRITE_FD = 3;

int
io_open_block_wr(const char * fname, bool anew)
{
    int flags = O_RDWR | O_BINARY | O_CLOEXEC;
    if (anew) flags |= O_CREAT | O_TRUNC;
    int fd = ::open(fname, flags, 0666);
    if (fd >= MIN_WRITE_FD || fd < 0) return fd;

    // We want to avoid using fd < MIN_WRITE_FD, in case some other code in
    // the same process tries to write to stdout or stderr, which would end up
    // corrupting our database.
    int badfd = fd;
#ifdef F_DUPFD_CLOEXEC
    // dup to the first unused fd >= MIN_WRITE_FD.
    fd = fcntl(badfd, F_DUPFD_CLOEXEC, MIN_WRITE_FD);
    // F_DUPFD_CLOEXEC may not be supported.
    if (fd < 0 && errno == EINVAL)
#endif
#ifdef F_DUPFD
    {
	fd = fcntl(badfd, F_DUPFD, MIN_WRITE_FD);
# ifdef FD_CLOEXEC
	if (fd >= 0)
	    (void)fcntl(fd, F_SETFD, FD_CLOEXEC);
# endif
    }
    int save_errno = errno;
    (void)close(badfd);
    errno = save_errno;
#else
    {
	vector<int> toclose;
	do {
	    toclose.push_back(badfd);
	    fd = dup(badfd);
	    if (fd < 0) {
		int save_errno = errno;
		for_each(toclose.begin(), toclose.end(), close);
		errno = save_errno;
		return fd;
	    }
	    badfd = fd;
	} while (fd < MIN_WRITE_FD);
	for_each(toclose.begin(), toclose.end(), close);
# ifdef FD_CLOEXEC
	(void)fcntl(fd, F_SETFD, FD_CLOEXEC);
# endif
    }
#endif
    Assert(fd >= MIN_WRITE_FD || fd < 0);
    return fd;
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
