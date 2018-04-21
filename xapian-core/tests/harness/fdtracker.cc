/** @file fdtracker.cc
 * @brief Track leaked file descriptors.
 */
/* Copyright (C) 2010,2014,2015,2018 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
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

#include "fdtracker.h"

#ifdef XAPIAN_TESTSUITE_TRACK_FDS

#include "safeunistd.h"
#include "safedirent.h"
#include "safeerrno.h"

#include <iostream>
#include <cstdlib>
#include <cstring> // For strerror().
#include <set>

#include "str.h"
#include "stringutils.h"

using namespace std;

// Directory to try to read open fds from.  If this directory doesn't exist
// then fd tracking will just be disabled.  It seems "/dev/fd" is the more
// common name for this.  On Linux and Cygwin, "/dev/fd" is usually a symlink
// to "/proc/self/fd", but that symlink can sometimes be missing so prefer
// the latter on these platforms.
#if defined __linux__ || defined __CYGWIN__
# define FD_DIRECTORY "/proc/self/fd"
#else
# define FD_DIRECTORY "/dev/fd"
#endif

FDTracker::~FDTracker()
{
    if (dir_void) {
	DIR * dir = static_cast<DIR*>(dir_void);
	closedir(dir);
    }
}

void
FDTracker::init()
{
    DIR * dir = opendir(FD_DIRECTORY);
    // Not all platforms have such a directory.
    if (!dir) return;
    dir_void = static_cast<void*>(dir);

    int ignore_fd = dirfd(dir);
    while (true) {
	errno = 0;
	struct dirent * entry = readdir(dir);
	if (!entry) {
	    if (errno == 0)
		break;
	    cout << "readdir failed: " << strerror(errno) << endl;
	    exit(1);
	}

	const char * name = entry->d_name;

	// Ignore at least '.' and '..'.
	if (name[0] < '0' || name[0] > '9')
	    continue;

	int fd = atoi(name);
	if (fd != ignore_fd)
	    fds.insert(fd);
    }
}

bool
FDTracker::check()
{
    bool ok = true;
    DIR * dir = static_cast<DIR*>(dir_void);
    if (!dir) return true;
    rewinddir(dir);

    message.resize(0);

    int ignore_fd = dirfd(dir);
    while (true) {
	errno = 0;
	struct dirent * entry = readdir(dir);
	if (!entry) {
	    if (errno == 0)
		break;
	    cout << "readdir failed: " << strerror(errno) << endl;
	    exit(1);
	}

	const char * name = entry->d_name;

	// Ignore at least '.' and '..'.
	if (name[0] < '0' || name[0] > '9')
	    continue;

	int fd = atoi(name);
	if (fd == ignore_fd || fds.find(fd) != fds.end()) continue;

	string proc_symlink = FD_DIRECTORY "/";
	proc_symlink += name;

	char buf[1024];
	// On some systems (Solaris, AIX) the entries aren't symlinks, so
	// don't complain if readlink() fails.
	int res = readlink(proc_symlink.c_str(), buf, sizeof(buf));
	if (res == CONST_STRLEN("/dev/urandom") &&
	    memcmp(buf, "/dev/urandom", CONST_STRLEN("/dev/urandom")) == 0) {
	    // /dev/urandom isn't a real leak - something in the C library
	    // opens it lazily (at least on Linux).
	    fds.insert(fd);
	    continue;
	}

	message += ' ';
	message += str(fd);
	if (res > 0) {
	    message += " -> ";
	    message.append(buf, res);
	}

	// Insert the leaked fd so we don't report it for future tests.
	fds.insert(fd);
	ok = false;
    }
    return ok;
}

#endif // XAPIAN_TESTSUITE_TRACK_FDS
