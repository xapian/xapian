/** @file dbfactory.cc
 * @brief Database factories for non-remote databases.
 */
/* Copyright 2002,2003,2004,2005,2006,2007,2008,2009 Olly Betts
 * Copyright 2008 Lemur Consulting Ltd
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

#include "xapian/dbfactory.h"

#include "xapian/database.h"
#include "xapian/error.h"
#include "xapian/version.h" // For XAPIAN_HAS_XXX_BACKEND.

#include "debuglog.h"
#include "fileutils.h"
#include "str.h"
#include "utils.h"

#include "safeerrno.h"

#ifdef XAPIAN_HAS_BRASS_BACKEND
# include "brass/brass_database.h"
#endif
#ifdef XAPIAN_HAS_CHERT_BACKEND
# include "chert/chert_database.h"
#endif
#ifdef XAPIAN_HAS_FLINT_BACKEND
# include "flint/flint_database.h"
#endif
#ifdef XAPIAN_HAS_INMEMORY_BACKEND
# include "inmemory/inmemory_database.h"
#endif

#include <fstream>
#include <string>

using namespace std;

namespace Xapian {

#ifdef XAPIAN_HAS_BRASS_BACKEND
Database
Brass::open(const string &dir) {
    LOGCALL_STATIC(API, Database, "Brass::open", dir);
    RETURN(Database(new BrassDatabase(dir)));
}

WritableDatabase
Brass::open(const string &dir, int action, int block_size) {
    LOGCALL_STATIC(API, WritableDatabase, "Brass::open", dir | action | block_size);
    RETURN(WritableDatabase(new BrassWritableDatabase(dir, action, block_size)));
}
#endif

#ifdef XAPIAN_HAS_CHERT_BACKEND
Database
Chert::open(const string &dir) {
    LOGCALL_STATIC(API, Database, "Chert::open", dir);
    return Database(new ChertDatabase(dir));
}

WritableDatabase
Chert::open(const string &dir, int action, int block_size) {
    LOGCALL_STATIC(API, WritableDatabase, "Chert::open", dir | action | block_size);
    return WritableDatabase(new ChertWritableDatabase(dir, action, block_size));
}
#endif

#ifdef XAPIAN_HAS_FLINT_BACKEND
Database
Flint::open(const string &dir) {
    LOGCALL_STATIC(API, Database, "Flint::open", dir);
    return Database(new FlintDatabase(dir));
}

WritableDatabase
Flint::open(const string &dir, int action, int block_size) {
    LOGCALL_STATIC(API, WritableDatabase, "Flint::open", dir | action | block_size);
    return WritableDatabase(new FlintWritableDatabase(dir, action, block_size));
}
#endif

#ifdef XAPIAN_HAS_INMEMORY_BACKEND
WritableDatabase
InMemory::open() {
    LOGCALL_STATIC(API, WritableDatabase, "InMemory::open", NO_ARGS);
    RETURN(WritableDatabase(new InMemoryDatabase));
}
#endif

static void
open_stub(Database &db, const string &file)
{
    // A stub database is a text file with one or more lines of this format:
    // <dbtype> <serialised db object>
    //
    // Lines which start with a "#" character are ignored.
    //
    // Any paths specified in stub database files which are relative will be
    // considered to be relative to the directory containing the stub database.
    ifstream stub(file.c_str());
    if (!stub) {
	string msg = "Couldn't open stub database file: ";
	msg += file;
	throw Xapian::DatabaseOpeningError(msg, errno);
    }
    string line;
    unsigned int line_no = 0;
    while (getline(stub, line)) {
	++line_no;
	if (line.empty() || line[0] == '#')
	    continue;
	string::size_type space = line.find(' ');
	if (space == string::npos) space = line.size();

	string type(line, 0, space);
	line.erase(0, space + 1);

	if (type == "auto") {
	    resolve_relative_path(line, file);
	    db.add_database(Database(line));
	    continue;
	}

#ifdef XAPIAN_HAS_CHERT_BACKEND
	if (type == "chert") {
	    resolve_relative_path(line, file);
	    db.add_database(Chert::open(line));
	    continue;
	}
#endif

#ifdef XAPIAN_HAS_FLINT_BACKEND
	if (type == "flint") {
	    resolve_relative_path(line, file);
	    db.add_database(Flint::open(line));
	    continue;
	}
#endif

#ifdef XAPIAN_HAS_BRASS_BACKEND
	if (type == "brass") {
	    resolve_relative_path(line, file);
	    db.add_database(Brass::open(line));
	    continue;
	}
#endif

#ifdef XAPIAN_HAS_REMOTE_BACKEND
	if (type == "remote") {
	    string::size_type colon = line.find(':');
	    if (colon == 0) {
		// prog
		// FIXME: timeouts
		// Is it a security risk?
		space = line.find(' ');
		string args;
		if (space != string::npos) {
		    args.assign(line, space + 1, string::npos);
		    line.assign(line, 1, space - 1);
		} else {
		    line.erase(0, 1);
		}
		db.add_database(Remote::open(line, args));
	    } else if (colon != string::npos) {
		// tcp
		// FIXME: timeouts
		unsigned int port = atoi(line.c_str() + colon + 1);
		line.erase(colon);
		db.add_database(Remote::open(line, port));
	    }
	    continue;
	}
#endif

#ifdef XAPIAN_HAS_INMEMORY_BACKEND
	if (type == "inmemory" && line.empty()) {
	    db.add_database(InMemory::open());
	    continue;
	}
#endif

	// Don't include the line itself - that might help an attacker
	// by revealing part of a sensitive file's contents if they can
	// arrange for it to be read as a stub database via infelicities in
	// an application which uses Xapian.  The line number is enough
	// information to identify the problem line.
	throw DatabaseOpeningError(file + ':' + str(line_no) + ": Bad line");
    }

    // Allowing a stub database with no databases listed allows things like
    // a "search all databases" feature to be implemented by generating a
    // stub database file without having to special case there not being any
    // databases yet.
    //
    // 1.0.x throws DatabaseOpeningError here, but with a "Bad line" message
    // with the line number just past the end of the file, which is a bit odd.
}

static void
open_stub(WritableDatabase &db, const string &file, int action)
{
    // A stub database is a text file with one or more lines of this format:
    // <dbtype> <serialised db object>
    //
    // Lines which start with a "#" character, and lines which have no spaces
    // in them, are ignored.
    //
    // Any paths specified in stub database files which are relative will be
    // considered to be relative to the directory containing the stub database.
    ifstream stub(file.c_str());
    if (!stub) {
	string msg = "Couldn't open stub database file: ";
	msg += file;
	throw Xapian::DatabaseOpeningError(msg, errno);
    }
    string line;
    unsigned int line_no = 0;
    while (true) {
	if (db.internal.size() > 1) {
	    throw DatabaseOpeningError(file + ": Can't open a stub database listing multiple databases as a WritableDatabase");
	}

	if (!getline(stub, line)) break;

	++line_no;
	if (line.empty() || line[0] == '#')
	    continue;
	string::size_type space = line.find(' ');
	if (space == string::npos) space = line.size();

	string type(line, 0, space);
	line.erase(0, space + 1);

	if (type == "auto") {
	    resolve_relative_path(line, file);
	    db.add_database(WritableDatabase(line, action));
	    continue;
	}

#ifdef XAPIAN_HAS_CHERT_BACKEND
	if (type == "chert") {
	    resolve_relative_path(line, file);
	    db.add_database(Chert::open(line, action));
	    continue;
	}
#endif

#ifdef XAPIAN_HAS_FLINT_BACKEND
	if (type == "flint") {
	    resolve_relative_path(line, file);
	    db.add_database(Flint::open(line, action));
	    continue;
	}
#endif

#ifdef XAPIAN_HAS_BRASS_BACKEND
	if (type == "brass") {
	    resolve_relative_path(line, file);
	    db.add_database(Brass::open(line, action));
	    continue;
	}
#endif

#ifdef XAPIAN_HAS_REMOTE_BACKEND
	if (type == "remote") {
	    string::size_type colon = line.find(':');
	    if (colon == 0) {
		// prog
		// FIXME: timeouts
		// Is it a security risk?
		space = line.find(' ');
		string args;
		if (space != string::npos) {
		    args.assign(line, space + 1, string::npos);
		    line.assign(line, 1, space - 1);
		} else {
		    line.erase(0, 1);
		}
		db.add_database(Remote::open_writable(line, args));
	    } else if (colon != string::npos) {
		// tcp
		// FIXME: timeouts
		unsigned int port = atoi(line.c_str() + colon + 1);
		line.erase(colon);
		db.add_database(Remote::open_writable(line, port));
	    }
	    continue;
	}
#endif

#ifdef XAPIAN_HAS_INMEMORY_BACKEND
	if (type == "inmemory" && line.empty()) {
	    db.add_database(InMemory::open());
	    continue;
	}
#endif

	// Don't include the line itself - that might help an attacker
	// by revealing part of a sensitive file's contents if they can
	// arrange for it to be read as a stub database via infelicities in
	// an application which uses Xapian.  The line number is enough
	// information to identify the problem line.
	throw DatabaseOpeningError(file + ':' + str(line_no) + ": Bad line");
    }

    if (db.internal.empty()) {
	throw DatabaseOpeningError(file + ": No databases listed");
    }
}

Database
Auto::open_stub(const string &file)
{
    LOGCALL_STATIC(API, Database, "Auto::open_stub", file);
    Database db;
    open_stub(db, file);
    RETURN(db);
}

WritableDatabase
Auto::open_stub(const string &file, int action)
{
    LOGCALL_STATIC(API, WritableDatabase, "Auto::open_stub", file | action);
    WritableDatabase db;
    open_stub(db, file, action);
    RETURN(db);
}

Database::Database(const string &path)
{
    LOGCALL_CTOR(API, "Database", path);

    struct stat statbuf;
    if (stat(path, &statbuf) == -1) {
	throw DatabaseOpeningError("Couldn't stat '" + path + "'", errno);
    }

    if (S_ISREG(statbuf.st_mode)) {
	// The path is a file, so assume it is a stub database file.
	open_stub(*this, path);
	return;
    }

    if (rare(!S_ISDIR(statbuf.st_mode))) {
	throw DatabaseOpeningError("Not a regular file or directory: '" + path + "'");
    }

#ifdef XAPIAN_HAS_CHERT_BACKEND
    if (file_exists(path + "/iamchert")) {
	internal.push_back(new ChertDatabase(path));
	return;
    }
#endif

#ifdef XAPIAN_HAS_FLINT_BACKEND
    if (file_exists(path + "/iamflint")) {
	internal.push_back(new FlintDatabase(path));
	return;
    }
#endif

#ifdef XAPIAN_HAS_BRASS_BACKEND
    if (file_exists(path + "/iambrass")) {
	internal.push_back(new BrassDatabase(path));
	return;
    }
#endif

    // Check for "stub directories".
    string stub_file = path;
    stub_file += "/XAPIANDB";
    if (rare(!file_exists(stub_file))) {
	throw DatabaseOpeningError("Couldn't detect type of database");
    }

    open_stub(*this, stub_file);
}

#if defined XAPIAN_HAS_FLINT_BACKEND || \
    defined XAPIAN_HAS_CHERT_BACKEND || \
    defined XAPIAN_HAS_BRASS_BACKEND
#define HAVE_DISK_BACKEND
#endif

WritableDatabase::WritableDatabase(const std::string &path, int action)
    : Database()
{
    LOGCALL_CTOR(API, "WritableDatabase", path | action);
#ifdef HAVE_DISK_BACKEND
    enum {
#ifdef XAPIAN_HAS_CHERT_BACKEND
	CHERT,
#endif
#ifdef XAPIAN_HAS_FLINT_BACKEND
	FLINT,
#endif
#ifdef XAPIAN_HAS_BRASS_BACKEND
	BRASS,
#endif
	UNSET
    } type = UNSET;
#endif
    struct stat statbuf;
    if (stat(path, &statbuf) == -1) {
	// ENOENT probably just means that we need to create the directory.
	if (errno != ENOENT)
	    throw DatabaseOpeningError("Couldn't stat '" + path + "'", errno);
    } else {
	// File or directory already exists.

	if (S_ISREG(statbuf.st_mode)) {
	    // The path is a file, so assume it is a stub database file.
	    open_stub(*this, path, action);
	    return;
	}

	if (rare(!S_ISDIR(statbuf.st_mode))) {
	    throw DatabaseOpeningError("Not a regular file or directory: '" + path + "'");
	}

	if (file_exists(path + "/iamchert")) {
	    // Existing chert DB.
#ifdef XAPIAN_HAS_CHERT_BACKEND
	    type = CHERT;
#else
	    throw FeatureUnavailableError("Chert backend disabled");
#endif
	} else if (file_exists(path + "/iamflint")) {
	    // Existing flint DB.
#ifdef XAPIAN_HAS_FLINT_BACKEND
	    type = FLINT;
#else
	    throw FeatureUnavailableError("Flint backend disabled");
#endif
	} else if (file_exists(path + "/iambrass")) {
	    // Existing brass DB.
#ifdef XAPIAN_HAS_BRASS_BACKEND
	    type = BRASS;
#else
	    throw FeatureUnavailableError("Brass backend disabled");
#endif
	} else {
	    // Check for "stub directories".
	    string stub_file = path;
	    stub_file += "/XAPIANDB";
	    if (usual(file_exists(stub_file))) {
		open_stub(*this, stub_file, action);
		return;
	    }
	}
    }

#ifdef HAVE_DISK_BACKEND
    switch (type) {
	case UNSET: {
#ifdef XAPIAN_HAS_BRASS_BACKEND
	    // If only brass is enabled, there's no point checking the
	    // environmental variable.
#if defined XAPIAN_HAS_CHERT_BACKEND || defined XAPIAN_HAS_FLINT_BACKEND
	    // If $XAPIAN_PREFER_BRASS is set to a non-empty value, prefer brass
	    // if there's no existing database.
	    const char *p = getenv("XAPIAN_PREFER_BRASS");
	    if (p && *p)
	       	goto brass;
#endif
#endif
	}
	// Fall through to first enabled case, so order the remaining cases
	// by preference.
#ifdef XAPIAN_HAS_CHERT_BACKEND
	case CHERT:
	    internal.push_back(new ChertWritableDatabase(path, action, 8192));
	    break;
#endif
#ifdef XAPIAN_HAS_FLINT_BACKEND
	case FLINT:
	    internal.push_back(new FlintWritableDatabase(path, action, 8192));
	    break;
#endif
#ifdef XAPIAN_HAS_BRASS_BACKEND
	case BRASS:
#if defined XAPIAN_HAS_CHERT_BACKEND || defined XAPIAN_HAS_FLINT_BACKEND
brass:
#endif
	    internal.push_back(new BrassWritableDatabase(path, action, 8192));
	    break;
#endif
    }
#else
    throw FeatureUnavailableError("No disk-based writable backend is enabled");
#endif
}

}
