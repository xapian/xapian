/** @file
 * @brief manage backends for testsuite
 */
/* Copyright 1999,2000,2001 BrightStation PLC
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

#include <cerrno>
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
    if (mkdir(dirname.c_str(), 0700) == 0) {
	return true;
    }

    int mkdir_errno = errno;
    if (mkdir_errno == EEXIST) {
	// Something exists at dirname, but we need to check if it is a directory.
	if (dir_exists(dirname)) {
	    return false;
	}
    }

    throw Xapian::DatabaseOpeningError("Failed to create directory",
				       mkdir_errno);
}

BackendManager::~BackendManager() { }

string
BackendManager::do_get_database_path(const vector<string> &)
{
    throw Xapian::InvalidArgumentError("Path isn't meaningful for this database type");
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
	// This cast avoids a -Wreturn-std-move warning from older clang (seen
	// with clang 8 and 11; not seen with clang 13).  We can't address this
	// by adding the suggested std::move() because GCC 13 -Wredundant-move
	// then warns that the std::move() is redundant!
	return static_cast<Xapian::Database>(wdb);
    }

    if (path_exists(path)) {
	try {
	    return get_database_by_path(path);
	} catch (const Xapian::DatabaseOpeningError &) {
	}
    }
    rm_rf(path);

    string tmp_dbleaf(dbleaf);
    tmp_dbleaf += '~';
    string tmp_path(path);
    tmp_path += '~';

    {
	Xapian::WritableDatabase wdb = get_generated_database(tmp_dbleaf);
	gen(wdb, arg);
    }
    finalise_generated_database(tmp_dbleaf);
    rename(tmp_path.c_str(), path.c_str());
    // For multi, the shards will use the temporary name, but that's not really
    // a problem.

    return get_database_by_path(path);
}

std::string
BackendManager::get_database_path(const std::string &dbname,
				  void (*gen)(Xapian::WritableDatabase&,
					      const std::string &),
				  const std::string &arg)
{
    string dbleaf = "db__";
    dbleaf += dbname;
    const string& path = get_generated_database_path(dbleaf);
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
	Xapian::WritableDatabase wdb = get_generated_database(tmp_dbleaf);
	gen(wdb, arg);
    }
    finalise_generated_database(tmp_dbleaf);
    rename(tmp_path.c_str(), path.c_str());

    return path;
}

Xapian::Database
BackendManager::get_database_by_path(const string& path)
{
    return Xapian::Database(path);
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
    throw Xapian::InvalidArgumentError("Attempted to open a disabled database");
}

Xapian::WritableDatabase
BackendManager::get_remote_writable_database(string)
{
    string msg = "BackendManager::get_remote_writable_database() "
		 "called for non-remote database (type is ";
    msg += get_dbtype();
    msg += ')';
    throw Xapian::InvalidOperationError(msg);
}

string
BackendManager::get_writable_database_path(const std::string &)
{
    throw Xapian::InvalidArgumentError("Path isn't meaningful for this database type");
}

Xapian::WritableDatabase
BackendManager::get_generated_database(const std::string& name)
{
    return get_writable_database(name, string());
}

string
BackendManager::get_compaction_output_path(const std::string&)
{
    throw Xapian::InvalidArgumentError("Compaction now supported for this database type");
}

string
BackendManager::get_generated_database_path(const std::string &)
{
    throw Xapian::InvalidArgumentError("Generated databases aren't supported for this database type");
}

void
BackendManager::finalise_generated_database(const std::string&)
{ }

Xapian::Database
BackendManager::get_remote_database(const vector<string> &, unsigned int)
{
    string msg = "BackendManager::get_remote_database() called for non-remote database (type is ";
    msg += get_dbtype();
    msg += ')';
    throw Xapian::InvalidOperationError(msg);
}

string
BackendManager::get_writable_database_args(const std::string&,
					   unsigned int)
{
    string msg = "BackendManager::get_writable_database_args() "
		 "called for non-remote database (type is ";
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
