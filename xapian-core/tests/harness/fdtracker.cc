/** @file
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

#include <iostream>
#include <cerrno>
#include <cstdlib>
#include <cstring> // For memcmp().

#include "errno_to_string.h"
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

void
FDTracker::mark_fd(int fd)
{
    if (fd >= 0) {
	if (size_t(fd) >= fds.size())
	    fds.resize((fd &~ 31) + 32);
	fds[fd] = true;
    }
}

bool
FDTracker::check_fd(int fd) const
{
    return size_t(fd) < fds.size() && fds[fd];
}

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

    // The list of fds we get will include the fd inside dir, but that's OK as
    // we keep dir open while the testcase runs and use it to recheck the open
    // fds at the end.
    while (true) {
	errno = 0;
	struct dirent * entry = readdir(dir);
	if (!entry) {
	    if (errno == 0)
		break;
	    cout << "readdir failed: " << errno_to_string(errno) << '\n';
	    exit(1);
	}

	const char * name = entry->d_name;

	// Ignore at least '.' and '..'.
	if (name[0] < '0' || name[0] > '9')
	    continue;

	mark_fd(atoi(name));
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

    while (true) {
	errno = 0;
	struct dirent * entry = readdir(dir);
	if (!entry) {
	    if (errno == 0)
		break;
	    cout << "readdir failed: " << errno_to_string(errno) << '\n';
	    exit(1);
	}

	const char * name = entry->d_name;

	// Ignore at least '.' and '..'.
	if (name[0] < '0' || name[0] > '9')
	    continue;

	int fd = atoi(name);
	if (check_fd(fd)) {
	    // This fd was already open before the testcase.
	    continue;
	}

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
	    mark_fd(fd);
	    continue;
	}

	message += ' ';
	message += str(fd);
	if (res > 0) {
	    message += " -> ";
	    message.append(buf, res);
	}

	// Mark the leaked fd as used so we don't report it for future tests.
	mark_fd(fd);
	ok = false;
    }
    return ok;
}

#endif // XAPIAN_TESTSUITE_TRACK_FDS
