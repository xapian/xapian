/** @file backendmanager_inmemory.cc
 * @brief BackendManager subclass for inmemory databases.
 */
/* Copyright (C) 2007,2009 Olly Betts
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

using namespace std;

std::string
BackendManagerInMemory::get_dbtype() const
{
    return "inmemory";
}

Xapian::Database
BackendManagerInMemory::do_get_database(const vector<string> & files)
{
    return getwritedb_inmemory(files);
}

Xapian::WritableDatabase
BackendManagerInMemory::get_writable_database(const string &,
					      const string & file)
{
    return getwritedb_inmemory(vector<string>(1, file));
}
