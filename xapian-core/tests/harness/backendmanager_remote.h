/** @file
 * @brief BackendManager subclass for remote databases.
 */
/* Copyright (C) 2008 Lemur Consulting Ltd
 * Copyright (C) 2018 Olly Betts
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

#ifndef XAPIAN_INCLUDED_BACKENDMANAGER_REMOTE_H
#define XAPIAN_INCLUDED_BACKENDMANAGER_REMOTE_H

#include "backendmanager.h"

#include <string>
#include <vector>

/// BackendManager subclass for remote databases.
class BackendManagerRemote : public BackendManager {
    /// Don't allow assignment.
    void operator=(const BackendManagerRemote &);

    /// Don't allow copying.
    BackendManagerRemote(const BackendManagerRemote &);

  protected:
    BackendManager* sub_manager;

  public:
    BackendManagerRemote(BackendManager* sub_manager_,
			 const std::string& dbtype_)
	: BackendManager(std::string(), dbtype_),
	  sub_manager(sub_manager_) {}

    /// Get the args for opening a remote database indexing a single file.
    std::string get_writable_database_args(const std::string & name,
					   const std::string & file);

    /** Get the args for opening a writable remote database with the
     *  specified timeout.
     */
    std::string get_writable_database_args(const std::string& path,
					   unsigned int timeout);

    /// Get the args for opening a remote database with the specified timeout.
    std::string get_remote_database_args(const std::vector<std::string> & files,
					 unsigned int timeout);

    /// Get the args for opening a remote database with the specified timeout.
    std::string get_remote_database_args(const std::string& name,
					 unsigned int timeout);

    /// Get the args for opening the last opened WritableDatabase.
    std::string get_writable_database_as_database_args();

    /// Get the args for opening the last opened WritableDatabase again.
    std::string get_writable_database_again_args();

    /// Get generated database path from sub_manager
    std::string get_generated_database_path(const std::string& name);

    /// Get generated database
    Xapian::WritableDatabase get_generated_database(const std::string& name);
};

#endif // XAPIAN_INCLUDED_BACKENDMANAGER_REMOTE_H
