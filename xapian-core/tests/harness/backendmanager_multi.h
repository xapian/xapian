/** @file
 * @brief BackendManager subclass for multi databases.
 */
/* Copyright (C) 2007,2009,2017,2018,2019,2020 Olly Betts
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

#ifndef XAPIAN_INCLUDED_BACKENDMANAGER_MULTI_H
#define XAPIAN_INCLUDED_BACKENDMANAGER_MULTI_H

#include "backendmanager.h"

#include <string>
#include <vector>

#include <xapian/database.h>

/// BackendManager subclass for multi databases.
class BackendManagerMulti : public BackendManager {
    /// Don't allow assignment.
    void operator=(const BackendManagerMulti &);

    /// Don't allow copying.
    BackendManagerMulti(const BackendManagerMulti &);

    /// The path of the last writable database used.
    std::string last_wdb_path;

    // vector of sub_managers.
    std::vector<BackendManager*> sub_managers;

    std::string cachedir;

    std::string createdb_multi(const std::string& name,
			       const std::vector<std::string>& files);

  protected:
    /// Get the path of the Xapian::Database instance.
    std::string do_get_database_path(const std::vector<std::string> & files);

  public:
    BackendManagerMulti(const std::string& datadir_,
			const std::vector<BackendManager*>& sub_manager_);

    Xapian::Database get_remote_database(const std::vector<std::string>& files,
					 unsigned int timeout);

    /// Create a Multi Xapian::WritableDatabase object indexing a single file.
    Xapian::WritableDatabase get_writable_database(const std::string & name, const std::string & file);

    /// Get the path of Xapian::WritableDatabase instance.
    std::string get_writable_database_path(const std::string & name);

    std::string get_compaction_output_path(const std::string& name);

    std::string get_generated_database_path(const std::string& name);

    /// Create a WritableDatabase object for the last opened WritableDatabase.
    Xapian::WritableDatabase get_writable_database_again();

    /// Get the path of the last opened WritableDatabase.
    std::string get_writable_database_path_again();
};

#endif // XAPIAN_INCLUDED_BACKENDMANAGER_MULTI_H
