/** @file
 * @brief Flint-compatible database locking.
 */
/* Copyright (C) 2005,2006,2007,2008,2009,2012,2014,2016,2017 Olly Betts
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

#ifndef PACKAGE
# error config.h must be included first in each C++ source file
#endif

#include <string>

#if defined __CYGWIN__ || defined __WIN32__
# include "safewindows.h"
#else
# include <sys/types.h>
#endif

class FlintLock {
    std::string filename;
#if defined __CYGWIN__ || defined __WIN32__
    HANDLE hFile = INVALID_HANDLE_VALUE;
#elif defined FLINTLOCK_USE_FLOCK
    int fd = -1;
#else
    int fd = -1;
    pid_t pid;
#endif

  public:
    typedef enum {
	SUCCESS, // We got the lock!
	INUSE, // Already locked by someone else.
	UNSUPPORTED, // Locking probably not supported (e.g. NFS without lockd).
	FDLIMIT, // Process hit its file descriptor limit.
	UNKNOWN // Locking failed for some unspecified reason (keep this last).
    } reason;

    /** Standard constructor. */
    explicit FlintLock(const std::string &filename_)
	: filename(filename_) {
	// Keep the same lockfile name as flint since the locking is compatible
	// and this avoids the possibility of creating two databases in the
	// same directory using different backends.
	filename += "/flintlock";
    }

    /** Constructor for use in read-only cases (like single-file glass). */
    FlintLock() {}

    operator bool() const {
#if defined __CYGWIN__ || defined __WIN32__
	return hFile != INVALID_HANDLE_VALUE;
#else
	return fd != -1;
#endif
    }

    // Release any lock held when we're destroyed.
    ~FlintLock() { release(); }

    /** Test if the lock is held.
     *
     *  If this object holds the lock, just returns true.  Otherwise it will
     *  try to test taking the lock, if that is possible to do on the current
     *  platform without actually take it (fcntl() locks support this).
     *
     *	Throws Xapian::UnimplemenetedError if the platform doesn't support
     *	testing the lock in this way, or Xapian::DatabaseLockError if there's
     *	an error while trying to perform the test.
     */
    bool test() const;

    /** Attempt to obtain the lock.
     *
     *  If the attempt fails with code "UNKNOWN", the string supplied in the
     *  explanation parameter will be set to contain any details available of
     *  the reason for the failure.
     *
     *	   @param exclusive  Get an exclusive lock?  Value currently ignored,
     *			     and the lock is always exclusive.
     *	   @param wait	     Wait until we can get the lock?
     */
    reason lock(bool exclusive, bool wait, std::string & explanation);

    /// Release the lock.
    void release();

    /// Throw Xapian::DatabaseLockError.
    [[noreturn]]
    void throw_databaselockerror(FlintLock::reason why,
				 const std::string & db_dir,
				 const std::string & explanation) const;
};

#endif // XAPIAN_INCLUDED_FLINT_LOCK_H
