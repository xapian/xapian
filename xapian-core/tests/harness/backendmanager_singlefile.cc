/** @file backendmanager_singlefile.cc
 * @brief BackendManager subclass for singlefile databases.
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

#include "backendmanager_singlefile.h"

#include "filetests.h"
#include "index_utils.h"
#include "unixcmds.h"

#include <cstdio> // For rename().
#include <cstring>
#include "safeerrno.h"

using namespace std;

BackendManagerSingleFile::BackendManagerSingleFile(const std::string & subtype_)
	: subtype(subtype_)
{
#ifdef XAPIAN_HAS_GLASS_BACKEND
    if (subtype == "glass") return;
#endif
    throw ("Unknown backend type \"" + subtype + "\" specified for singlefile database subdatabases");
}

std::string
BackendManagerSingleFile::get_dbtype() const
{
    return "singlefile_" + subtype;
}

#define NUMBER_OF_SUB_DBS 2

string
BackendManagerSingleFile::createdb_singlefile(const vector<string> & files)
{
    string dbdir = ".singlefile" + subtype;
    create_dir_if_needed(dbdir);

    string dbname = "db";
    vector<string>::const_iterator i;
    for (i = files.begin(); i != files.end(); ++i) {
	dbname += "__";
	dbname += *i;
    }
    string dbpath = dbdir + "/" + dbname;

    if (file_exists(dbpath)) return dbpath;

    string db_source = dbpath + ".src";
    int flags = Xapian::DB_CREATE_OR_OVERWRITE;
    if (subtype == "glass") {
	flags |= Xapian::DB_BACKEND_GLASS;
    } else {
	string msg = "Unknown singlefiledb subtype: ";
	msg += subtype;
	throw msg;
    }

    string tmpfile = dbpath;
    tmpfile += ".tmp";

    Xapian::WritableDatabase db(db_source, flags);
    FileIndexer(get_datadir(), files).index_to(db);
    db.commit();
    db.compact(tmpfile, Xapian::DBCOMPACT_SINGLE_FILE);
    db.close();

    rm_rf(db_source);

    rename(tmpfile.c_str(), dbpath.c_str());

    return dbpath;
}

string
BackendManagerSingleFile::do_get_database_path(const vector<string> & files)
{
    return createdb_singlefile(files);
}

Xapian::WritableDatabase
BackendManagerSingleFile::get_writable_database(const string &, const string &)
{
    throw Xapian::UnimplementedError("Single-file databases don't support writing");
}
