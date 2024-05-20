/** @file
 * @brief Database factories for non-remote databases.
 */
/* Copyright 2002-2024 Olly Betts
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
#include "glass/glass_defs.h"
#ifdef XAPIAN_HAS_HONEY_BACKEND
# include "honey/honey_database.h"
#endif
#include "honey/honey_defs.h"
#ifdef XAPIAN_HAS_INMEMORY_BACKEND
# include "inmemory/inmemory_database.h"
#endif
// Even if none of the above get included, we still need a definition of
// Database::Internal.
#include "backends/databaseinternal.h"

#include <string>

using namespace std;

namespace Xapian {

static void
open_stub(Database& db, string_view file)
{
    read_stub_file(file,
		   [&db](string_view path) {
		       db.add_database(Database(path));
		   },
		   [&db](string_view path) {
#ifdef XAPIAN_HAS_GLASS_BACKEND
		       db.add_database(Database(new GlassDatabase(path)));
#else
		       (void)path;
#endif
		   },
		   [&db](string_view path) {
#ifdef XAPIAN_HAS_HONEY_BACKEND
		       db.add_database(Database(new HoneyDatabase(path)));
#else
		       (void)path;
#endif
		   },
		   [&db](string_view prog, string_view args) {
#ifdef XAPIAN_HAS_REMOTE_BACKEND
		       db.add_database(Remote::open(prog, args));
#else
		       (void)prog;
		       (void)args;
#endif
		   },
		   [&db](string_view host, unsigned port) {
#ifdef XAPIAN_HAS_REMOTE_BACKEND
		       db.add_database(Remote::open(host, port));
#else
		       (void)host;
		       (void)port;
#endif
		   },
		   [&db]() {
#ifdef XAPIAN_HAS_INMEMORY_BACKEND
		       db.add_database(Database(""sv, DB_BACKEND_INMEMORY));
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
open_stub(WritableDatabase& db, string_view file, int flags)
{
    read_stub_file(file,
		   [&db, flags](string_view path) {
		       db.add_database(WritableDatabase(path, flags));
		   },
		   [&db, &flags](string_view path) {
		       flags |= DB_BACKEND_GLASS;
		       db.add_database(WritableDatabase(path, flags));
		   },
		   [](string_view) {
		       auto msg = "Honey databases don't support writing";
		       throw Xapian::DatabaseOpeningError(msg);
		   },
		   [&db, flags](string_view prog, string_view args) {
#ifdef XAPIAN_HAS_REMOTE_BACKEND
		       db.add_database(Remote::open_writable(prog, args,
							     0, flags));
#else
		       (void)prog;
		       (void)args;
#endif
		   },
		   [&db, flags](string_view host, unsigned port) {
#ifdef XAPIAN_HAS_REMOTE_BACKEND
		       db.add_database(Remote::open_writable(host, port,
							     0, 10000, flags));
#else
		       (void)host;
		       (void)port;
#endif
		   },
		   [&db]() {
		       db.add_database(WritableDatabase(""sv,
							DB_BACKEND_INMEMORY));
		   });

    if (db.internal->size() == 0) {
	throw DatabaseOpeningError(string{file} + ": No databases listed");
    }
}

Database::Database(string_view path, int flags)
    : Database()
{
    LOGCALL_CTOR(API, "Database", path|flags);

    int type = flags & DB_BACKEND_MASK_;
    switch (type) {
	case DB_BACKEND_CHERT:
	    throw FeatureUnavailableError("Chert backend no longer supported");
	case DB_BACKEND_GLASS:
#ifdef XAPIAN_HAS_GLASS_BACKEND
	    internal = new GlassDatabase(path);
	    return;
#else
	    throw FeatureUnavailableError("Glass backend disabled");
#endif
	case DB_BACKEND_HONEY:
#ifdef XAPIAN_HAS_HONEY_BACKEND
	    internal = new HoneyDatabase(path);
	    return;
#else
	    throw FeatureUnavailableError("Honey backend disabled");
#endif
	case DB_BACKEND_STUB:
	    open_stub(*this, path);
	    return;
	case DB_BACKEND_INMEMORY:
#ifdef XAPIAN_HAS_INMEMORY_BACKEND
	    internal = new InMemoryDatabase();
	    return;
#else
	    throw FeatureUnavailableError("Inmemory backend disabled");
#endif
    }

    string filename{path};
    struct stat statbuf;
    if (stat(filename.c_str(), &statbuf) == -1) {
	if (errno == ENOENT) {
	    throw DatabaseNotFoundError("Couldn't stat '" + filename + "'",
					errno);
	} else {
	    throw DatabaseOpeningError("Couldn't stat '" + filename + "'",
				       errno);
	}
    }

    if (S_ISREG(statbuf.st_mode)) {
	// Could be a stub database file, or a single file glass database.

	// Initialise to avoid bogus warning from GCC 4.9.2 with -Os.
	int fd = -1;
	switch (test_if_single_file_db(statbuf, filename, &fd)) {
	    case BACKEND_GLASS:
#ifdef XAPIAN_HAS_GLASS_BACKEND
		// Single file glass format.
		internal = new GlassDatabase(fd);
		return;
#else
		throw FeatureUnavailableError("Glass backend disabled");
#endif
	    case BACKEND_HONEY:
#ifdef XAPIAN_HAS_HONEY_BACKEND
		// Single file honey format.
		internal = new HoneyDatabase(fd);
		return;
#else
		throw FeatureUnavailableError("Honey backend disabled");
#endif
	}

	open_stub(*this, path);
	return;
    }

    if (rare(!S_ISDIR(statbuf.st_mode))) {
	throw DatabaseOpeningError("Not a regular file or directory: "
				   "'" + filename + "'");
    }

#ifdef XAPIAN_HAS_GLASS_BACKEND
    filename += "/iamglass";
    if (file_exists(filename)) {
	internal = new GlassDatabase(path);
	return;
    }
#endif

#ifdef XAPIAN_HAS_HONEY_BACKEND
    filename.resize(path.size());
    filename += "/iamhoney";
    if (file_exists(filename)) {
	internal = new HoneyDatabase(path);
	return;
    }
#endif

    // Check for "stub directories".
    filename.resize(path.size());
    filename += "/XAPIANDB";
    if (usual(file_exists(filename))) {
	open_stub(*this, filename);
	return;
    }

#ifndef XAPIAN_HAS_GLASS_BACKEND
    filename.resize(path.size());
    filename += "/iamglass";
    if (file_exists(filename)) {
	throw FeatureUnavailableError("Glass backend disabled");
    }
#endif
#ifndef XAPIAN_HAS_HONEY_BACKEND
    filename.resize(path.size());
    filename += "/iamhoney";
    if (file_exists(filename)) {
	throw FeatureUnavailableError("Honey backend disabled");
    }
#endif
    filename.resize(path.size());
    filename += "/iamchert";
    if (file_exists(filename)) {
	throw FeatureUnavailableError("Chert backend no longer supported");
    }
    filename.resize(path.size());
    filename += "/iamflint";
    if (file_exists(filename)) {
	throw FeatureUnavailableError("Flint backend no longer supported");
    }

    throw DatabaseNotFoundError("Couldn't detect type of database");
}

/** Helper factory function.
 *
 *  This allows us to initialise Database::internal via the constructor's
 *  initialiser list, which we want to be able to do as Database::internal
 *  is an intrusive_ptr_nonnull, so we can't set it to NULL in the initialiser
 *  list and then fill it in later in the constructor body.
 */
static Database::Internal*
database_factory(int fd, int flags)
{
    if (rare(fd < 0))
	throw InvalidArgumentError("fd < 0", EBADF);

#if defined XAPIAN_HAS_GLASS_BACKEND || defined XAPIAN_HAS_HONEY_BACKEND
    int type = flags & DB_BACKEND_MASK_;
    if (type == 0) {
	switch (test_if_single_file_db(fd)) {
	  case BACKEND_GLASS:
	    type = DB_BACKEND_GLASS;
	    break;
	  case BACKEND_HONEY:
	    type = DB_BACKEND_HONEY;
	    break;
	}
    }
    switch (type) {
#ifdef XAPIAN_HAS_GLASS_BACKEND
	case DB_BACKEND_GLASS:
	    return new GlassDatabase(fd);
#endif
#ifdef XAPIAN_HAS_HONEY_BACKEND
	case DB_BACKEND_HONEY:
	    return new HoneyDatabase(fd);
#endif
    }
#endif

    (void)::close(fd);
    throw DatabaseOpeningError("Couldn't detect type of database fd");
}

Database::Database(int fd, int flags)
    : internal(database_factory(fd, flags))
{
    LOGCALL_CTOR(API, "Database", fd|flags);
}

#if defined XAPIAN_HAS_GLASS_BACKEND
#define HAVE_DISK_BACKEND
#endif

WritableDatabase::WritableDatabase(std::string_view path,
				   int flags,
				   int block_size)
    : Database()
{
    LOGCALL_CTOR(API, "WritableDatabase", path|flags|block_size);
    // Avoid warning if all disk-based backends are disabled.
    (void)block_size;
    string filename{path};
    int type = flags & DB_BACKEND_MASK_;
    // Clear the backend bits, so we just pass on other flags to open_stub, etc.
    flags &= ~DB_BACKEND_MASK_;
    if (type == 0) {
	struct stat statbuf;
	if (stat(filename.c_str(), &statbuf) == -1) {
	    // ENOENT probably just means that we need to create the directory.
	    if (errno != ENOENT)
		throw DatabaseOpeningError("Couldn't stat '" + filename + "'",
					   errno);
	} else {
	    // File or directory already exists.

	    if (S_ISREG(statbuf.st_mode)) {
		// The path is a file, so assume it is a stub database file.
		open_stub(*this, path, flags);
		return;
	    }

	    if (rare(!S_ISDIR(statbuf.st_mode))) {
		throw DatabaseOpeningError("Not a regular file or directory: "
					   "'" + filename + "'");
	    }

	    filename += "/iamglass";
	    if (file_exists(filename)) {
		// Existing glass DB.
#ifdef XAPIAN_HAS_GLASS_BACKEND
		type = DB_BACKEND_GLASS;
#else
		throw FeatureUnavailableError("Glass backend disabled");
#endif
	    }

	    filename.resize(path.size());
	    filename += "/iamhoney";
	    if (file_exists(filename)) {
		// Existing honey DB.
		throw InvalidOperationError("Honey backend doesn't support "
					    "updating existing databases");
	    }

	    filename.resize(path.size());
	    filename += "/iamchert";
	    if (file_exists(filename)) {
		// Existing chert DB.
		throw FeatureUnavailableError("Chert backend no longer supported");
	    }

	    filename.resize(path.size());
	    filename += "/iamflint";
	    if (file_exists(filename)) {
		// Existing flint DB.
		throw FeatureUnavailableError("Flint backend no longer supported");
	    }

	    // Check for "stub directories".
	    filename.resize(path.size());
	    filename += "/XAPIANDB";
	    if (usual(file_exists(filename))) {
		open_stub(*this, filename, flags);
		return;
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
	    internal = new GlassWritableDatabase(path, flags, block_size);
	    return;
#endif
	case DB_BACKEND_HONEY:
	    throw InvalidArgumentError("Honey backend doesn't support "
				       "updating existing databases");
	case DB_BACKEND_CHERT:
	    throw FeatureUnavailableError("Chert backend no longer supported");
	case DB_BACKEND_INMEMORY:
#ifdef XAPIAN_HAS_INMEMORY_BACKEND
	    internal = new InMemoryDatabase();
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
