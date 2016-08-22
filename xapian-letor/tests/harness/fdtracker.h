/** @file fdtracker.h
 * @brief Track leaked file descriptors.
 */
/* Copyright (C) 2010 Olly Betts
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

class FDTracker {
    std::set<int> fds;

    void * dir_void;

    std::string message;

  public:

    FDTracker() : dir_void(NULL) { }

    ~FDTracker();

    void init();

    bool check();

    const std::string & get_message() const { return message; }
};

#endif // XAPIAN_INCLUDED_FDTRACKER_H
