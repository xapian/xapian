/** @file backendmanager_remoteprog.h
 * @brief BackendManager subclass for remoteprog databases.
 */
/* Copyright (C) 2007 Olly Betts
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

#include <string>

#include <xapian/base.h>
#include <xapian/types.h>
#include <xapian/postingiterator.h>

#include "positionlist.h"

/// BackendManager subclass for remoteprog databases.
class BackendManagerRemoteProg : public BackendManager {
    /// Don't allow assignment.
    void operator=(const BackendManagerRemoteProg &);

    /// Don't allow copying.
    BackendManagerRemoteProg(const BackendManagerRemoteProg &);

    /// The path of the last writable database used.
    std::string last_wdb_name;

  public:
    BackendManagerRemoteProg() { }

    /** We have virtual methods and want to be able to delete derived classes
     *  using a pointer to the base class, so we need a virtual destructor.
     */
    virtual ~BackendManagerRemoteProg();

    /// Return a string representing the current database type.
    const char * get_dbtype() const;

    /// Create a RemoteProg Xapian::Database object indexing multiple files.
    Xapian::Database get_database(const std::vector<std::string> & files);

    /// Create a RemoteProg Xapian::Database object indexing a single file.
    Xapian::Database get_database(const std::string & file);

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
