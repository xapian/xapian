/** @file
 * @brief Check the consistency of a database or table.
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2019 Olly Betts
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
#include "xapian/database.h"

#include "xapian/constants.h"
#include "xapian/error.h"

#ifdef XAPIAN_HAS_GLASS_BACKEND
#include "glass/glass_changes.h"
#include "glass/glass_dbcheck.h"
#include "glass/glass_version.h"
#endif
#ifdef XAPIAN_HAS_CHERT_BACKEND
#include "chert/chert_database.h"
#include "chert/chert_dbcheck.h"
#include "chert/chert_types.h"
#include "chert/chert_version.h"
#endif

#include "backends.h"
#include "databasehelpers.h"
#include "filetests.h"
#include "omassert.h"
#include "stringutils.h"

#include <ostream>
#include <stdexcept>

using namespace std;

#ifdef XAPIAN_HAS_GLASS_BACKEND
// Tables to check for a glass database.  Note: it's important to check
// termlist before postlist so that we can cross-check the document lengths.
static const struct { char name[9]; } glass_tables[] = {
    { "docdata" },
    { "termlist" },
    { "postlist" },
    { "position" },
    { "spelling" },
    { "synonym" }
};
#endif

[[noreturn]]
static void
throw_no_db_to_check()
{
    auto msg = "Couldn't find Xapian database or table to check";
    throw Xapian::DatabaseOpeningError(msg, ENOENT);
}

// FIXME: We don't currently cross-check wdf between postlist and termlist.
// It's hard to see how to efficiently.  We do cross-check doclens, but that
// "only" requires (4 * last_docid()) bytes.

#if defined XAPIAN_HAS_CHERT_BACKEND || defined XAPIAN_HAS_GLASS_BACKEND
static void
reserve_doclens(vector<Xapian::termcount>& doclens, Xapian::docid last_docid,
		ostream * out)
{
    if (last_docid >= 0x40000000ul / sizeof(Xapian::termcount)) {
	// The memory block needed by the vector would be >= 1GB.
	if (out)
	    *out << "Cross-checking document lengths between the postlist and "
		    "termlist tables would use more than 1GB of memory, so "
		    "skipping that check" << endl;
	return;
    }
    try {
	doclens.reserve(last_docid + 1);
    } catch (const std::bad_alloc &) {
	// Failed to allocate the required memory.
	if (out)
	    *out << "Couldn't allocate enough memory for cross-checking document "
		    "lengths between the postlist and termlist tables, so "
		    "skipping that check" << endl;
    } catch (const std::length_error &) {
	// There are too many elements for the vector to handle!
	if (out)
	    *out << "Couldn't allocate enough elements for cross-checking document "
		    "lengths between the postlist and termlist tables, so "
		    "skipping that check" << endl;
    }
}
#endif

static size_t
check_db_dir(const string & path, int opts, std::ostream *out)
{
    struct stat sb;
    if (stat((path + "/iamchert").c_str(), &sb) == 0) {
#ifndef XAPIAN_HAS_CHERT_BACKEND
	(void)opts;
	(void)out;
	throw Xapian::FeatureUnavailableError("Chert database support isn't enabled");
#else
	// Check a whole chert database directory.
	vector<Xapian::termcount> doclens;
	size_t errors = 0;

	// If we can't read the doccount or last docid, set them to their
	// maximum values to suppress errors.
	Xapian::doccount doccount = Xapian::doccount(-1);
	Xapian::docid db_last_docid = CHERT_MAX_DOCID;

	chert_revision_number_t rev = 0;
	chert_revision_number_t * rev_ptr = &rev;
	try {
	    // Open at the lower level so we can get the revision number.
	    ChertDatabase db(path);
	    doccount = db.get_doccount();
	    db_last_docid = db.get_lastdocid();
	    reserve_doclens(doclens, db_last_docid, out);
	    rev = db.get_revision_number();
	} catch (const Xapian::Error & e) {
	    // Ignore so we can check a database too broken to open.
	    if (out)
		*out << "Database couldn't be opened for reading: "
		     << e.get_description()
		     << "\nContinuing check anyway" << endl;
	    ++errors;
	}

	size_t pre_table_check_errors = errors;

	// Check all the btrees.
	//
	// Note: it's important to check "termlist" before "postlist" so
	// that we can cross-check the document lengths; also we check
	// "record" first as that's the last committed, so has the most
	// reliable rootblock revision in DBCHECK_FIX mode.
	static const struct { char name[9]; } tables[] = {
	    { "record" },
	    { "termlist" },
	    { "postlist" },
	    { "position" },
	    { "spelling" },
	    { "synonym" }
	};
	for (auto t : tables) {
	    const char * name = t.name;
	    if (out)
		*out << name << ":\n";
	    if (strcmp(name, "record") != 0 && strcmp(name, "postlist") != 0) {
		// Other tables are created lazily, so may not exist.
		string table(path);
		table += '/';
		table += name;
		table += ".DB";
		if (!file_exists(table)) {
		    if (out) {
			if (strcmp(name, "termlist") == 0) {
			    *out << "Not present.\n";
			} else {
			    *out << "Lazily created, and not yet used.\n";
			}
			*out << endl;
		    }
		    continue;
		}
	    }
	    errors += check_chert_table(name, path, rev_ptr, opts, doclens,
					doccount, db_last_docid, out);
	}

	if (errors == pre_table_check_errors && (opts & Xapian::DBCHECK_FIX)) {
	    // Check the version file is OK and if not, recreate it.
	    ChertVersion iam(path);
	    try {
		iam.read_and_check();
	    } catch (const Xapian::DatabaseError &) {
		iam.create();
	    }
	}
	return errors;
#endif
    }

    if (stat((path + "/iamglass").c_str(), &sb) == 0) {
#ifndef XAPIAN_HAS_GLASS_BACKEND
	(void)opts;
	(void)out;
	throw Xapian::FeatureUnavailableError("Glass database support isn't enabled");
#else
	// Check a whole glass database directory.
	vector<Xapian::termcount> doclens;
	size_t errors = 0;

	try {
	    // Check if the database can actually be opened.
	    Xapian::Database db(path);
	} catch (const Xapian::Error & e) {
	    // Continue - we can still usefully look at how it is broken.
	    if (out)
		*out << "Database couldn't be opened for reading: "
		     << e.get_description()
		     << "\nContinuing check anyway" << endl;
	    ++errors;
	}

	GlassVersion version_file(path);
	version_file.read();
	for (glass_revision_number_t r = version_file.get_revision(); r != 0; --r) {
	    string changes_file = path;
	    changes_file += "/changes";
	    changes_file += str(r);
	    if (file_exists(changes_file))
		GlassChanges::check(changes_file);
	}

	Xapian::docid doccount = version_file.get_doccount();
	Xapian::docid db_last_docid = version_file.get_last_docid();
	if (db_last_docid < doccount) {
	    if (out)
		*out << "last_docid = " << db_last_docid << " < doccount = "
		     << doccount << endl;
	    ++errors;
	}
	reserve_doclens(doclens, db_last_docid, out);

	// Check all the tables.
	for (auto t : glass_tables) {
	    errors += check_glass_table(t.name, path, version_file, opts,
					doclens, out);
	}
	return errors;
#endif
    }

    if (stat((path + "/iamflint").c_str(), &sb) == 0) {
	// Flint is no longer supported as of Xapian 1.3.0.
	throw Xapian::FeatureUnavailableError("Flint database support was removed in Xapian 1.3.0");
    }

    if (stat((path + "/iambrass").c_str(), &sb) == 0) {
	// Brass was renamed to glass as of Xapian 1.3.2.
	throw Xapian::FeatureUnavailableError("Brass database support was removed in Xapian 1.3.2");
    }

    if (stat((path + "/record_DB").c_str(), &sb) == 0) {
	// Quartz is no longer supported as of Xapian 1.1.0.
	throw Xapian::FeatureUnavailableError("Quartz database support was removed in Xapian 1.1.0");
    }

    throw Xapian::DatabaseOpeningError(
	    "Directory does not contain a Xapian database");
}

/** Check a database table.
 *
 *  @param filename	The filename of the table (only used to get the directory and
 *  @param opts		Xapian::check() options
 *  @param out		std::ostream to write messages to (or NULL for no messages)
 *  @param backend	Backend type (a BACKEND_XXX constant)
 */
static size_t
check_db_table(const string& filename, int opts, std::ostream* out, int backend)
{
    size_t p = filename.find_last_of(DIR_SEPS);
    // If we found a directory separator, advance p to the next character.  If
    // we didn't, incrementing string::npos will give us 0, which is what we
    // want.
    ++p;

    string dir(filename, 0, p);

    string tablename;
    while (p != filename.size()) {
	char ch = filename[p++];
	if (ch == '.') break;
	tablename += C_tolower(ch);
    }

#if defined XAPIAN_HAS_CHERT_BACKEND || defined XAPIAN_HAS_GLASS_BACKEND
    vector<Xapian::termcount> doclens;
#else
    (void)opts;
    (void)out;
#endif

    switch (backend) {
      case BACKEND_GLASS: {
#ifndef XAPIAN_HAS_GLASS_BACKEND
	auto msg = "Glass database support isn't enabled";
	throw Xapian::FeatureUnavailableError(msg);
#else
	GlassVersion version_file(dir);
	version_file.read();
	return check_glass_table(tablename.c_str(), dir, version_file, opts,
				 doclens, out);
#endif
      }

      case BACKEND_CHERT:
	break;

      default:
	Assert(false);
	break;
    }

    // Flint and brass also used the extension ".DB", so check that we
    // haven't been passed a single table in a flint or brass database.
    struct stat sb;
    if (stat((dir + "/iamflint").c_str(), &sb) == 0) {
	// Flint is no longer supported as of Xapian 1.3.0.
	throw Xapian::FeatureUnavailableError("Flint database support was removed in Xapian 1.3.0");
    }
    if (stat((dir + "/iambrass").c_str(), &sb) == 0) {
	// Brass was renamed to glass as of Xapian 1.3.2.
	throw Xapian::FeatureUnavailableError("Brass database support was removed in Xapian 1.3.2");
    }
#ifndef XAPIAN_HAS_CHERT_BACKEND
    throw Xapian::FeatureUnavailableError("Chert database support isn't enabled");
#else
    // Set the doccount and the last docid to their maximum values to suppress
    // errors.
    return check_chert_table(tablename.c_str(), dir, NULL, opts, doclens,
			     Xapian::doccount(-1), CHERT_MAX_DOCID, out);
#endif
}

/** Check a single file DB from an fd.
 *
 *  Closes the fd.
 */
static size_t
check_db_fd(int fd, int opts, std::ostream* out, int backend)
{
    if (backend == BACKEND_UNKNOWN) {
	// FIXME: Actually probe.
	backend = BACKEND_GLASS;
    }

    size_t errors = 0;
    switch (backend) {
      case BACKEND_GLASS: {
	// Check a single-file glass database.
#ifdef XAPIAN_HAS_GLASS_BACKEND
	// GlassVersion's destructor will close fd.
	GlassVersion version_file(fd);
	version_file.read();

	Xapian::docid doccount = version_file.get_doccount();
	Xapian::docid db_last_docid = version_file.get_last_docid();
	if (db_last_docid < doccount) {
	    if (out)
		*out << "last_docid = " << db_last_docid << " < doccount = "
		     << doccount << endl;
	    ++errors;
	}
	vector<Xapian::termcount> doclens;
	reserve_doclens(doclens, db_last_docid, out);

	// Check all the tables.
	for (auto t : glass_tables) {
	    errors += check_glass_table(t.name, fd, version_file.get_offset(),
					version_file, opts, doclens,
					out);
	}
	break;
#else
	(void)opts;
	(void)out;
	::close(fd);
	throw Xapian::FeatureUnavailableError("Glass database support isn't enabled");
#endif
      }
      default:
	Assert(false);
    }
    return errors;
}

namespace Xapian {

static size_t
check_stub(const string& stub_path, int opts, std::ostream* out)
{
    size_t errors = 0;
    read_stub_file(stub_path,
		   [&errors, opts, out](const string& path) {
		       errors += Database::check(path, opts, out);
		   },
		   [&errors, opts, out](const string& path) {
		       // FIXME: Doesn't check the database type is chert.
		       errors += Database::check(path, opts, out);
		   },
		   [&errors, opts, out](const string& path) {
		       // FIXME: Doesn't check the database type is glass.
		       errors += Database::check(path, opts, out);
		   },
		   [](const string&, const string&) {
		       auto msg = "Remote database checking not implemented";
		       throw Xapian::UnimplementedError(msg);
		   },
		   [](const string&, unsigned) {
		       auto msg = "Remote database checking not implemented";
		       throw Xapian::UnimplementedError(msg);
		   },
		   []() {
		       auto msg = "InMemory database checking not implemented";
		       throw Xapian::UnimplementedError(msg);
		   });
    return errors;
}

size_t
Database::check_(const string * path_ptr, int fd, int opts, std::ostream *out)
{
    if (!out) {
	// If we have nowhere to write output, then disable all the options
	// which only affect what we output.
	opts &= Xapian::DBCHECK_FIX;
    }

    if (path_ptr == NULL) {
	return check_db_fd(fd, opts, out, BACKEND_UNKNOWN);
    }

    const string & path = *path_ptr;
    if (path.empty()) throw_no_db_to_check();
    struct stat sb;
    if (stat(path.c_str(), &sb) == 0) {
	if (S_ISDIR(sb.st_mode)) {
	    return check_db_dir(path, opts, out);
	}

	if (S_ISREG(sb.st_mode)) {
	    int backend = test_if_single_file_db(sb, path, &fd);
	    if (backend != BACKEND_UNKNOWN) {
		return check_db_fd(fd, opts, out, backend);
	    }
	    // Could be a single table or a stub database file.  Look at the
	    // extension to determine the type.
	    if (endswith(path, ".DB")) {
		// It could also be flint or brass, but we check for those below.
		backend = BACKEND_CHERT;
	    } else if (endswith(path, "." GLASS_TABLE_EXTENSION)) {
		backend = BACKEND_GLASS;
	    } else {
		return check_stub(path, opts, out);
	    }

	    return check_db_table(path, opts, out, backend);
	}

	throw Xapian::DatabaseOpeningError("Not a regular file or directory");
    }

    // The filename passed doesn't exist - see if it's the basename of the
    // table (perhaps with "." after it), so the user can do xapian-check on
    // "foo/termlist" or "foo/termlist." (which you would get from filename
    // completion with older backends).
    string filename = path;
    if (endswith(filename, '.')) {
	filename.resize(filename.size() - 1);
    }

    int backend = BACKEND_UNKNOWN;
    if (stat((filename + ".DB").c_str(), &sb) == 0) {
	// It could also be flint or brass, but we check for those below.
	backend = BACKEND_CHERT;
    } else if (stat((filename + "." GLASS_TABLE_EXTENSION).c_str(), &sb) == 0) {
	backend = BACKEND_GLASS;
    } else {
	throw_no_db_to_check();
    }

    return check_db_table(path, opts, out, backend);
}

}
