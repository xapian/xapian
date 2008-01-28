/** @file replication.cc
 * @brief Replication support for Xapian databases.
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

#include <config.h>

#include <xapian/replication.h>

#include "omdebug.h"

#include <string>

using namespace std;
using namespace Xapian;

void
DatabaseMaster::write_changesets_to_fd(int fd,
				       const string & start_revision) const
{
    DEBUGAPICALL(string, "DatabaseMaster::write_changesets_to_fd",
		 fd << ", " << start_revision);
    // FIXME - implement
    (void) fd;
    (void) start_revision;
}

DatabaseReplica::DatabaseReplica(const std::string & path_)
	: path(path_), db()
{
}

string
DatabaseReplica::get_revision_info() const
{
    DEBUGAPICALL(string, "DatabaseReplica::get_revision_info", "");
    RETURN("");
}

bool 
DatabaseReplica::apply_next_changeset_from_fd(int fd)
{
    DEBUGAPICALL(bool, "DatabaseReplica::apply_next_changeset_from_fd", fd);
    // FIXME - implement
    (void) fd;
    RETURN(false);
}
