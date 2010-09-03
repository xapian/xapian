/** @file backendmanager_flint.cc
 * @brief BackendManager subclass for flint databases.
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

#include "backendmanager_flint.h"

using namespace std;

std::string
BackendManagerFlint::get_dbtype() const
{
    return "flint";
}

string
BackendManagerFlint::do_get_database_path(const vector<string> & files)
{
    return createdb_flint(files);
}

Xapian::WritableDatabase
BackendManagerFlint::get_writable_database(const string & name,
					   const string & file)
{
    last_wdb_name = name;
    return getwritedb_flint(name, vector<string>(1, file));
}

string
BackendManagerFlint::get_writable_database_path(const string & name)
{
    return getwritedb_flint_path(name);
}

Xapian::Database
BackendManagerFlint::get_writable_database_as_database()
{
    return Xapian::Flint::open(".flint/" + last_wdb_name);
}

Xapian::WritableDatabase
BackendManagerFlint::get_writable_database_again()
{
    return Xapian::Flint::open(".flint/" + last_wdb_name, Xapian::DB_OPEN);
}
