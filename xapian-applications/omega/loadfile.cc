/* loadfile.cc: load a file into a std::string.
 *
 * Copyright (C) 2006,2010,2015 Olly Betts
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

#ifdef HAVE_POSIX_FADVISE
# ifdef __linux__
#  define _POSIX_C_SOURCE 200112L // for posix_fadvise from fcntl.h
#  define _DEFAULT_SOURCE 1 // Needed to get lstat() for glibc >= 2.20
#  define _BSD_SOURCE 1 // Needed to get lstat() for glibc < 2.20
# endif
#endif

#include "loadfile.h"

#include <algorithm>
#include <string>

#include "safeerrno.h"
#include "safefcntl.h"
#include <sys/types.h>
#include "safesysstat.h"
#include "safeunistd.h"

using namespace std;

bool
load_file(const string &file_name, size_t max_to_read, int flags,
	  string &output, bool &truncated)
{
    (void)flags; // Avoid possible "unused" warning.
    mode_t mode = O_RDONLY;
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
    if (flags & NOCACHE)
	posix_fadvise(fd, 0, 0, POSIX_FADV_NOREUSE); // or POSIX_FADV_SEQUENTIAL
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

    char blk[4096];
    size_t n = st.st_size;
    truncated = (max_to_read && max_to_read < n);
    if (truncated) {
	n = max_to_read;
	truncated = true;
    }

    output.resize(0);
    output.reserve(n);
    while (n) {
	int c = read(fd, blk, min(n, sizeof(blk)));
	if (c <= 0) {
	    if (c < 0 && errno == EINTR) continue;
	    break;
	}
	output.append(blk, c);
	n -= c;
    }

#ifdef HAVE_POSIX_FADVISE
    if (flags & NOCACHE)
	posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED);
#endif

    close(fd);

    return true;
}
