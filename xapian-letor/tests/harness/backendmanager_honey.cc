/** @file
 * @brief BackendManager subclass for honey databases
 */
/* Copyright (C) 2007,2008,2009,2013,2017,2018 Olly Betts
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

#include "backendmanager_honey.h"

#ifdef XAPIAN_HAS_HONEY_BACKEND

#include "filetests.h"
#include "index_utils.h"
#include "unixcmds.h"

#include <cstdio> // For rename().

using namespace std;

#define CACHE_DIRECTORY ".honey"

string
BackendManagerHoney::get_dbtype() const
{
    return "honey";
}

string
BackendManagerHoney::do_get_database_path(const vector<string> & files)
{
    string dbdir = CACHE_DIRECTORY;
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
    int flags = Xapian::DB_CREATE_OR_OVERWRITE | Xapian::DB_BACKEND_GLASS;

    string tmpfile = dbpath;
    tmpfile += ".tmp";

    Xapian::WritableDatabase db(db_source, flags);
    FileIndexer(get_datadir(), files).index_to(db);
    db.commit();
    db.compact(tmpfile,
	       Xapian::DB_BACKEND_HONEY | Xapian::DBCOMPACT_NO_RENUMBER);
    db.close();

    rm_rf(db_source);

    rename(tmpfile.c_str(), dbpath.c_str());

    return dbpath;
}

Xapian::WritableDatabase
BackendManagerHoney::get_generated_database(const string& name)
{
    // Create generated database inside glass cache
    // to prevent a valid glass db inside honey cache
    // if testsuite was interrupted.
    return generated_sub_manager->get_generated_database(name);
}

void
BackendManagerHoney::finalise_generated_database(const string& name)
{
    create_dir_if_needed(CACHE_DIRECTORY);

    // path to the temporary generated db
    string generated_db_path =
	generated_sub_manager->get_generated_database_path(name);

    // path to honey tmpfile
    string tmpfile = CACHE_DIRECTORY "/" + name;
    tmpfile += ".tmp";

    // path to final honey db
    string path = CACHE_DIRECTORY "/" + name;

    // Convert a glass backend to honey.
    Xapian::WritableDatabase wdb(generated_db_path);
    wdb.compact(tmpfile,
		Xapian::DB_BACKEND_HONEY | Xapian::DBCOMPACT_NO_RENUMBER);
    wdb.close();

    rename(tmpfile.c_str(), path.c_str());
}

Xapian::WritableDatabase
BackendManagerHoney::get_writable_database(const string &, const string &)
{
    throw Xapian::UnimplementedError("Honey databases don't support writing");
}

string
BackendManagerHoney::get_generated_database_path(const string& name)
{
    return CACHE_DIRECTORY "/" + name;
}

string
BackendManagerHoney::get_compaction_output_path(const string& name)
{
    return CACHE_DIRECTORY "/" + name;
}

#endif
