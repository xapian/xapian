/** @file fdtracker.cc
 * @brief Track leaked file descriptors.
 */
/* Copyright (C) 2010,2014 Olly Betts
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

FDTracker::~FDTracker()
{
#ifndef __WIN32__
    if (dir_void) {
	DIR * dir = static_cast<DIR*>(dir_void);
	closedir(dir);
    }
#endif
}

void
FDTracker::init()
{
#ifndef __WIN32__
    DIR * dir = opendir("/proc/self/fd");
    // Not all platforms have /proc/self/fd.
    if (!dir) return;
    dir_void = static_cast<void*>(dir);

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
	if (name[0] < '0' || name[0] > '9')
	    continue;

	int fd = atoi(name);
	fds.insert(fd);
    }
#endif
}

bool
FDTracker::check()
{
    bool ok = true;
#ifndef __WIN32__
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
	    cout << "readdir failed: " << strerror(errno) << endl;
	    exit(1);
	}

	const char * name = entry->d_name;

	// Ignore at least '.' and '..'.
	if (name[0] < '0' || name[0] > '9')
	    continue;

	int fd = atoi(name);
	if (fds.find(fd) != fds.end()) continue;

	string proc_symlink = "/proc/self/fd/";
	proc_symlink += name;

	char buf[1024];
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
#endif
    return ok;
}
