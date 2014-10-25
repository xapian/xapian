/** @file backendmanager_chert.cc
 * @brief BackendManager subclass for chert databases.
 */
/* Copyright (C) 2007,2008,2009,2013 Olly Betts
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

#include "backendmanager_chert.h"

using namespace std;

std::string
BackendManagerChert::get_dbtype() const
{
    return "chert";
}

string
BackendManagerChert::do_get_database_path(const vector<string> & files)
{
    return createdb_chert(files);
}

Xapian::WritableDatabase
BackendManagerChert::get_writable_database(const string & name,
					   const string & file)
{
    last_wdb_name = name;
    return getwritedb_chert(name, vector<string>(1, file));
}

string
BackendManagerChert::get_writable_database_path(const string & name)
{
    return getwritedb_chert_path(name);
}

Xapian::Database
BackendManagerChert::get_writable_database_as_database()
{
    return Xapian::Database(".chert/" + last_wdb_name,
			    Xapian::DB_BACKEND_CHERT);
}

Xapian::WritableDatabase
BackendManagerChert::get_writable_database_again()
{
    return Xapian::WritableDatabase(".chert/" + last_wdb_name,
				    Xapian::DB_OPEN|Xapian::DB_BACKEND_CHERT);
}
