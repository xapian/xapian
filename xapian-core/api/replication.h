/** @file
 * @brief Replication support for Xapian databases.
 */
/* Copyright 2008 Lemur Consulting Ltd
 * Copyright 2008,2011,2015,2016 Olly Betts
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

#ifndef XAPIAN_INCLUDED_REPLICATION_H
#define XAPIAN_INCLUDED_REPLICATION_H

#include "xapian/visibility.h"

#include <string>

namespace Xapian {

/** Information about the steps involved in performing a replication. */
struct ReplicationInfo {
    /// Number of changesets applied.
    int changeset_count;

    /// Number of times a full database copy was performed.
    int fullcopy_count;

    /** True if and only if the replication corresponds to a change in the
     *  live version of the database.  Note that this may be false even if
     *  changeset_count and fullcopy_count are non-zero, since the changes
     *  may have been made to a non-live copy of the database.
     */
    bool changed;

    ReplicationInfo()
	: changeset_count(0),
	  fullcopy_count(0),
	  changed(false)
    {}

    void clear() {
	changeset_count = 0;
	fullcopy_count = 0;
	changed = false;
    }
};

/** Access to a master database for replication. */
class XAPIAN_VISIBILITY_DEFAULT DatabaseMaster {
    /// The path to the master database.
    std::string path;

  public:
    /** Create a new DatabaseMaster for the database at the specified path.
     *
     *  The database isn't actually opened until a set of changesets is
     *  requested.
     */
    explicit DatabaseMaster(const std::string & path_) : path(path_) {}

    /** Write a set of changesets for upgrading the database to a file.
     *
     *  The changesets will be such that, if they are applied in order to a
     *  copy of the database at the start revision, a copy of the database
     *  at the current revision (i.e. the revision which the database
     *  object is currently open at) will be produced.
     *
     *  If suitable changesets have been stored in the database, this will
     *  write the appropriate changesets, in order.  If suitable changesets
     *  are not available, this will write a copy of sufficient blocks of
     *  the database to reconstruct the current revision.
     *
     *  This will therefore potentially write a very large amount of data
     *  to the file descriptor.
     *
     *  @param fd       An open file descriptor to write the changes to.
     *
     *  @param start_revision The starting revision of the database that the
     *                  changesets are to be applied to.  Specify an empty
     *                  string to get a "creation" changeset, which
     *                  includes the creation of the database.  The
     *                  revision will include the unique identifier for the
     *                  database, if one is available.
     *
     *  @param info     If non-NULL, the supplied structure will be updated
     *                  to reflect the changes written to the file
     *                  descriptor.
     */
    void write_changesets_to_fd(int fd,
				const std::string & start_revision,
				ReplicationInfo * info) const;

    /// Return a string describing this object.
    std::string get_description() const;
};

/** Access to a database replica, for applying replication to it.
 *
 *  Warning: the replication interface is currently experimental, and is liable
 *  to change between releases without warning.
 */
class XAPIAN_VISIBILITY_DEFAULT DatabaseReplica {
    /// Class holding details of the replica.
    class Internal;
    /// Internals.
    Internal * internal;

    /// No copying.
    DatabaseReplica(const DatabaseReplica & other);

    /// No assignment.
    void operator=(const DatabaseReplica & other);

  public:
    /// Destructor.
    ~DatabaseReplica();

    /** Open a DatabaseReplica for the database at the specified path.
     *
     *  The path should either point to a database previously created by a
     *  DatabaseReplica, or to a path which doesn't yet exist.
     *
     *  The path should always be in a directory which exists.
     *
     *  If the specified path does not contain a database, a database will be
     *  created when an appropriate changeset is supplied to the replica.
     *
     *  @param path       The path to make the replica at.
     */
    explicit DatabaseReplica(const std::string & path);

    /** Get a string describing the current revision of the replica.
     *
     *  The revision information includes a unique identifier for the master
     *  database that the replica is of, as well as information about the exact
     *  revision of the master database that the replica represents.  This
     *  information allows the master database to send the appropriate
     *  changeset to mirror whatever changes have been made on the master.
     */
    std::string get_revision_info() const;

    /** Set the file descriptor to read changesets from.
     *
     *  This will be remembered in the DatabaseReplica, but the caller is still
     *  responsible for closing it after it is finished with.
     *
     *  @param fd The file descriptor to read the changeset from.
     */
    void set_read_fd(int fd);

    /** Read and apply the next changeset.
     *
     *  If no changesets are found on the file descriptor, returns false
     *  immediately.
     *
     *  If any changesets are found on the file descriptor, exactly one of them
     *  is applied.
     *
     *  A common way to use this method is to call it repeatedly until it
     *  returns false, with an appropriate gap between each call.
     *
     *  Information beyond the end of the next changeset may be read from the
     *  file descriptor and cached in the DatabaseReplica object.  Therefore,
     *  the file descriptor shouldn't be accessed by any other external code,
     *  since it will be in an indeterminate state.
     *
     *  Note that if this raises an exception (other than DatabaseCorruptError)
     *  the database will be left in a valid and consistent state.  It
     *  may or may not be changed from its initial state, and may or may not be
     *  fully synchronised with the master database.
     *
     *  @param info     If non-NULL, the supplied structure will be updated
     *                  to reflect the changes read from the file descriptor.
     *  @param reader_close_time	Wait at least this many seconds between
     *					applying changesets to allow active
     *					readers to finish.
     *
     *  @return true if there are more changesets to apply on the file
     *  descriptor, false otherwise.
     */
    bool apply_next_changeset(ReplicationInfo * info,
			      double reader_close_time);

    /// Return a string describing this object.
    std::string get_description() const;
};

}

#endif /* XAPIAN_INCLUDED_REPLICATION_H */
