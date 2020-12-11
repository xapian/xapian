/** @file
 * @brief load a file into a std::string.
 */
/* Copyright (C) 2006,2010,2012,2015,2018 Olly Betts
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

#include "loadfile.h"

#include <algorithm>
#include <cerrno>
#include <string>

#include "safefcntl.h"
#include <sys/types.h>
#include "safesysstat.h"
#include "safeunistd.h"

using namespace std;

bool
load_file_from_fd(int fd, string& output)
{
    output.resize(0);
    char blk[4096];
    while (true) {
	ssize_t c = read(fd, blk, sizeof(blk));
	if (c <= 0) {
	    if (c == 0) break;
	    if (errno == EINTR) continue;
	    return false;
	}
	output.append(blk, c);
    }

    return true;
}

bool
load_file(const string& file_name, size_t max_to_read, int flags,
	  string& output, bool* truncated)
{
    mode_t mode = O_BINARY | O_RDONLY;
#if defined O_NOATIME && O_NOATIME != 0
    if (flags & NOATIME) mode |= O_NOATIME;
#endif

    int fd = open(file_name.c_str(), mode);
#if defined O_NOATIME && O_NOATIME != 0
    if (fd < 0 && (mode & O_NOATIME)) {
	mode &= ~O_NOATIME;
	fd = open(file_name.c_str(), mode);
    }
#endif
    if (fd < 0) return false;

#ifdef HAVE_POSIX_FADVISE
# ifndef __linux__
    // On Linux, POSIX_FADV_NOREUSE has been a no-op since 2.6.18 (released
    // 2006) and before that it was incorrectly implemented as an alias for
    // POSIX_FADV_WILLNEED.  There have been a few attempts to make
    // POSIX_FADV_NOREUSE actually work on Linux but nothing has been merged so
    // for now let's not waste effort making a syscall we know to currently be
    // a no-op.  We can revise this conditional if it gets usefully
    // implemented.
    if (flags & NOCACHE)
	posix_fadvise(fd, 0, 0, POSIX_FADV_NOREUSE);
# endif
#endif

    struct stat st;
    if (fstat(fd, &st) < 0) {
	int errno_save = errno;
	close(fd);
	errno = errno_save;
	return false;
    }

    if (!S_ISREG(st.st_mode)) {
	close(fd);
	errno = EINVAL;
	return false;
    }

    size_t n = st.st_size;
    if (max_to_read && max_to_read < n) {
	n = max_to_read;
	if (truncated) *truncated = true;
    } else {
	if (truncated) *truncated = false;
    }

    output.resize(0);
    output.reserve(n);
    while (n) {
	char blk[4096];
	int c = read(fd, blk, min(n, sizeof(blk)));
	if (c <= 0) {
	    if (c == 0) break;
	    if (errno == EINTR) continue;
	    return false;
	}
	output.append(blk, c);
	n -= c;
    }

    if (flags & NOCACHE) {
#ifdef HAVE_POSIX_FADVISE
# ifdef __linux__
	// Linux doesn't implement POSIX_FADV_NOREUSE so instead we use
	// POSIX_FADV_DONTNEED just before closing the fd.  This is a bit more
	// aggressive than we ideally want - really we just want to stop our
	// reads from pushing other pages out of the OS cache, but if the
	// pages we read are already cached it would probably be better to
	// leave them cached after the read.
	posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED);
# endif
#endif
    }
    close(fd);

    return true;
}
