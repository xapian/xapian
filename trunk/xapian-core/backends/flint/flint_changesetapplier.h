/** @file flint_changesetapplier.h
 * @brief Apply changesets to flint databases.
 */
/* Copyright (C) 2008 Lemur Consulting Ltd
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

#ifndef XAPIAN_INCLUDED_FLINT_CHANGESETAPPLIER_H
#define XAPIAN_INCLUDED_FLINT_CHANGESETAPPLIER_H

#include "flint_types.h"
#include <string>

class FlintChangesetApplier {
    int fd;
  public:

    /** Create a new FlintChangesetApplier object.
     *
     *  Some changesets cannot be applied directly to a live database -
     *  instead, a temporary database needs to be created, and then swapped in
     *  place of the original database.  Therefore, the FlintChangesetApplier
     *  creates a "stub" database at the specified path (which can be
     *  atomically updated to point to a new path), and creates the actual
     *  databases at separate paths.
     *
     *  If the path specified does not exist, and the first changeset read from
     *  the file descriptor is a "creation" changeset,  a stub database will be
     *  created at the specified path, and the actual database will be created
     *  in a subdirectory of the data directory supplied.

     *  @param fd_    The file descriptor to read from.
     *  @param stubpath_  The path that the resulting database should be accessed at.
     */

    FlintChangesetApplier(int fd_,
			  const std::string & stubpath_,
			  const std::string & datadir_)
	: fd(fd_), stubpath(stubpath_), datadir(datadir_)
    {}

    /** Read the next changeset from the file descriptor and apply it.
     *
     *  @return true if there are more changesets to apply, false if the final
     *  changes on the file descriptor has now been applied.
     */
    bool apply_next_changeset();
};

#endif // XAPIAN_INCLUDED_FLINT_CHANGESETAPPLIER_H
