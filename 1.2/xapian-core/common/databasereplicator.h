/** @file databasereplicator.h
 * @brief Class to manage replication of databases.
 */
/* Copyright (C) 2008 Lemur Consulting Ltd
 * Copyright (C) 2009,2010 Olly Betts
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

#ifndef XAPIAN_INCLUDED_DATABASEREPLICATOR_H
#define XAPIAN_INCLUDED_DATABASEREPLICATOR_H

#include <string>

#include "xapian/types.h"

class RemoteConnection;

namespace Xapian {

/** Base class for database replicator objects.
 *
 *  This is subclassed by each database backend which supports replication.
 */
class DatabaseReplicator {
    private:
	/// Copies are not allowed.
	DatabaseReplicator(const DatabaseReplicator &);

	/// Assignment is not allowed.
	void operator=(const DatabaseReplicator &);

    protected:
	/** Constructor to allow construction of subclasses from the open()
	 *  method.
	 */
	DatabaseReplicator() { }

    public:
	/** Destroy the replicator.
	 */
        virtual ~DatabaseReplicator();

	/** Open a DatabaseReplicator for the given path.
	 *
	 *  The type of the database at the path is automatically detected.
	 */
	static DatabaseReplicator * open(const std::string & path);

	/** Check if the revision of the database is at least that of a target.
	 *
	 *  @param rev The database revision.
	 *  @param target The target revision.
	 */
	virtual bool check_revision_at_least(const std::string & rev,
					     const std::string & target) const = 0;

	/** Read and apply the next changeset.
	 *
	 *  @param conn The remote connection manager.
	 *
	 *  @param end_time The time to timeout at.
	 *
	 *  @param db_valid Whether the database is known to be valid at the
	 *  start of the changeset.  If this is true, some additional checks
	 *  are performed.
	 */
	virtual std::string apply_changeset_from_conn(RemoteConnection & conn,
						      double end_time,
						      bool db_valid) const = 0;

	/** Get a UUID for the replica.
	 *
	 *  If the UUID cannot be read (for example, because the database is
	 *  not valid), this should return the empty string, rather than
	 *  raising an exception.
	 */
	virtual std::string get_uuid() const = 0;
};

}

#endif /* XAPIAN_INCLUDED_DATABASEREPLICATOR_H */
