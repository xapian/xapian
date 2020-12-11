/** @file
 * @brief BackendManager subclass for inmemory databases.
 */
/* Copyright (C) 2007,2009,2018 Olly Betts
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

#include <config.h>

#include "backendmanager_inmemory.h"

#ifdef XAPIAN_HAS_INMEMORY_BACKEND

using namespace std;

std::string
BackendManagerInMemory::get_dbtype() const
{
    return "inmemory";
}

Xapian::Database
BackendManagerInMemory::do_get_database(const vector<string>& files)
{
    Xapian::WritableDatabase wdb(string(), Xapian::DB_BACKEND_INMEMORY);
    index_files_to_database(wdb, files);
    return std::move(wdb);
}

Xapian::WritableDatabase
BackendManagerInMemory::get_writable_database(const string&,
					      const string& file)
{
    Xapian::WritableDatabase wdb(string(), Xapian::DB_BACKEND_INMEMORY);
    index_files_to_database(wdb, vector<string>(1, file));
    return wdb;
}

string
BackendManagerInMemory::get_generated_database_path(const std::string&)
{
    return string();
}

#endif // XAPIAN_HAS_INMEMORY_BACKEND
