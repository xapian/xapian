/** @file io_utils.h
 * @brief Wrappers for low-level POSIX I/O routines.
 */
/* Copyright (C) 2006,2007,2008,2009,2011,2014 Olly Betts
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

#ifndef XAPIAN_INCLUDED_IO_UTILS_H
#define XAPIAN_INCLUDED_IO_UTILS_H

#include <sys/types.h>
#include "safefcntl.h"
#include "safeunistd.h"
#include <string>

/** Ensure all data previously written to file descriptor fd has been written to
 *  disk.
 *
 *  Returns false if this could not be done.
 */
inline bool io_sync(int fd)
{
#if defined HAVE_FDATASYNC
    // If we have it, prefer fdatasync() over fsync() as the former avoids
    // updating the access time so is probably a little more efficient.
    return fdatasync(fd) == 0;
#elif defined HAVE_FSYNC
    return fsync(fd) == 0;
#elif defined __WIN32__
    return _commit(fd) == 0;
#else
# error Cannot implement io_sync() without fdatasync(), fsync(), or _commit()
#endif
}

inline bool io_full_sync(int fd)
{
#ifdef F_FULLFSYNC
    /* Only supported on Mac OS X (at the time of writing at least).
     *
     * This call ensures that data has actually been written to disk, not just
     * to the drive's write cache, so it provides better protection from power
     * failures, etc.  It does take longer though.
     *
     * According to the sqlite sources, this shouldn't fail on a local FS so
     * a failure means that the file system doesn't support this operation and
     * therefore it's best to fallback to fdatasync()/fsync().
     */
    if (fcntl(fd, F_FULLFSYNC, 0) == 0)
	return true;
#endif
    return io_sync(fd);
}

/** Read n bytes (or until EOF) into block pointed to by p from file descriptor
 *  fd.
 *
 *  If less than min bytes are read, throw an exception.
 *
 *  Returns the number of bytes actually read.
 */
size_t io_read(int fd, char * p, size_t n, size_t min);

/** Write n bytes from block pointed to by p to file descriptor fd. */
void io_write(int fd, const char * p, size_t n);

/** Delete a file.
 *
 *  @param	filename	The file to delete.
 *
 *  @exception	Xapian::DatabaseError is thrown if @a filename existed but
 *		couldn't be unlinked.
 *  @return	true if @a filename was successfully removed; false if it
 *		didn't exist.  If the file is on NFS, false may be returned
 *		even if the file was removed (if the server fails after
 *		removing the file but before telling the client, and the
 *		client then retries).
 */
bool io_unlink(const std::string & filename);

#endif // XAPIAN_INCLUDED_IO_UTILS_H
