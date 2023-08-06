/** @file
 * @brief BackendManager subclass for multi databases.
 */
/* Copyright (C) 2007,2008,2009,2011,2012,2013,2015,2017,2018,2019,2020 Olly Betts
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

#include "errno_to_string.h"
#include "filetests.h"
#include "index_utils.h"
#include "str.h"

#include "safeunistd.h"

#include <cerrno>
#include <cstdio> // For rename().
#include <cstring>

#ifdef __WIN32__
# include <stdlib.h>
# include <winerror.h>
#endif

using namespace std;

static std::string
build_dbtype(const vector<BackendManager*>& sub_managers)
{
    string dbtype = "multi";
    if (sub_managers.size() == 2 &&
	sub_managers[0]->get_dbtype() == sub_managers[1]->get_dbtype()) {
	dbtype += "_" + sub_managers[0]->get_dbtype();
    } else {
	for (auto sub_manager : sub_managers) {
	    dbtype += "_" + sub_manager->get_dbtype();
	}
    }
    return dbtype;
}

BackendManagerMulti::BackendManagerMulti(const std::string& datadir_,
					 const vector<BackendManager*>& sub_managers_)
    : BackendManager(datadir_, build_dbtype(sub_managers_)),
      sub_managers(sub_managers_)
{
    cachedir = ".multi";
    if (sub_managers.size() == 2 &&
	sub_managers[0]->get_dbtype() == sub_managers[1]->get_dbtype()) {
	cachedir += sub_managers[0]->get_dbtype();
    } else {
	for (auto sub_manager : sub_managers) {
	    cachedir += sub_manager->get_dbtype();
	}
    }
    // Ensure the directory we store cached test databases in exists.
    (void)create_dir_if_needed(cachedir);
}

#define NUMBER_OF_SUB_DBS 2

string
BackendManagerMulti::createdb_multi(const string& name,
				    const vector<string>& files)
{
    string dbname;
    if (!name.empty()) {
	dbname = name;
    } else {
	dbname = "db";
	for (const string& file : files) {
	    dbname += "__";
	    dbname += file;
	}
    }

    string db_path = cachedir;
    db_path += '/';
    db_path += dbname;

    if (!name.empty()) {
#ifdef __WIN32__
retry_unlink:
#endif
	if (unlink(db_path.c_str()) < 0 && errno != ENOENT) {
#ifdef __WIN32__
	    if (errno == EACCES && _doserrno == ERROR_SHARING_VIOLATION) {
		/* This happens when running multiremoteprog tests under
		 * Wine with a cross-build from Linux to mingw64 x86-64.
		 *
		 * FIXME: Work out what is going on...
		 */
		sleep(1);
		goto retry_unlink;
	    }
#endif
	    string msg = "Couldn't unlink file '";
	    msg += db_path;
	    msg += "' (";
	    errno_to_string(errno, msg);
	    msg += ')';
	    throw msg;
	}
    } else {
	// Use cached database if there is one.
	if (file_exists(db_path)) return db_path;
    }

    string tmpfile = db_path + ".tmp";
    ofstream out(tmpfile.c_str());
    if (!out.is_open()) {
	string msg = "Couldn't create file '";
	msg += tmpfile;
	msg += "' (";
	errno_to_string(errno, msg);
	msg += ')';
	throw msg;
    }

    // Open NUMBER_OF_SUB_DBS databases and index files to them alternately so
    // a multi-db combining them contains the documents in the expected order.
    Xapian::WritableDatabase dbs;

    string dbbase = db_path;
    dbbase += "___";
    size_t dbbase_len = dbbase.size();

    for (size_t n = 0; n < NUMBER_OF_SUB_DBS; ++n) {
	const string& subtype = sub_managers[n]->get_dbtype();
	int flags = Xapian::DB_CREATE_OR_OVERWRITE;
	if (subtype == "glass") {
	    flags |= Xapian::DB_BACKEND_GLASS;
	    dbbase += str(n);
	    dbs.add_database(Xapian::WritableDatabase(dbbase, flags));
	    out << subtype << ' ' << dbname << "___" << n << '\n';
	} else if (subtype == "chert") {
	    flags |= Xapian::DB_BACKEND_CHERT;
	    dbbase += str(n);
	    dbs.add_database(Xapian::WritableDatabase(dbbase, flags));
	    out << subtype << ' ' << dbname << "___" << n << '\n';
	} else if (subtype == "remoteprog_glass") {
	    flags |= Xapian::DB_BACKEND_GLASS;
	    dbbase += str(n);
	    Xapian::WritableDatabase remote_db(dbbase, flags);
	    remote_db.close();
	    string args = sub_managers[n]->get_writable_database_args(dbbase,
								      300000);

	    dbs.add_database(
		sub_managers[n]->get_remote_writable_database(args));

	    out << "remote :" << BackendManager::get_xapian_progsrv_command()
		<< " " << args << '\n';
	} else {
	    string msg = "Unknown multidb subtype: ";
	    msg += subtype;
	    throw msg;
	}
	dbbase.resize(dbbase_len);
    }

    out.close();

    FileIndexer(get_datadir(), files).index_to(dbs);
    dbs.close();

#ifdef __WIN32__
retry_rename:
#endif
    if (rename(tmpfile.c_str(), db_path.c_str()) < 0) {
#ifdef __WIN32__
	if (errno == EACCES) {
	    if (_doserrno == ERROR_SHARING_VIOLATION) {
		// Sometimes we hit this failure case.  It happens with various
		// testcases, and with both mingw and MSVC.  We've seen it on
		// both appveyor and GHA.
		//
		// Debugging shows the destination file doesn't exist (and it
		// shouldn't).  The _doserrno code is ERROR_SHARING_VIOLATION
		// which suggests that tmpfile is still open, but a sleep+retry
		// makes it work.  Perhaps some AV is kicking in and opening
		// newly created files to inspect them or something?
		//
		// FIXME: It would be good to get to the bottom of this!
		sleep(1);
		goto retry_rename;
	    }
	}
#endif
	string msg = "rename failed (";
	errno_to_string(errno, msg);
	msg += ')';
	throw msg;
    }

    last_wdb_path = db_path;
    return db_path;
}

string
BackendManagerMulti::do_get_database_path(const vector<string> & files)
{
    return createdb_multi(string(), files);
}

Xapian::WritableDatabase
BackendManagerMulti::get_writable_database(const string& name, const string& file)
{
    vector<string> files;
    if (!file.empty()) files.push_back(file);
    return Xapian::WritableDatabase(createdb_multi(name, files));
}

string
BackendManagerMulti::get_writable_database_path(const std::string& name)
{
    return cachedir + "/" + name;
}

Xapian::Database
BackendManagerMulti::get_remote_database(const std::vector<std::string>& files,
					 unsigned int timeout)
{
    Xapian::Database db;
    size_t remotes = 0;
    for (auto sub_manager : sub_managers) {
	if (sub_manager->get_dbtype().find("remote") == string::npos) {
	    db.add_database(sub_manager->get_database(files));
	    continue;
	}

	++remotes;
	db.add_database(sub_manager->get_remote_database(files, timeout));
    }

    if (remotes == 0) {
	// It's useful to support mixed local/remote multi databases with a
	// custom timeout so we can test timeout and keepalive handling for
	// this case, but this method shouldn't be called on an all-local
	// multi database.
	const char* m = "BackendManager::get_remote_database() called for "
			"multi with no remote shards";
	throw Xapian::InvalidOperationError(m);
    }
    return db;
}

string
BackendManagerMulti::get_compaction_output_path(const string& name)
{
    return cachedir + "/" + name;
}

string
BackendManagerMulti::get_generated_database_path(const string& name)
{
    return BackendManagerMulti::get_writable_database_path(name);
}

Xapian::WritableDatabase
BackendManagerMulti::get_writable_database_again()
{
    return Xapian::WritableDatabase(last_wdb_path);
}

string
BackendManagerMulti::get_writable_database_path_again()
{
    return last_wdb_path;
}
