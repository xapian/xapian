/* flint_lock.h: database locking for flint backend.
 *
 * Copyright (C) 2005 Olly Betts
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#ifndef XAPIAN_INCLUDED_FLINT_LOCK_H
#define XAPIAN_INCLUDED_FLINT_LOCK_H

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <string>

class FlintLock {
    std::string filename;
    int fd;
    pid_t pid;

  public:
    FlintLock(const std::string &filename_) : filename(filename_), fd(-1) { }

    bool lock(bool exclusive);
    void release();

    operator bool() { return fd != -1; }
};

#endif // XAPIAN_INCLUDED_FLINT_LOCK_H
