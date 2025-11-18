/** @file
 * @brief Wrappers for low-level POSIX I/O routines.
 */
/* Copyright (C) 2006-2025 Olly Betts
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

#ifndef PACKAGE
# error config.h must be included first in each C++ source file
#endif

#include <sys/types.h>
#include "safefcntl.h"
#include "safeunistd.h"
#include <limits>
#include <string>

/** Open a block-based file for reading.
 *
 *  @param filename  The path of the file to open.
 */
inline int io_open_block_rd(const char* filename) {
    return ::open(filename, O_RDONLY | O_BINARY | O_CLOEXEC);
}

/** Open a block-based file for reading.
 *
 *  @param filename  The path of the file to open.
 */
inline int io_open_block_rd(const std::string& filename)
{
    return io_open_block_rd(filename.c_str());
}

/** Open a block-based file for writing.
 *
 *  @param filename  The path of the file to open.
 *  @param anew   If true, open the file anew (create or truncate it).
 */
int io_open_block_wr(const char* filename, bool anew);

/** Open a block-based file for writing.
 *
 *  @param filename  The path of the file to open.
 *  @param anew  If true, open the file anew (create or truncate it).
 */
inline int io_open_block_wr(const std::string& filename, bool anew)
{
    return io_open_block_wr(filename.c_str(), anew);
}

/** Open a stream-based file for reading.
 *
 *  @param filename  The path of the file to open.
 */
inline int io_open_stream_rd(const char* filename) {
    return ::open(filename, O_RDONLY | O_BINARY | O_CLOEXEC);
}

/** Open a stream-based file for reading.
 *
 *  @param filename  The path of the file to open.
 */
inline int io_open_stream_rd(const std::string& filename)
{
    return io_open_stream_rd(filename.c_str());
}

/** Open a stream-based file for writing.
 *
 *  @param filename  The path of the file to open.
 *  @param anew   If true, open the file anew (create or truncate it).
 */
int io_open_stream_wr(const char* filename, bool anew);

/** Open a stream-based file for writing.
 *
 *  @param filename  The path of the file to open.
 *  @param anew  If true, open the file anew (create or truncate it).
 */
inline int io_open_stream_wr(const std::string& filename, bool anew)
{
    return io_open_stream_wr(filename.c_str(), anew);
}

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
    /* Only supported on macOS (at the time of writing at least).
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
 *  If a read error occurs, throws DatabaseError.
 *
 *  If @a min is specified and EOF is reached after less than @a min bytes,
 *  throws DatabaseCorruptError.
 *
 *  Returns the number of bytes actually read.
 */
size_t io_read(int fd, char * p, size_t n, size_t min = 0);

/** Write n bytes from block pointed to by p to file descriptor fd. */
void io_write(int fd, const char * p, size_t n);

inline void io_write(int fd, const unsigned char * p, size_t n) {
    io_write(fd, reinterpret_cast<const char *>(p), n);
}

/** Read n bytes (or until EOF) into block pointed to by p from file descriptor
 *  fd starting at position o.
 *
 *  If a read error occurs, throws DatabaseError.
 *
 *  If @a min is specified and EOF is reached after less than @a min bytes,
 *  throws DatabaseCorruptError.
 *
 *  The current file position may or may not be updated.
 *
 *  Returns the number of bytes actually read.
 */
size_t io_pread(int fd, char * p, size_t n, off_t o, size_t min = 0);

/** Write n bytes from block pointed to by p to file descriptor fd starting at
 *  position o.
 *
 *  If a read error occurs, throws DatabaseError.
 *
 *  The current file position may or may not be updated.
 */
void io_pwrite(int fd, const char * p, size_t n, off_t o);

/** Readahead block b size n bytes from file descriptor fd.
 *
 *  Returns false if we can't readahead on this fd.
 */
#ifdef HAVE_POSIX_FADVISE
bool io_readahead_block(int fd, size_t n, off_t b, off_t o = 0);
#else
inline bool io_readahead_block(int, size_t, off_t, off_t = 0) { return false; }
#endif

/// Read block b size n bytes into buffer p from file descriptor fd, offset o.
void io_read_block(int fd, char * p, size_t n, off_t b, off_t o = 0);

/// Write block b size n bytes from buffer p to file descriptor fd, offset o.
void io_write_block(int fd, const char * p, size_t n, off_t b, off_t o = 0);

inline void io_write_block(int fd, const unsigned char * p, size_t n, off_t b) {
    io_write_block(fd, reinterpret_cast<const char *>(p), n, b);
}

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

/** Rename a temporary file to its final position.
 *
 *  Attempts to deal with NFS infelicities.  If the rename fails, the temporary
 *  file is removed.
 *
 *  @return	true if the rename succeeded; false if it failed (and errno will
 *		be set appropriately).
 */
bool io_tmp_rename(const std::string & tmp_file, const std::string & real_file);

/** Protect against stray writes to fds we use pwrite() on.
 *
 *  Set the file position high to protect against user code or other libraries
 *  accidentally trying to write to our fd.  To avoid problems we're rolling
 *  this out gradually on platforms we've tested it on.
 */
static inline void io_protect_from_write(int fd) {
#if !defined HAVE_PREAD || !defined HAVE_PWRITE
    // No point setting the file position high here as it'll just get reset
    // by the first block read or write.
    (void)fd;
#elif defined __linux__
    // The maximum off_t value works for at least btrfs.
    if (lseek(fd, std::numeric_limits<off_t>::max(), SEEK_SET) < 0) {
	if constexpr (sizeof(off_t) > 4) {
	    // Try the actual maximum for ext4 (which matches the documented
	    // maximum filesize) since ext4 is very widely used.
	    (void)lseek(fd, off_t(0xffffffff000), SEEK_SET);
	}
    }
#elif defined _AIX
    // It seems prudent to try the maximum off_t value first.
    if (lseek(fd, std::numeric_limits<off_t>::max(), SEEK_SET) < 0) {
	if constexpr (sizeof(off_t) > 4) {
	    // Actual maximum seen in testing AIX 7.1 and 7.3 on JFS.
	    (void)lseek(fd, off_t(0xffffffff000), SEEK_SET);
	}
    }
#elif defined __CYGWIN__ || \
      defined __DragonFly__ || \
      defined __FreeBSD__ || \
      defined __APPLE__ || \
      defined __NetBSD__ || \
      defined __OpenBSD__ || \
      defined __sun__
    // The maximum off_t value worked in testing on:
    // * Cygwin 3.6.5
    // * DragonFlyBSD 6.4.2
    // * FreeBSD 14.0 and 15.0
    // * macOS 10.10 and 12.6
    // * NetBSD 10.0
    // * OpenBSD 7.5
    // * Solaris 10 and 11.4
    (void)lseek(fd, std::numeric_limits<off_t>::max(), SEEK_SET);
#elif defined __EMSCRIPTEN__
    if constexpr (sizeof(off_t) > 4) {
	// Anything larger fails with EOVERFLOW (tested with Emscripten SDK
	// 4.0.19).
	(void)lseek(fd, off_t(0x20000000000000), SEEK_SET);
    }
#else
    (void)fd;
#endif
}

#endif // XAPIAN_INCLUDED_IO_UTILS_H
