/** @file
 * @brief Database factories for non-remote databases.
 */
/* Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2011,2012,2013,2014,2015,2016,2017,2019 Olly Betts
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

#include "backends.h"
#include "databasehelpers.h"
#include "debuglog.h"
#include "filetests.h"
#include "fileutils.h"
#include "posixy_wrapper.h"
#include "str.h"

#include <cerrno>

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

#include <string>

using namespace std;

namespace Xapian {

static void
open_stub(Database& db, const string& file)
{
    read_stub_file(file,
		   [&db](const string& path) {
		       db.add_database(Database(path));
		   },
		   [&db](const string& path) {
#ifdef XAPIAN_HAS_CHERT_BACKEND
		       db.add_database(Database(new ChertDatabase(path)));
#else
		       (void)path;
#endif
		   },
		   [&db](const string& path) {
#ifdef XAPIAN_HAS_GLASS_BACKEND
		       db.add_database(Database(new GlassDatabase(path)));
#else
		       (void)path;
#endif
		   },
		   [&db](const string& prog, const string& args) {
#ifdef XAPIAN_HAS_REMOTE_BACKEND
		       db.add_database(Remote::open(prog, args));
#else
		       (void)prog;
		       (void)args;
#endif
		   },
		   [&db](const string& host, unsigned port) {
#ifdef XAPIAN_HAS_REMOTE_BACKEND
		       db.add_database(Remote::open(host, port));
#else
		       (void)host;
		       (void)port;
#endif
		   },
		   [&db]() {
#ifdef XAPIAN_HAS_INMEMORY_BACKEND
		       db.add_database(Database(string(), DB_BACKEND_INMEMORY));
#endif
		   });

    // Allowing a stub database with no databases listed allows things like
    // a "search all databases" feature to be implemented by generating a
    // stub database file without having to special case there not being any
    // databases yet.
    //
    // 1.0.x threw DatabaseOpeningError here, but with a "Bad line" message
    // with the line number just past the end of the file, which was a bit odd.
}

static void
open_stub(WritableDatabase& db, const string& file, int flags)
{
    read_stub_file(file,
		   [&db, flags](const string& path) {
		       db.add_database(WritableDatabase(path, flags));
		   },
		   [&db, &flags](const string& path) {
		       flags |= DB_BACKEND_CHERT;
		       db.add_database(WritableDatabase(path, flags));
		   },
		   [&db, &flags](const string& path) {
		       flags |= DB_BACKEND_GLASS;
		       db.add_database(WritableDatabase(path, flags));
		   },
		   [&db, flags](const string& prog, const string& args) {
#ifdef XAPIAN_HAS_REMOTE_BACKEND
		       db.add_database(Remote::open_writable(prog, args,
							     0, flags));
#else
		       (void)prog;
		       (void)args;
#endif
		   },
		   [&db, flags](const string& host, unsigned port) {
#ifdef XAPIAN_HAS_REMOTE_BACKEND
		       db.add_database(Remote::open_writable(host, port,
							     0, 10000, flags));
#else
		       (void)host;
		       (void)port;
#endif
		   },
		   [&db]() {
		       db.add_database(WritableDatabase(string(),
							DB_BACKEND_INMEMORY));
		   });

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
	if (errno == ENOENT) {
	    throw DatabaseNotFoundError("Couldn't stat '" + path + "'", errno);
	} else {
	    throw DatabaseOpeningError("Couldn't stat '" + path + "'", errno);
	}
    }

    if (S_ISREG(statbuf.st_mode)) {
	// Could be a stub database file, or a single file glass database.

	// Initialise to avoid bogus warning from GCC 4.9.2 with -Os.
	int fd = -1;
	switch (test_if_single_file_db(statbuf, path, &fd)) {
	  case BACKEND_GLASS:
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

    throw DatabaseNotFoundError("Couldn't detect type of database");
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
