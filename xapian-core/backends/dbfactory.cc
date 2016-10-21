/** @file dbfactory.cc
 * @brief Database factories for non-remote databases.
 */
/* Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2011,2012,2013,2014,2015,2016 Olly Betts
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
#include "posixy_wrapper.h"
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

static bool
check_if_single_file_db(const struct stat & sb, const string & path,
			int * fd_ptr = NULL)
{
#ifdef XAPIAN_HAS_GLASS_BACKEND
    if (!S_ISREG(sb.st_mode)) return false;
    // Look at the size as a clue - if it's 0 or not a multiple of 2048,
    // then it's not a single-file glass database.  If it is, peek at the start
    // of the file to determine which it is.
    if (sb.st_size == 0 || sb.st_size % 2048 != 0) return false;
    int fd = posixy_open(path.c_str(), O_RDONLY|O_BINARY);
    if (fd != -1) {
	char magic_buf[14];
	// FIXME: Don't duplicate magic check here...
	if (io_read(fd, magic_buf, 14) == 14 &&
	    lseek(fd, 0, SEEK_SET) == 0 &&
	    memcmp(magic_buf, "\x0f\x0dXapian Glass", 14) == 0) {
	    if (fd_ptr) {
		*fd_ptr = fd;
	    } else {
		::close(fd);
	    }
	    return true;
	}
	::close(fd);
    }
#else
    (void)sb;
    (void)path;
    (void)fd_ptr;
#endif
    return false;
}

namespace Xapian {

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
	    db.add_database(Database(new ChertDatabase(line)));
	    continue;
	}
#endif

#ifdef XAPIAN_HAS_GLASS_BACKEND
	if (type == "glass") {
	    resolve_relative_path(line, file);
	    db.add_database(Database(new GlassDatabase(line)));
	    continue;
	}
#endif

#ifdef XAPIAN_HAS_REMOTE_BACKEND
	if (type == "remote" && !line.empty()) {
	    if (line[0] == ':') {
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
		continue;
	    }
	    string::size_type colon = line.rfind(':');
	    if (colon != string::npos) {
		// tcp
		// FIXME: timeouts
		// Avoid misparsing an IPv6 address without a port number.  The
		// port number is required, so just leave that case to the
		// error handling further below.
		if (!(line[0] == '[' && line.back() == ']')) {
		    unsigned int port = atoi(line.c_str() + colon + 1);
		    line.erase(colon);
		    if (line[0] == '[' && line.back() == ']') {
			line.erase(line.size() - 1, 1);
			line.erase(0, 1);
		    }
		    db.add_database(Remote::open(line, port));
		    continue;
		}
	    }
	}
#endif

#ifdef XAPIAN_HAS_INMEMORY_BACKEND
	if (type == "inmemory" && line.empty()) {
	    db.add_database(Database(string(), DB_BACKEND_INMEMORY));
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
open_stub(WritableDatabase &db, const string &file, int flags)
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
	    db.add_database(WritableDatabase(line, flags));
	    continue;
	}

#ifdef XAPIAN_HAS_CHERT_BACKEND
	if (type == "chert") {
	    resolve_relative_path(line, file);
	    db.add_database(WritableDatabase(line, flags|DB_BACKEND_CHERT));
	    continue;
	}
#endif

#ifdef XAPIAN_HAS_GLASS_BACKEND
	if (type == "glass") {
	    resolve_relative_path(line, file);
	    db.add_database(WritableDatabase(line, flags|DB_BACKEND_GLASS));
	    continue;
	}
#endif

#ifdef XAPIAN_HAS_REMOTE_BACKEND
	if (type == "remote") {
	    if (line[0] == ':') {
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
		continue;
	    }
	    string::size_type colon = line.rfind(':');
	    if (colon != string::npos) {
		// tcp
		// FIXME: timeouts
		// Avoid misparsing an IPv6 address without a port number.  The
		// port number is required, so just leave that case to the
		// error handling further below.
		if (!(line[0] == '[' && line.back() == ']')) {
		    unsigned int port = atoi(line.c_str() + colon + 1);
		    line.erase(colon);
		    if (line[0] == '[' && line.back() == ']') {
			line.erase(line.size() - 1, 1);
			line.erase(0, 1);
		    }
		    db.add_database(Remote::open_writable(line, port, 0, 10000, flags));
		    continue;
		}
	    }
	}
#endif

#ifdef XAPIAN_HAS_INMEMORY_BACKEND
	if (type == "inmemory" && line.empty()) {
	    db.add_database(WritableDatabase(string(), DB_BACKEND_INMEMORY));
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

Database::Database(const string &path, int flags)
{
    LOGCALL_CTOR(API, "Database", path|flags);

    int type = flags & DB_BACKEND_MASK_;
    switch (type) {
	case DB_BACKEND_CHERT:
#ifdef XAPIAN_HAS_CHERT_BACKEND
	    internal.push_back(new ChertDatabase(path));
	    return;
#else
	    throw FeatureUnavailableError("Chert backend disabled");
#endif
	case DB_BACKEND_GLASS:
#ifdef XAPIAN_HAS_GLASS_BACKEND
	    internal.push_back(new GlassDatabase(path));
	    return;
#else
	    throw FeatureUnavailableError("Glass backend disabled");
#endif
	case DB_BACKEND_STUB:
	    open_stub(*this, path);
	    return;
	case DB_BACKEND_INMEMORY:
#ifdef XAPIAN_HAS_INMEMORY_BACKEND
	    internal.push_back(new InMemoryDatabase());
	    return;
#else
	    throw FeatureUnavailableError("Inmemory backend disabled");
#endif
    }

    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) == -1) {
	throw DatabaseOpeningError("Couldn't stat '" + path + "'", errno);
    }

    if (S_ISREG(statbuf.st_mode)) {
	// Could be a stub database file, or a single file glass database.
	int fd;
	if (check_if_single_file_db(statbuf, path, &fd)) {
#ifdef XAPIAN_HAS_GLASS_BACKEND
	    // Single file glass format.
	    internal.push_back(new GlassDatabase(fd));
	    return;
#else
	    throw FeatureUnavailableError("Glass backend disabled");
#endif
	}

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

#ifdef XAPIAN_HAS_GLASS_BACKEND
    if (file_exists(path + "/iamglass")) {
	internal.push_back(new GlassDatabase(path));
	return;
    }
#endif

    // Check for "stub directories".
    string stub_file = path;
    stub_file += "/XAPIANDB";
    if (usual(file_exists(stub_file))) {
	open_stub(*this, stub_file);
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

Database::Database(int fd, int flags)
{
    LOGCALL_CTOR(API, "Database", fd|flags);
    if (rare(fd < 0))
	throw InvalidArgumentError("fd < 0");

#ifdef XAPIAN_HAS_GLASS_BACKEND
    int type = flags & DB_BACKEND_MASK_;
    switch (type) {
	case 0: case DB_BACKEND_GLASS:
	    internal.push_back(new GlassDatabase(fd));
	    return;
    }
#else
    (void)flags;
#endif

    (void)::close(fd);
    throw DatabaseOpeningError("Couldn't detect type of database");
}

#if defined XAPIAN_HAS_CHERT_BACKEND || \
    defined XAPIAN_HAS_GLASS_BACKEND
#define HAVE_DISK_BACKEND
#endif

WritableDatabase::WritableDatabase(const std::string &path, int flags, int block_size)
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
		open_stub(*this, path, flags);
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
		    open_stub(*this, stub_file, flags);
		    return;
		}
	    }
	}
    }

    switch (type) {
	case DB_BACKEND_STUB:
	    open_stub(*this, path, flags);
	    return;
	case 0:
	    // Fall through to first enabled case, so order the remaining cases
	    // by preference.
#ifdef XAPIAN_HAS_GLASS_BACKEND
	case DB_BACKEND_GLASS:
	    internal.push_back(new GlassWritableDatabase(path, flags, block_size));
	    return;
#endif
#ifdef XAPIAN_HAS_CHERT_BACKEND
	case DB_BACKEND_CHERT:
	    internal.push_back(new ChertWritableDatabase(path, flags, block_size));
	    return;
#endif
	case DB_BACKEND_INMEMORY:
#ifdef XAPIAN_HAS_INMEMORY_BACKEND
	    internal.push_back(new InMemoryDatabase());
	    return;
#else
	    throw FeatureUnavailableError("Inmemory backend disabled");
#endif
    }
#ifndef HAVE_DISK_BACKEND
    throw FeatureUnavailableError("No disk-based writable backend is enabled");
#endif
}

}
