/* flint_lock.h: database locking for flint backend.
 *
 * Copyright (C) 2005,2006,2007 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef XAPIAN_INCLUDED_FLINT_LOCK_H
#define XAPIAN_INCLUDED_FLINT_LOCK_H

#include <string>

#if defined __CYGWIN__ || defined __WIN32__
#include "safewindows.h"
#else
#include "safefcntl.h"
#include <unistd.h>
#include <stdlib.h>
#ifdef _NEWLIB_VERSION
// Workaround bug in newlib (at least some versions) - socklen_t doesn't
// get defined if you just "#include <sys/socket.h>".
#include <netinet/in.h>
#endif
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

class FlintLock {
    std::string filename;
#if defined __CYGWIN__ || defined __WIN32__
    HANDLE hFile;
#else
    int fd;
    pid_t pid;
#endif

  public:
#if defined __CYGWIN__ || defined __WIN32__
    FlintLock(const std::string &filename_)
	: filename(filename_), hFile(INVALID_HANDLE_VALUE) { }
    operator bool() { return hFile != INVALID_HANDLE_VALUE; }
#else
    FlintLock(const std::string &filename_) : filename(filename_), fd(-1) { }
    operator bool() { return fd != -1; }
#endif

    bool lock(bool exclusive);
    void release();
};

#endif // XAPIAN_INCLUDED_FLINT_LOCK_H
