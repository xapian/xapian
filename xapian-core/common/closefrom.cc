/** @file closefrom.cc
 * @brief Implementation of closefrom() function.
 */
/* Copyright (C) 2010,2011,2012,2016 Olly Betts
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

// We don't currently need closefrom() on __WIN32__.
#if !defined HAVE_CLOSEFROM && !defined __WIN32__

#include "closefrom.h"

#include "safeerrno.h"
#include "safefcntl.h"
#include "safeunistd.h"

#ifdef HAVE_SYS_RESOURCE_H
# include <sys/types.h>
# include <sys/resource.h>
#endif

#if defined __linux__ || defined __APPLE__
# include "safedirent.h"
# include <cstdlib>

using namespace std;
#endif

static int
get_maxfd() {
#ifdef F_MAXFD
    int maxfd = fcntl(0, F_MAXFD);
    if (maxfd >= 0) return maxfd;
#endif
#ifdef HAVE_GETRLIMIT
    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) == 0 &&
	rl.rlim_max != RLIM_INFINITY) {
	return static_cast<int>(rl.rlim_max) - 1;
    }
#endif
    return static_cast<int>(sysconf(_SC_OPEN_MAX)) - 1;
}

void
Xapian::Internal::closefrom(int fd)
{
    int maxfd = -1;
#ifdef F_CLOSEM
    // Apparently supported by at least NetBSD, AIX, IRIX.
    if (fcntl(fd, F_CLOSEM, 0) >= 0)
	return;
#elif defined __linux__ || defined __APPLE__
#if 0
    // Some platforms (e.g. AIX) have /proc/<pid>/fd but not /proc/self - if
    // any such platforms don't have either closefrom() or F_CLOSEM but do
    // have getdirentries() then this code can be used.
    char path[6 + sizeof(pid_t) * 3 + 4];
    sprintf(path, "/proc/%ld/fd", long(getpid()));
#elif defined __linux__
    const char * path = "/proc/self/fd";
#elif defined __APPLE__ // Mac OS X
    const char * path = "/dev/fd";
#endif
    int dir = open(path, O_RDONLY|O_DIRECTORY);
    if (dir >= 0) {
	off_t base = 0;
	while (true) {
	    char buf[1024];
	    errno = 0;
	    // We use getdirentries() instead of opendir()/readdir() here
	    // because the latter can call malloc(), which isn't safe to do
	    // between fork() and exec() in a multi-threaded program.
	    ssize_t c = getdirentries(dir, buf, sizeof(buf), &base);
	    if (c == 0) {
		close(dir);
		return;
	    }
	    if (c < 0) {
		// Fallback if getdirentries() fails.
		break;
	    }
	    struct dirent *d;
	    for (ssize_t pos = 0; pos < c; pos += d->d_reclen) {
		d = reinterpret_cast<struct dirent*>(buf + pos);
		const char * leaf = d->d_name;
		if (leaf[0] < '0' || leaf[0] > '9') {
		    // Skip '.' and '..'.
		    continue;
		}
		int n = atoi(leaf);
		if (n < fd) {
		    // FD below threshold.
		    continue;
		}
		if (n == dir) {
		    // Don't close the fd open on the directory.
		    continue;
		}
#ifdef __linux__
		// Running under valgrind causes some entries above the
		// reported RLIMIT_NOFILE value to appear in
		// /proc/self/fd - see:
		// https://bugs.kde.org/show_bug.cgi?id=191758
		//
		// If we try to close these, valgrind issues a warning about
		// trying to close an invalid file descriptor.  These entries
		// start at 1024, so we check that value first so we can
		// usually avoid having to read the fd limit when we're not
		// running under valgrind.
		if (n >= 1024) {
		    if (maxfd < 0)
			maxfd = get_maxfd();
		    if (n > maxfd)
			continue;
		}
#endif
		// Retry on EINTR.
		while (close(n) < 0 && errno == EINTR) { }
	    }
	}
	close(dir);
    }
#endif
    if (maxfd < 0)
	maxfd = get_maxfd();
    while (fd <= maxfd) {
	// Retry on EINTR; just ignore other errors (we'll get EBADF if fd
	// isn't open so that's OK).
	while (close(fd) < 0 && errno == EINTR) { }
	++fd;
    }
}

#endif
