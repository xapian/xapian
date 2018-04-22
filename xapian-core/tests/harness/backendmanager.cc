/* backendmanager.cc: manage backends for testsuite
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2011,2012,2013,2016,2017,2018 Olly Betts
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

#include "filetests.h"
#include "index_utils.h"
#include "backendmanager.h"
#include "unixcmds.h"

using namespace std;

[[noreturn]]
static void
invalid_operation(const char* msg)
{
    throw Xapian::InvalidOperationError(msg);
}

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

BackendManager::~BackendManager() { }

std::string
BackendManager::get_dbtype() const
{
    return "none";
}

string
BackendManager::do_get_database_path(const vector<string> &)
{
    invalid_operation("Path isn't meaningful for this database type");
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

Xapian::Database
BackendManager::get_database(const std::string &dbname,
			     void (*gen)(Xapian::WritableDatabase&,
					 const std::string &),
			     const std::string &arg)
{
    string dbleaf = "db__";
    dbleaf += dbname;
    const string& path = get_generated_database_path(dbleaf);
    if (path.empty()) {
	// InMemory doesn't have a path but we want to support generated
	// databases for it.
	Xapian::WritableDatabase wdb = get_writable_database(path, path);
	gen(wdb, arg);
	return wdb;
    }

    if (path_exists(path)) {
	try {
	    return Xapian::Database(path);
	} catch (const Xapian::DatabaseOpeningError &) {
	}
    }
    rm_rf(path);

    string tmp_dbleaf(dbleaf);
    tmp_dbleaf += '~';
    string tmp_path(path);
    tmp_path += '~';

    {
	Xapian::WritableDatabase wdb = get_writable_database(tmp_dbleaf,
							     string());
	gen(wdb, arg);
    }
    rename(tmp_path.c_str(), path.c_str());
    // For multi, the shards will use the temporary name, but that's not really
    // a problem.

    return Xapian::Database(path);
}

std::string
BackendManager::get_database_path(const std::string &dbname,
				  void (*gen)(Xapian::WritableDatabase&,
					      const std::string &),
				  const std::string &arg)
{
    string dbleaf = "db__";
    dbleaf += dbname;
    const string & path = get_generated_database_path(dbleaf);
    if (path_exists(path)) {
	try {
	    (void)Xapian::Database(path);
	    return path;
	} catch (const Xapian::DatabaseOpeningError &) {
	}
    }
    rm_rf(path);

    string tmp_dbleaf(dbleaf);
    tmp_dbleaf += '~';
    string tmp_path(path);
    tmp_path += '~';

    {
	Xapian::WritableDatabase wdb = get_writable_database(tmp_dbleaf,
							     string());
	gen(wdb, arg);
    }
    rename(tmp_path.c_str(), path.c_str());

    return path;
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

Xapian::WritableDatabase
BackendManager::get_writable_database(const string &, const string &)
{
    invalid_operation("Attempted to open a disabled database");
}

string
BackendManager::get_writable_database_path(const std::string &)
{
    invalid_operation("Path isn't meaningful for this database type");
}

string
BackendManager::get_compaction_output_path(const std::string&)
{
    invalid_operation("Compaction not supported for this database type");
}

string
BackendManager::get_generated_database_path(const std::string &)
{
    invalid_operation("Generated databases aren't supported for this database "
		      "type");
}

Xapian::Database
BackendManager::get_remote_database(const vector<string> &, unsigned int)
{
    string msg = "BackendManager::get_remote_database() called for non-remote "
		 "database (type is ";
    msg += get_dbtype();
    msg += ')';
    throw Xapian::InvalidOperationError(msg);
}

Xapian::Database
BackendManager::get_writable_database_as_database()
{
    return Xapian::Database(get_writable_database_path_again());
}

Xapian::WritableDatabase
BackendManager::get_writable_database_again()
{
    string msg = "Backend ";
    msg += get_dbtype();
    msg += " doesn't support get_writable_database_again()";
    throw Xapian::InvalidOperationError(msg);
}

string
BackendManager::get_writable_database_path_again()
{
    string msg = "Backend ";
    msg += get_dbtype();
    msg += " doesn't support get_writable_database_path_again()";
    throw Xapian::InvalidOperationError(msg);
}

void
BackendManager::clean_up()
{
}

void
BackendManager::kill_server()
{
    invalid_operation("Server kill operation not supported for non-remote"
		      "database type");
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
