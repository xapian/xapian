/* backendmanager.cc: manage backends for testsuite
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2011,2012,2013,2016 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include <xapian.h>

#ifdef HAVE_VALGRIND
# include <valgrind/memcheck.h>
#endif

#include "safeerrno.h"

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include <sys/types.h>
#include "safesysstat.h"

#include "index_utils.h"
#include "backendmanager.h"
#include "unixcmds.h"

using namespace std;

void
BackendManager::index_files_to_database(Xapian::WritableDatabase & database,
	    const vector<string> & files)
{
    FileIndexer(datadir, files).index_to(database);
}

/** Create the directory dirname if needed.  Returns true if the
 *  directory was created and false if it was already there.  Throws
 *  an exception if there was an error (eg not a directory).
 */
bool
BackendManager::create_dir_if_needed(const string &dirname)
{
    // create a directory if not present
    struct stat sbuf;
    int result = stat(dirname.c_str(), &sbuf);
    if (result < 0) {
    if (errno != ENOENT)
	throw Xapian::DatabaseOpeningError("Can't stat directory");
    if (mkdir(dirname.c_str(), 0700) < 0)
	throw Xapian::DatabaseOpeningError("Can't create directory");
	return true; // Successfully created a directory.
    }
    if (!S_ISDIR(sbuf.st_mode))
	throw Xapian::DatabaseOpeningError("Is not a directory.");
    return false; // Already a directory.
}

string
BackendManager::createdb(const vector<string> &files)
{
    string parent_dir = ".letor-db";
    create_dir_if_needed(parent_dir);

    string dbdir = parent_dir + "/db";
    for (vector<string>::const_iterator i = files.begin();
	 i != files.end(); ++i) {
	dbdir += '_';
	dbdir += *i;
    }
    // If the database is readonly, we can reuse it if it exists.
    if (create_dir_if_needed(dbdir)) {
    // Directory was created, so do the indexing.
	Xapian::WritableDatabase db(dbdir, Xapian::DB_CREATE);
	index_files_to_database(db, files);
	db.commit();
    }
    return dbdir;
}

BackendManager::~BackendManager() { }

std::string
BackendManager::get_dbtype() const
{
    return "default";
}

string
BackendManager::do_get_database_path(const vector<string> & files)
{
    return createdb(files);
}

Xapian::Database
BackendManager::do_get_database(const vector<string> & files)
{
    return Xapian::Database(do_get_database_path(files));
}

Xapian::Database
BackendManager::get_database(const vector<string> & files)
{
    return do_get_database(files);
}

Xapian::Database
BackendManager::get_database(const string & file)
{
    return do_get_database(vector<string>(1, file));
}

string
BackendManager::get_database_path(const vector<string> & files)
{
    return do_get_database_path(files);
}

string
BackendManager::get_database_path(const string & file)
{
    return do_get_database_path(vector<string>(1, file));
}

void
BackendManager::clean_up()
{
}

const char *
BackendManager::get_xapian_progsrv_command()
{
#ifdef HAVE_VALGRIND
    if (RUNNING_ON_VALGRIND) {
    return "./runsrv " XAPIAN_PROGSRV;
    }
#endif
    return XAPIAN_PROGSRV;
}
