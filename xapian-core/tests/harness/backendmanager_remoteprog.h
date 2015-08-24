/** @file backendmanager_remoteprog.h
 * @brief BackendManager subclass for remoteprog databases.
 */
/* Copyright (C) 2007,2009 Olly Betts
 * Copyright (C) 2008 Lemur Consulting Ltd
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

#ifndef XAPIAN_INCLUDED_BACKENDMANAGER_REMOTEPROG_H
#define XAPIAN_INCLUDED_BACKENDMANAGER_REMOTEPROG_H

#include "backendmanager.h"
#include "backendmanager_remote.h"

#include <string>

#include <xapian/types.h>
#include <xapian/postingiterator.h>

#include "positionlist.h"

/// BackendManager subclass for remoteprog databases.
class BackendManagerRemoteProg : public BackendManagerRemote {
    /// Don't allow assignment.
    void operator=(const BackendManagerRemoteProg &);

    /// Don't allow copying.
    BackendManagerRemoteProg(const BackendManagerRemoteProg &);

    /// The path of the last writable database used.
    std::string last_wdb_name;

  private:
    /// Create a Xapian::Database object indexing multiple files.
    Xapian::Database do_get_database(const std::vector<std::string> & files);

  public:
    BackendManagerRemoteProg(const std::string & remote_type_)
	: BackendManagerRemote(remote_type_) { }

    /// Return a string representing the current database type.
    std::string get_dbtype() const;

    /// Create a RemoteProg Xapian::WritableDatabase object indexing a single file.
    Xapian::WritableDatabase get_writable_database(const std::string & name,
						   const std::string & file);

    /// Create a RemoteProg Xapian::Database with the specified timeout.
    Xapian::Database get_remote_database(const std::vector<std::string> & files,
					 unsigned int timeout);

    /// Create a Database object for the last opened WritableDatabase.
    Xapian::Database get_writable_database_as_database();

    /// Create a WritableDatabase object for the last opened WritableDatabase.
    Xapian::WritableDatabase get_writable_database_again();
};

#endif // XAPIAN_INCLUDED_BACKENDMANAGER_REMOTEPROG_H
