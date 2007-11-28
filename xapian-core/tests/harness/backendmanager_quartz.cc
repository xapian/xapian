/** @file backendmanager_quartz.cc
 * @brief BackendManager subclass for quartz databases.
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

#include <config.h>

// We have to use the deprecated Quartz::open() method.
#define XAPIAN_DEPRECATED(D) D
#include <xapian.h>

#include "backendmanager_quartz.h"

using namespace std;

BackendManagerQuartz::~BackendManagerQuartz() { }

const char *
BackendManagerQuartz::get_dbtype() const
{
    return "quartz";
}

Xapian::Database
BackendManagerQuartz::get_database(const vector<string> & files)
{
    return Xapian::Quartz::open(createdb_quartz(files));
}

Xapian::Database
BackendManagerQuartz::get_database(const string & file)
{
    return Xapian::Quartz::open(createdb_quartz(vector<string>(1, file)));
}

Xapian::WritableDatabase
BackendManagerQuartz::get_writable_database(const string & name,
					    const string & file)
{
    last_wdb_name = name;
    return getwritedb_quartz(name, vector<string>(1, file));
}

Xapian::Database
BackendManagerQuartz::get_writable_database_as_database()
{
    return Xapian::Quartz::open(".quartz/" + last_wdb_name);
}

Xapian::WritableDatabase
BackendManagerQuartz::get_writable_database_again()
{
    return Xapian::Quartz::open(".quartz/" + last_wdb_name, Xapian::DB_OPEN);
}
