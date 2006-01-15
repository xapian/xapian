/* backendmanager.cc: manage backends for testsuite
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006 Olly Betts
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
#include <fstream>
#include <string>
#include <vector>

#include <sys/stat.h>
#include <cerrno>
#ifdef HAVE_SYS_ERRNO_H
# include <sys/errno.h>
#endif

#include <xapian.h>
#include "index_utils.h"
#include "backendmanager.h"
#include "omdebug.h"
#include "utils.h"

using namespace std;

static void
index_files_to_database(Xapian::WritableDatabase & database,
                        const vector<string> & paths)
{
    vector<string>::const_iterator p;
    for (p = paths.begin(); p != paths.end(); ++p) {
	const string & filename = *p;
	ifstream from(filename.c_str());
	if (!from)
	    throw Xapian::DatabaseOpeningError("Cannot open file " + filename +
		    " for indexing");

	while (from) {
	    database.add_document(document_from_stream(from));
	}
    }
}

#ifdef XAPIAN_BUILD_BACKEND_MUSCAT36
static void
index_files_to_m36(const string &prog, const string &dbdir,
		   const vector<string> & paths)
{
    string dump = dbdir + "/DATA";
    ofstream out(dump.c_str());
    string valuefile = dbdir + "/keyfile";
    ofstream values(valuefile.c_str());
    values << "omrocks!"; // magic word
    vector<string>::const_iterator p;
    for (p = paths.begin(); p != paths.end(); ++p) {
	const string & filename = *p;
	ifstream from(filename.c_str());
	if (!from)
	    throw Xapian::DatabaseOpeningError("Cannot open file " + filename +
		    " for indexing");

	while (from) {
	    Xapian::Document doc = document_from_stream(from);
	    out << "#RSTART#\n" << doc.get_data() << "\n#REND#\n#TSTART#\n";
	    {
		Xapian::TermIterator i = doc.termlist_begin();
		Xapian::TermIterator i_end = doc.termlist_end();
		for ( ; i != i_end; ++i) {
		    out << *i << endl;
		}
	    }
	    out << "#TEND#\n";
	    Xapian::ValueIterator value_i = doc.values_begin();
	    string value = string("\0\0\0\0\0\0\0", 8);
	    if (value_i != doc.values_end()) value = (*value_i) + value;
	    value = value.substr(0, 8);
	    values << value;
	}
    }
    out.close();
    string cmd = "../../makeda/" + prog + " -source " + dump +
	" -da " + dbdir + "/ -work " + dbdir + "/tmp- > /dev/null";
    system(cmd);
    unlink(dump);
}
#endif

void
BackendManager::set_dbtype(const string &type)
{
    if (type == current_type) {
	// leave it as it is.
    } else if (type == "inmemory") {
#ifdef XAPIAN_BUILD_BACKEND_INMEMORY
	do_getdb = &BackendManager::getdb_inmemory;
	do_getwritedb = &BackendManager::getwritedb_inmemory;
#else
	do_getdb = &BackendManager::getdb_void;
	do_getwritedb = &BackendManager::getwritedb_void;
#endif
#if 0
#ifdef XAPIAN_BUILD_BACKEND_INMEMORY
    } else if (type == "inmemoryerr") {
	do_getdb = &BackendManager::getdb_inmemoryerr;
	do_getwritedb = &BackendManager::getwritedb_inmemoryerr;
    } else if (type == "inmemoryerr2") {
	do_getdb = &BackendManager::getdb_inmemoryerr2;
	do_getwritedb = &BackendManager::getwritedb_inmemoryerr2;
    } else if (type == "inmemoryerr3") {
	do_getdb = &BackendManager::getdb_inmemoryerr3;
	do_getwritedb = &BackendManager::getwritedb_inmemoryerr3;
#else
    } else if (type == "inmemoryerr" || type == "inmemoryerr2" ||
	       type == "inmemoryerr3") {
	do_getdb = &BackendManager::getdb_void;
	do_getwritedb = &BackendManager::getwritedb_void;
#endif
#endif
    } else if (type == "flint") {
#ifdef XAPIAN_BUILD_BACKEND_FLINT
	do_getdb = &BackendManager::getdb_flint;
	do_getwritedb = &BackendManager::getwritedb_flint;
	rmdir(".flint");
#else
	do_getdb = &BackendManager::getdb_void;
	do_getwritedb = &BackendManager::getwritedb_void;
#endif
    } else if (type == "quartz") {
#ifdef XAPIAN_BUILD_BACKEND_QUARTZ
	do_getdb = &BackendManager::getdb_quartz;
	do_getwritedb = &BackendManager::getwritedb_quartz;
	rmdir(".quartz");
#else
	do_getdb = &BackendManager::getdb_void;
	do_getwritedb = &BackendManager::getwritedb_void;
#endif
    } else if (type == "remote") {
#ifdef XAPIAN_BUILD_BACKEND_REMOTE
	do_getdb = &BackendManager::getdb_remote;
	do_getwritedb = &BackendManager::getwritedb_remote;
#else
	do_getdb = &BackendManager::getdb_void;
	do_getwritedb = &BackendManager::getwritedb_void;
#endif
#ifdef XAPIAN_BUILD_BACKEND_MUSCAT36
    } else if (type == "da") {
	do_getdb = &BackendManager::getdb_da;
	do_getwritedb = &BackendManager::getwritedb_da;
	rmdir(".da");
    } else if (type == "db") {
	do_getdb = &BackendManager::getdb_db;
	do_getwritedb = &BackendManager::getwritedb_db;
	rmdir(".db");
    } else if (type == "daflimsy") {
	do_getdb = &BackendManager::getdb_daflimsy;
	do_getwritedb = &BackendManager::getwritedb_daflimsy;
	rmdir(".daflimsy");
    } else if (type == "dbflimsy") {
	do_getdb = &BackendManager::getdb_dbflimsy;
	do_getwritedb = &BackendManager::getwritedb_dbflimsy;
	rmdir(".dbflimsy");
#else
    } else if (type == "da" || type == "db" || type == "daflimsy" ||
	       type == "dbflimsy") {
	do_getdb = &BackendManager::getdb_void;
	do_getwritedb = &BackendManager::getwritedb_void;
#endif
    } else if (type == "void") {
	do_getdb = &BackendManager::getdb_void;
	do_getwritedb = &BackendManager::getwritedb_void;
    } else {
	throw Xapian::InvalidArgumentError(
		"Expected inmemory, flint, quartz, remote, da, db, "
		"daflimsy, dbflimsy, or void");
    }
    current_type = type;
}

void
BackendManager::set_datadir(const string &datadir_)
{
    datadir = datadir_;
}

string
BackendManager::get_datadir()
{
    return datadir;
}

Xapian::Database
BackendManager::getdb_void(const vector<string> &)
{
    throw Xapian::InvalidArgumentError("Attempted to open a disabled database");
}

Xapian::WritableDatabase
BackendManager::getwritedb_void(const vector<string> &)
{
    throw Xapian::InvalidArgumentError("Attempted to open a disabled database");
}

vector<string>
BackendManager::change_names_to_paths(const vector<string> &dbnames)
{
    vector<string> paths;
    vector<string>::const_iterator i;
    for (i = dbnames.begin(); i != dbnames.end(); ++i) {
	if (!i->empty()) {
	    if (datadir.empty()) {
		paths.push_back(*i);
	    } else {
		paths.push_back(datadir + "/" + *i + ".txt");
	    }
	}
    }
    return paths;
}

#ifdef XAPIAN_BUILD_BACKEND_INMEMORY
Xapian::Database
BackendManager::getdb_inmemory(const vector<string> &dbnames)
{
    return getwritedb_inmemory(dbnames);
}

Xapian::WritableDatabase
BackendManager::getwritedb_inmemory(const vector<string> &dbnames)
{
    Xapian::WritableDatabase db(Xapian::InMemory::open());
    index_files_to_database(db, change_names_to_paths(dbnames));
    return db;
}

#if 0
Xapian::Database
BackendManager::getdb_inmemoryerr(const vector<string> &dbnames)
{
    return getwritedb_inmemoryerr(dbnames);
}

Xapian::WritableDatabase
BackendManager::getwritedb_inmemoryerr(const vector<string> &dbnames)
{
    // FIXME: params.set("inmemory_errornext", 1);
    Xapian::WritableDatabase db(Xapian::InMemory::open());
    index_files_to_database(db, change_names_to_paths(dbnames));

    return db;
}

Xapian::Database
BackendManager::getdb_inmemoryerr2(const vector<string> &dbnames)
{
    return getwritedb_inmemoryerr2(dbnames);
}

Xapian::WritableDatabase
BackendManager::getwritedb_inmemoryerr2(const vector<string> &dbnames)
{
    // FIXME: params.set("inmemory_abortnext", 1);
    Xapian::WritableDatabase db(Xapian::InMemory::open());
    index_files_to_database(db, change_names_to_paths(dbnames));

    return db;
}

Xapian::Database
BackendManager::getdb_inmemoryerr3(const vector<string> &dbnames)
{
    return getwritedb_inmemoryerr3(dbnames);
}

Xapian::WritableDatabase
BackendManager::getwritedb_inmemoryerr3(const vector<string> &dbnames)
{
    // params.set("inmemory_abortnext", 2);
    Xapian::WritableDatabase db(Xapian::InMemory::open());
    index_files_to_database(db, change_names_to_paths(dbnames));

    return db;
}
#endif
#endif

/** Create the directory dirname if needed.  Returns true if the
 *  directory was created and false if it was already there.  Throws
 *  an exception if there was an error (eg not a directory).
 */
bool create_dir_if_needed(const string &dirname)
{
    // create a directory if not present
    struct stat sbuf;
    int result = stat(dirname, &sbuf);
    if (result < 0) {
	if (errno != ENOENT)
	    throw Xapian::DatabaseOpeningError("Can't stat directory");
        if (mkdir(dirname, 0700) < 0)
	    throw Xapian::DatabaseOpeningError("Can't create directory");
	return true; // Successfully created a directory.
    }
    if (!S_ISDIR(sbuf.st_mode))
	throw Xapian::DatabaseOpeningError("Is not a directory.");
    return false; // Already a directory.
}

#ifdef XAPIAN_BUILD_BACKEND_FLINT
Xapian::Database
BackendManager::getdb_flint(const vector<string> &dbnames)
{
    string parent_dir = ".flint";
    create_dir_if_needed(parent_dir);

    string dbdir = parent_dir + "/db";
    for (vector<string>::const_iterator i = dbnames.begin();
	 i != dbnames.end(); i++) {
	dbdir += "=" + *i;
    }
    // If the database is readonly, we can reuse it if it exists.
    if (create_dir_if_needed(dbdir)) {
	// Directory was created, so do the indexing.
	Xapian::WritableDatabase db(Xapian::Flint::open(dbdir, Xapian::DB_CREATE, 2048));
	index_files_to_database(db, change_names_to_paths(dbnames));
    }
    return Xapian::Flint::open(dbdir);
}

Xapian::WritableDatabase
BackendManager::getwritedb_flint(const vector<string> &dbnames)
{
    string parent_dir = ".flint";
    create_dir_if_needed(parent_dir);

    // Add 'w' to distinguish writable dbs (which need to be recreated on each
    // use) from readonly ones (which can be reused).
    string dbdir = parent_dir + "/dbw";
    for (vector<string>::const_iterator i = dbnames.begin();
	 i != dbnames.end(); ++i) {
	dbdir += "=" + *i;
    }
    // For a writable database we need to start afresh each time.
    rmdir(dbdir);
    (void)create_dir_if_needed(dbdir);
    touch(dbdir + "/log");
    // directory was created, so do the indexing.
    Xapian::WritableDatabase db(Xapian::Flint::open(dbdir, Xapian::DB_CREATE, 2048));
    index_files_to_database(db, change_names_to_paths(dbnames));
    return db;
}
#endif

#ifdef XAPIAN_BUILD_BACKEND_QUARTZ
Xapian::Database
BackendManager::getdb_quartz(const vector<string> &dbnames)
{
    string parent_dir = ".quartz";
    create_dir_if_needed(parent_dir);

    string dbdir = parent_dir + "/db";
    for (vector<string>::const_iterator i = dbnames.begin();
	 i != dbnames.end(); i++) {
	dbdir += "=" + *i;
    }
    // If the database is readonly, we can reuse it if it exists.
    if (create_dir_if_needed(dbdir)) {
	// Directory was created, so do the indexing.
	Xapian::WritableDatabase db(Xapian::Quartz::open(dbdir, Xapian::DB_CREATE, 2048));
	index_files_to_database(db, change_names_to_paths(dbnames));
    }
    return Xapian::Quartz::open(dbdir);
}

Xapian::WritableDatabase
BackendManager::getwritedb_quartz(const vector<string> &dbnames)
{
    string parent_dir = ".quartz";
    create_dir_if_needed(parent_dir);

    // Add 'w' to distinguish writable dbs (which need to be recreated on each
    // use) from readonly ones (which can be reused).
    string dbdir = parent_dir + "/dbw";
    for (vector<string>::const_iterator i = dbnames.begin();
	 i != dbnames.end(); ++i) {
	dbdir += "=" + *i;
    }
    // For a writable database we need to start afresh each time.
    rmdir(dbdir);
    (void)create_dir_if_needed(dbdir);
    touch(dbdir + "/log");
    // directory was created, so do the indexing.
    Xapian::WritableDatabase db(Xapian::Quartz::open(dbdir, Xapian::DB_CREATE, 2048));
    index_files_to_database(db, change_names_to_paths(dbnames));
    return db;
}
#endif

#ifdef XAPIAN_BUILD_BACKEND_REMOTE
Xapian::Database
BackendManager::getdb_remote(const vector<string> &dbnames)
{
    // run an xapian-progsrv for now.  Later we should also use xapian-tcpsrv
    string args = datadir;
    bool timeout = false;
    vector<string>::const_iterator i;
    for (i = dbnames.begin(); i != dbnames.end(); ++i) {
	if (*i == "#TIMEOUT#") {
	    ++i;
	    if (i == dbnames.end()) {
		throw Xapian::InvalidArgumentError("Missing timeout parameter");
	    }
	    args += " -t" + *i;
	    timeout = true;
	} else {
	    args += ' ';
	    args += *i;
	}
    }
    // Nice long timeout (5 minutes) so we don't timeout just because
    // the host is slow.
    if (!timeout) args += " -t300000";
    return Xapian::Remote::open("../bin/xapian-progsrv", args);
}

Xapian::WritableDatabase
BackendManager::getwritedb_remote(const vector<string> &/*dbnames*/)
{
    throw Xapian::InvalidArgumentError("Attempted to open writable remote database");
}
#endif

#ifdef XAPIAN_BUILD_BACKEND_MUSCAT36
Xapian::Database
BackendManager::getdb_da(const vector<string> &dbnames)
{
    string parent_dir = ".da";
    create_dir_if_needed(parent_dir);

    string dbdir = parent_dir + "/db";
    for (vector<string>::const_iterator i = dbnames.begin();
	 i != dbnames.end();
	 i++) {
	dbdir += "=" + *i;
    }
    if (create_dir_if_needed(dbdir)) {
	// directory was created, so do the indexing.
	// need to build temporary file and run it through makeda (yum)
	index_files_to_m36("makeDA", dbdir, change_names_to_paths(dbnames));
    }
    return Xapian::Muscat36::open_da(dbdir + "/R", dbdir + "/T",
				     dbdir + "/keyfile");
}

Xapian::Database
BackendManager::getdb_daflimsy(const vector<string> &dbnames)
{
    string parent_dir = ".daflimsy";
    create_dir_if_needed(parent_dir);

    string dbdir = parent_dir + "/db";
    for (vector<string>::const_iterator i = dbnames.begin();
	 i != dbnames.end();
	 i++) {
	dbdir += "=" + *i;
    }
    if (create_dir_if_needed(dbdir)) {
	// directory was created, so do the indexing.
	// need to build temporary file and run it through makeda (yum)
	index_files_to_m36("makeDAflimsy", dbdir, change_names_to_paths(dbnames));
    }
    return Xapian::Muscat36::open_da(dbdir + "/R", dbdir + "/T",
				     dbdir + "/keyfile", false);
}

Xapian::WritableDatabase
BackendManager::getwritedb_da(const vector<string> &/*dbnames*/)
{
    throw Xapian::InvalidArgumentError("Attempted to open writable da database");
}

Xapian::WritableDatabase
BackendManager::getwritedb_daflimsy(const vector<string> &/*dbnames*/)
{
    throw Xapian::InvalidArgumentError("Attempted to open writable daflimsy database");
}

Xapian::Database
BackendManager::getdb_db(const vector<string> &dbnames)
{
    string parent_dir = ".db";
    create_dir_if_needed(parent_dir);

    string dbdir = parent_dir + "/db";
    for (vector<string>::const_iterator i = dbnames.begin();
	 i != dbnames.end();
	 i++) {
	dbdir += "=" + *i;
    }
    if (create_dir_if_needed(dbdir)) {
	// directory was created, so do the indexing.
	// need to build temporary file and run it through makedb (yum)
	index_files_to_m36("makeDB", dbdir, change_names_to_paths(dbnames));
    }
    return Xapian::Muscat36::open_db(dbdir + "/DB", dbdir + "/keyfile");
}

Xapian::Database
BackendManager::getdb_dbflimsy(const vector<string> &dbnames)
{
    string parent_dir = ".dbflimsy";
    create_dir_if_needed(parent_dir);

    string dbdir = parent_dir + "/db";
    for (vector<string>::const_iterator i = dbnames.begin();
	 i != dbnames.end();
	 i++) {
	dbdir += "=" + *i;
    }
    // should autodetect flimsy - don't specify to test this
    if (create_dir_if_needed(dbdir)) {
	// directory was created, so do the indexing.
	// need to build temporary file and run it through makedb (yum)
	index_files_to_m36("makeDBflimsy", dbdir, change_names_to_paths(dbnames));
    }
    return Xapian::Muscat36::open_db(dbdir + "/DB", dbdir + "/keyfile");
}

Xapian::WritableDatabase
BackendManager::getwritedb_db(const vector<string> &/*dbnames*/)
{
    throw Xapian::InvalidArgumentError("Attempted to open writable db database");
}

Xapian::WritableDatabase
BackendManager::getwritedb_dbflimsy(const vector<string> &/*dbnames*/)
{
    throw Xapian::InvalidArgumentError("Attempted to open writable dbflimsy database");
}
#endif

Xapian::Database
BackendManager::get_database(const vector<string> &dbnames)
{
    return (this->*do_getdb)(dbnames);
}

Xapian::Database
BackendManager::get_database(const string &dbname)
{
    vector<string> dbnames;
    dbnames.push_back(dbname);
    return (this->*do_getdb)(dbnames);
}

Xapian::WritableDatabase
BackendManager::get_writable_database(const string &dbname)
{
    vector<string> dbnames;
    dbnames.push_back(dbname);
    return (this->*do_getwritedb)(dbnames);
}
