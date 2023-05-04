/** @file
 * @brief BackendManager subclass for chert databases.
 */
/* Copyright (C) 2007,2008,2009,2013,2018 Olly Betts
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

#include "filetests.h"
#include "unixcmds.h"

using namespace std;

#define CACHE_DIRECTORY ".chert"

// Use minimum block size to try to tickle more bugs.
#define BLOCK_SIZE 2048

BackendManagerChert::BackendManagerChert(const string& datadir_)
    : BackendManager(datadir_, "chert")
{
    // Ensure the directory we store cached test databases in exists.
    (void)create_dir_if_needed(CACHE_DIRECTORY);
}

string
BackendManagerChert::do_get_database_path(const vector<string> & files)
{
    string db_path = CACHE_DIRECTORY "/db";
    for (const string& file : files) {
	db_path += "__";
	db_path += file;
    }

    if (!dir_exists(db_path)) {
	// No cached DB exists.  Create at a temporary path and rename
	// so we don't leave a partial DB in place upon failure.
	string tmp_path = db_path + ".tmp";
	// Make sure there's nothing existing at our temporary path.
	rm_rf(tmp_path);
	auto flags = Xapian::DB_CREATE|Xapian::DB_BACKEND_CHERT;
	Xapian::WritableDatabase wdb(tmp_path, flags, BLOCK_SIZE);
	index_files_to_database(wdb, files);
	wdb.close();
	if (rename(tmp_path.c_str(), db_path.c_str()) < 0) {
	    throw Xapian::DatabaseError("rename failed", errno);
	}
    }

    return db_path;
}

Xapian::WritableDatabase
BackendManagerChert::get_writable_database(const string & name,
					   const string & file)
{
    last_wdb_name = name;
    string db_path = CACHE_DIRECTORY "/" + name;

    // We can't use a cached version, as it may have been modified by the
    // testcase.
    rm_rf(db_path);

    auto flags = Xapian::DB_CREATE|Xapian::DB_BACKEND_CHERT;
    Xapian::WritableDatabase wdb(db_path, flags, BLOCK_SIZE);
    index_files_to_database(wdb, vector<string>(1, file));

    return wdb;
}

string
BackendManagerChert::get_writable_database_path(const string & name)
{
    return CACHE_DIRECTORY "/" + name;
}

string
BackendManagerChert::get_compaction_output_path(const string& name)
{
    return CACHE_DIRECTORY "/" + name;
}

string
BackendManagerChert::get_generated_database_path(const std::string & name)
{
    return BackendManagerChert::get_writable_database_path(name);
}

Xapian::WritableDatabase
BackendManagerChert::get_writable_database_again()
{
    return Xapian::WritableDatabase(CACHE_DIRECTORY "/" + last_wdb_name,
				    Xapian::DB_OPEN|Xapian::DB_BACKEND_CHERT);
}

string
BackendManagerChert::get_writable_database_path_again()
{
    return CACHE_DIRECTORY "/" + last_wdb_name;
}
