/** @file backendmanager_brass.cc
 * @brief BackendManager subclass for brass databases.
 */
/* Copyright (C) 2007,2008,2009 Olly Betts
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

#include "backendmanager_brass.h"

using namespace std;

std::string
BackendManagerBrass::get_dbtype() const
{
    return "brass";
}

string
BackendManagerBrass::do_get_database_path(const vector<string> & files)
{
    return createdb_brass(files);
}

Xapian::WritableDatabase
BackendManagerBrass::get_writable_database(const string & name,
					   const string & file)
{
    last_wdb_name = name;
    return getwritedb_brass(name, vector<string>(1, file));
}

string
BackendManagerBrass::get_writable_database_path(const string & name)
{
    return getwritedb_brass_path(name);
}

Xapian::Database
BackendManagerBrass::get_writable_database_as_database()
{
    return Xapian::Brass::open(".brass/" + last_wdb_name);
}

Xapian::WritableDatabase
BackendManagerBrass::get_writable_database_again()
{
    return Xapian::Brass::open(".brass/" + last_wdb_name, Xapian::DB_OPEN);
}
