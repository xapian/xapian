/** @file backendmanager_multi.cc
 * @brief BackendManager subclass for multi databases.
 */
/* Copyright (C) 2007,2008,2009,2011,2012,2013,2015 Olly Betts
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

#include <config.h>

#include "backendmanager_multi.h"

#include "filetests.h"
#include "index_utils.h"
#include "str.h"

#include <cstdio> // For rename().
#include <cstring>
#include "safeerrno.h"

using namespace std;

BackendManagerMulti::BackendManagerMulti(const std::string & subtype_)
	: subtype(subtype_)
{
#ifdef XAPIAN_HAS_GLASS_BACKEND
    if (subtype == "glass") return;
#endif
#ifdef XAPIAN_HAS_CHERT_BACKEND
    if (subtype == "chert") return;
#endif
    throw ("Unknown backend type \"" + subtype + "\" specified for multi database subdatabases");
}

std::string
BackendManagerMulti::get_dbtype() const
{
    return "multi_" + subtype;
}

#define NUMBER_OF_SUB_DBS 2

string
BackendManagerMulti::createdb_multi(const vector<string> & files)
{
    string dbdir = ".multi" + subtype;
    create_dir_if_needed(dbdir);

    string dbname = "db";
    vector<string>::const_iterator i;
    for (i = files.begin(); i != files.end(); ++i) {
	dbname += "__";
	dbname += *i;
    }
    string dbpath = dbdir + "/" + dbname;

    if (file_exists(dbpath)) return dbpath;

    string tmpfile = dbpath;
    tmpfile += ".tmp";
    ofstream out(tmpfile.c_str());
    if (!out.is_open()) {
	string msg = "Couldn't create file '";
	msg += tmpfile;
	msg += "' (";
	msg += strerror(errno);
	msg += ')';
	throw msg;
    }

    // Open NUMBER_OF_SUB_DBS databases and index files to them alternately so
    // a multi-db combining them contains the documents in the expected order.
    Xapian::WritableDatabase dbs;
    int flags = Xapian::DB_CREATE_OR_OVERWRITE;
    if (subtype == "glass") {
	flags |= Xapian::DB_BACKEND_GLASS;
    } else if (subtype == "chert") {
	flags |= Xapian::DB_BACKEND_CHERT;
    } else {
	string msg = "Unknown multidb subtype: ";
	msg += subtype;
	throw msg;
    }
    string dbbase = dbdir;
    dbbase += '/';
    dbbase += dbname;
    dbbase += "___";
    size_t dbbase_len = dbbase.size();
    string line = subtype;
    line += ' ';
    line += dbname;
    line += "___";
    for (size_t n = 0; n < NUMBER_OF_SUB_DBS; ++n) {
	dbbase += str(n);
	dbs.add_database(Xapian::WritableDatabase(dbbase, flags));
	dbbase.resize(dbbase_len);
	out << line << n << '\n';
    }
    out.close();

    FileIndexer(get_datadir(), files).index_to(dbs);

    rename(tmpfile.c_str(), dbpath.c_str());

    return dbpath;
}

string
BackendManagerMulti::do_get_database_path(const vector<string> & files)
{
    return createdb_multi(files);
}

Xapian::WritableDatabase
BackendManagerMulti::get_writable_database(const string &, const string &)
{
    throw Xapian::UnimplementedError("Multi-databases don't support writing");
}
