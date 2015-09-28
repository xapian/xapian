/** @file dbfactory.cc
 * @brief Database factories for non-remote databases.
 */
/* Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2011,2012,2013,2014 Olly Betts
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

#include "xapian/constants.h"
#include "xapian/database.h"
#include "xapian/error.h"
#include "xapian/version.h" // For XAPIAN_HAS_XXX_BACKEND.

#include "debuglog.h"
#include "filetests.h"
#include "fileutils.h"
#include "str.h"

#include "safeerrno.h"
#include <cstdlib> // For atoi().

#ifdef XAPIAN_HAS_GLASS_BACKEND
# include "glass/glass_database.h"
#endif
#ifdef XAPIAN_HAS_CHERT_BACKEND
# include "chert/chert_database.h"
#endif
#ifdef XAPIAN_HAS_INMEMORY_BACKEND
# include "inmemory/inmemory_database.h"
#endif
// Even if none of the above get included, we still need a definition of
// Database::Internal.
#include "backends/database.h"

#include <fstream>
#include <string>

using namespace std;

namespace Xapian {

#ifdef XAPIAN_HAS_INMEMORY_BACKEND
WritableDatabase
InMemory::open() {
    LOGCALL_STATIC(API, WritableDatabase, "InMemory::open", NO_ARGS);
    RETURN(WritableDatabase(new InMemoryDatabase));
}
#endif

static void
open_stub(Database &db, const string &file,
    const std::string * encryption_key,
    const std::string& encryption_cipher)
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
	    db.add_database(Database(line, 0, encryption_key, encryption_cipher));
	    continue;
	}

#ifdef XAPIAN_HAS_CHERT_BACKEND
	if (type == "chert") {
	    resolve_relative_path(line, file);
	    db.add_database(Database(new ChertDatabase(line, 0, 0, encryption_key, encryption_cipher)));
	    continue;
	}
#endif

#ifdef XAPIAN_HAS_GLASS_BACKEND
	if (type == "glass") {
	    resolve_relative_path(line, file);
	    db.add_database(Database(new GlassDatabase(line, 0, 0, encryption_key, encryption_cipher)));
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

	if (type == "flint") {
	    throw FeatureUnavailableError("Flint backend no longer supported");
	}

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
open_stub(WritableDatabase &db, const string &file, int flags,
    const std::string * encryption_key,
    const std::string& encryption_cipher)
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
	    db.add_database(WritableDatabase(line, flags, 0,
            encryption_key, encryption_cipher));
	    continue;
	}

#ifdef XAPIAN_HAS_CHERT_BACKEND
	if (type == "chert") {
	    resolve_relative_path(line, file);
	    db.add_database(WritableDatabase(line, flags|DB_BACKEND_CHERT, 0,
            encryption_key, encryption_cipher));
	    continue;
	}
#endif

#ifdef XAPIAN_HAS_GLASS_BACKEND
	if (type == "glass") {
	    resolve_relative_path(line, file);
	    db.add_database(WritableDatabase(line, flags|DB_BACKEND_GLASS, 0,
            encryption_key, encryption_cipher));
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
		db.add_database(Remote::open_writable(line, args, 0, flags));
	    } else if (colon != string::npos) {
		// tcp
		// FIXME: timeouts
		unsigned int port = atoi(line.c_str() + colon + 1);
		line.erase(colon);
		db.add_database(Remote::open_writable(line, port, 0, 10000, flags));
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

	if (type == "flint") {
	    throw FeatureUnavailableError("Flint backend no longer supported");
	}

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

Database::Database(const string &path, int flags,
    const std::string * encryption_key,
    const std::string& encryption_cipher)
{
    LOGCALL_CTOR(API, "Database", path|flags);

    int type = flags & DB_BACKEND_MASK_;
    switch (type) {
	case DB_BACKEND_CHERT:
#ifdef XAPIAN_HAS_CHERT_BACKEND
	    internal.push_back(new ChertDatabase(path, 0, 0,
            encryption_key, encryption_cipher));
	    return;
#else
	    throw FeatureUnavailableError("Chert backend disabled");
#endif
	case DB_BACKEND_GLASS:
#ifdef XAPIAN_HAS_GLASS_BACKEND
	    internal.push_back(new GlassDatabase(path, 0, 0,
            encryption_key, encryption_cipher));
	    return;
#else
	    throw FeatureUnavailableError("Glass backend disabled");
#endif
	case DB_BACKEND_STUB:
	    open_stub(*this, path, encryption_key, encryption_cipher);
	    return;
    }

    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) == -1) {
	throw DatabaseOpeningError("Couldn't stat '" + path + "'", errno);
    }

    if (S_ISREG(statbuf.st_mode)) {
	// The path is a file, so assume it is a stub database file.
	open_stub(*this, path, encryption_key, encryption_cipher);
	return;
    }

    if (rare(!S_ISDIR(statbuf.st_mode))) {
	throw DatabaseOpeningError("Not a regular file or directory: '" + path + "'");
    }

#ifdef XAPIAN_HAS_CHERT_BACKEND
    if (file_exists(path + "/iamchert")) {
	internal.push_back(new ChertDatabase(path, 0, 0,
        encryption_key, encryption_cipher));
	return;
    }
#endif

#ifdef XAPIAN_HAS_GLASS_BACKEND
    if (file_exists(path + "/iamglass")) {
	internal.push_back(new GlassDatabase(path, 0, 0,
        encryption_key, encryption_cipher));
	return;
    }
#endif

    // Check for "stub directories".
    string stub_file = path;
    stub_file += "/XAPIANDB";
    if (usual(file_exists(stub_file))) {
	open_stub(*this, stub_file, encryption_key, encryption_cipher);
	return;
    }

#ifndef XAPIAN_HAS_CHERT_BACKEND
    if (file_exists(path + "/iamchert")) {
	throw FeatureUnavailableError("Chert backend disabled");
    }
#endif
#ifndef XAPIAN_HAS_GLASS_BACKEND
    if (file_exists(path + "/iamglass")) {
	throw FeatureUnavailableError("Glass backend disabled");
    }
#endif
    if (file_exists(path + "/iamflint")) {
	throw FeatureUnavailableError("Flint backend no longer supported");
    }

    throw DatabaseOpeningError("Couldn't detect type of database");
}

#if defined XAPIAN_HAS_CHERT_BACKEND || \
    defined XAPIAN_HAS_GLASS_BACKEND
#define HAVE_DISK_BACKEND
#endif

WritableDatabase::WritableDatabase(const std::string &path, int flags, int block_size,
    const std::string * encryption_key, const std::string& encryption_cipher)
    : Database()
{
    LOGCALL_CTOR(API, "WritableDatabase", path|flags|block_size);
    // Avoid warning if both chert and glass are disabled.
    (void)block_size;
    int type = flags & DB_BACKEND_MASK_;
    // Clear the backend bits, so we just pass on other flags to open_stub, etc.
    flags &= ~DB_BACKEND_MASK_;
    if (type == 0) {
	struct stat statbuf;
	if (stat(path.c_str(), &statbuf) == -1) {
	    // ENOENT probably just means that we need to create the directory.
	    if (errno != ENOENT)
		throw DatabaseOpeningError("Couldn't stat '" + path + "'", errno);
	} else {
	    // File or directory already exists.

	    if (S_ISREG(statbuf.st_mode)) {
		// The path is a file, so assume it is a stub database file.
		open_stub(*this, path, flags, encryption_key, encryption_cipher);
		return;
	    }

	    if (rare(!S_ISDIR(statbuf.st_mode))) {
		throw DatabaseOpeningError("Not a regular file or directory: '" + path + "'");
	    }

	    if (file_exists(path + "/iamchert")) {
		// Existing chert DB.
#ifdef XAPIAN_HAS_CHERT_BACKEND
		type = DB_BACKEND_CHERT;
#else
		throw FeatureUnavailableError("Chert backend disabled");
#endif
	    } else if (file_exists(path + "/iamglass")) {
		// Existing glass DB.
#ifdef XAPIAN_HAS_GLASS_BACKEND
		type = DB_BACKEND_GLASS;
#else
		throw FeatureUnavailableError("Glass backend disabled");
#endif
	    } else if (file_exists(path + "/iamflint")) {
		// Existing flint DB.
		throw FeatureUnavailableError("Flint backend no longer supported");
	    } else {
		// Check for "stub directories".
		string stub_file = path;
		stub_file += "/XAPIANDB";
		if (usual(file_exists(stub_file))) {
		    open_stub(*this, stub_file, flags, encryption_key, encryption_cipher);
		    return;
		}
	    }
	}
    }

    switch (type) {
	case DB_BACKEND_STUB:
	    open_stub(*this, path, flags, encryption_key, encryption_cipher);
	    return;
	case 0: {
	    // If only one backend is enabled, there's no point checking the
	    // environmental variable.
#if defined XAPIAN_HAS_CHERT_BACKEND && defined XAPIAN_HAS_GLASS_BACKEND
	    // If $XAPIAN_PREFER_GLASS is set to a non-empty value, prefer glass
	    // if there's no existing database.
	    const char *p = getenv("XAPIAN_PREFER_GLASS");
	    if (p && *p)
	       	goto glass;
#endif
	}
	// Fall through to first enabled case, so order the remaining cases
	// by preference.
#ifdef XAPIAN_HAS_CHERT_BACKEND
	case DB_BACKEND_CHERT:
	    internal.push_back(new ChertWritableDatabase(path, flags, block_size,
            encryption_key, encryption_cipher));
	    return;
#endif
#ifdef XAPIAN_HAS_GLASS_BACKEND
	case DB_BACKEND_GLASS:
#ifdef XAPIAN_HAS_CHERT_BACKEND
glass:
#endif
	    internal.push_back(new GlassWritableDatabase(path, flags, block_size,
            encryption_key, encryption_cipher));
	    return;
#endif
    }
#ifndef HAVE_DISK_BACKEND
    throw FeatureUnavailableError("No disk-based writable backend is enabled");
#endif
}

}
