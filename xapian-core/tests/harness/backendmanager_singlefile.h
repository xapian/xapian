/** @file
 * @brief BackendManager subclass for singlefile databases.
 */
/* Copyright (C) 2007,2009,2015,2018,2023 Olly Betts
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

#ifndef XAPIAN_INCLUDED_BACKENDMANAGER_SINGLEFILE_H
#define XAPIAN_INCLUDED_BACKENDMANAGER_SINGLEFILE_H

#include "backendmanager.h"

#include <string>

#include <xapian/database.h>

/// BackendManager subclass for singlefile databases.
class BackendManagerSingleFile : public BackendManager {
    /// Don't allow assignment.
    void operator=(const BackendManagerSingleFile &);

    /// Don't allow copying.
    BackendManagerSingleFile(const BackendManagerSingleFile &);

    BackendManager* sub_manager;

    std::string cachedir;

  protected:
    /// Get the path of the Xapian::Database instance.
    std::string do_get_database_path(const std::vector<std::string> & files);

  public:
    BackendManagerSingleFile(const std::string& datadir_,
			     BackendManager* sub_manager_);

    /// Get generated database.
    Xapian::WritableDatabase get_generated_database(const std::string& name);

    /// Finalise generated database.
    void finalise_generated_database(const std::string& name);

    /// Create a Xapian::WritableDatabase object.
    Xapian::WritableDatabase get_writable_database(const std::string & name, const std::string & file);

    std::string get_generated_database_path(const std::string& name);

    std::string get_compaction_output_path(const std::string& name);
};

#endif // XAPIAN_INCLUDED_BACKENDMANAGER_SINGLEFILE_H
