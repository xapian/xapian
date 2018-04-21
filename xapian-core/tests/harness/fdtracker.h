/** @file fdtracker.h
 * @brief Track leaked file descriptors.
 */
/* Copyright (C) 2010,2018 Olly Betts
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

#ifndef XAPIAN_INCLUDED_FDTRACKER_H
#define XAPIAN_INCLUDED_FDTRACKER_H

#include <set>
#include <string>

// Disable fd tracking where it can't possibly work.
#if !defined __WIN32__ && defined HAVE_DIRFD
# define XAPIAN_TESTSUITE_TRACK_FDS
#endif

class FDTracker {
#ifdef XAPIAN_TESTSUITE_TRACK_FDS
    std::set<int> fds;

    /** The DIR* from opendir("/proc/self/fd") (or equivalent) cast to void*.
     *
     *  We store this cast to void* here to minimise the header we have to
     *  include here.
     */
    void * dir_void;

    std::string message;

  public:
    FDTracker() : dir_void(NULL) { }

    ~FDTracker();

    void init();

    bool check();

    const std::string & get_message() const { return message; }
#else
  public:
    FDTracker() { }

    void init() { }

    bool check() { return true; }

    std::string get_message() const { return std::string(); }
#endif

    FDTracker(const FDTracker&) = delete;

    FDTracker& operator=(const FDTracker&) = delete;
};

#endif // XAPIAN_INCLUDED_FDTRACKER_H
