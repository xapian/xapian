/** @file
 * @brief BackendManager subclass for honey databases.
 */
/* Copyright (C) 2007,2008,2009,2017 Olly Betts
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

#ifndef XAPIAN_INCLUDED_BACKENDMANAGER_HONEY_H
#define XAPIAN_INCLUDED_BACKENDMANAGER_HONEY_H

#include "backendmanager.h"

#include <string>

#include <xapian/database.h>
#include <xapian/types.h>

/// BackendManager subclass for honey databases.
class BackendManagerHoney : public BackendManager {
    /// Don't allow assignment.
    void operator=(const BackendManagerHoney &);

    /// Don't allow copying.
    BackendManagerHoney(const BackendManagerHoney &);

    /// The path of the last writable database used.
    std::string last_wdb_name;

    /// BackendManager for generated testcases
    BackendManager* generated_sub_manager;

  protected:
    /// Get the path of Honey Xapian::Database instance.
    std::string do_get_database_path(const std::vector<std::string> & files);

  public:
    BackendManagerHoney(const std::string& datadir_)
	: BackendManager(datadir_),
	  generated_sub_manager(NULL) { }

    BackendManagerHoney(const std::string& datadir_,
			BackendManager* generated_sub_manager_)
	: BackendManager(datadir_),
	  generated_sub_manager(generated_sub_manager_) { }

    /// Return a string representing the current database type.
    std::string get_dbtype() const;

    /// Get generated database for honey
    Xapian::WritableDatabase get_generated_database(const std::string& name);

    /// Finalise generated database
    void finalise_generated_database(const std::string& name);

    /// Create a Xapian::WritableDatabase object.
    Xapian::WritableDatabase get_writable_database(const std::string& name,
						   const std::string& file);

    std::string get_generated_database_path(const std::string& name);

    std::string get_compaction_output_path(const std::string& name);
};

#endif // XAPIAN_INCLUDED_BACKENDMANAGER_HONEY_H
