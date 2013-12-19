/** @file dbcheck.cc
 * @brief Check the consistency of a database or table.
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2011,2012,2013 Olly Betts
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
#include "xapian/dbfactory.h"
#include "xapian/error.h"

#ifdef XAPIAN_HAS_BRASS_BACKEND
#include "brass/brass_database.h"
#include "brass/brass_dbcheck.h"
#include "brass/brass_types.h"
#endif
#ifdef XAPIAN_HAS_CHERT_BACKEND
#include "chert/chert_database.h"
#include "chert/chert_dbcheck.h"
#include "chert/chert_types.h"
#include "chert/chert_version.h"
#endif

#include "filetests.h"
#include "stringutils.h"

#include <stdexcept>
#include <iostream>

using namespace std;

// FIXME: We don't currently cross-check wdf between postlist and termlist.
// It's hard to see how to efficiently.  We do cross-check doclens, but that
// "only" requires (4 * last_docid()) bytes.

#if defined XAPIAN_HAS_BRASS_BACKEND || defined XAPIAN_HAS_CHERT_BACKEND
static void
reserve_doclens(vector<Xapian::termcount>& doclens, Xapian::docid last_docid,
		ostream & out)
{
    if (last_docid >= 0x40000000ul / sizeof(Xapian::termcount)) {
	// The memory block needed by the vector would be >= 1GB.
	out << "Cross-checking document lengths between the postlist and "
	       "termlist tables would use more than 1GB of memory, so "
	       "skipping that check" << endl;
	return;
    }
    try {
	doclens.reserve(last_docid + 1);
    } catch (const std::bad_alloc &) {
	// Failed to allocate the required memory.
	out << "Couldn't allocate enough memory for cross-checking document "
	       "lengths between the postlist and termlist tables, so "
	       "skipping that check" << endl;
    } catch (const std::length_error &) {
	// There are too many elements for the vector to handle!
	out << "Couldn't allocate enough elements for cross-checking document "
	       "lengths between the postlist and termlist tables, so "
	       "skipping that check" << endl;
    }
}
#endif

namespace Xapian {

size_t
Database::check(const string & path, int opts, std::ostream &out)
{
    vector<Xapian::termcount> doclens;
    size_t errors = 0;
    struct stat sb;
    if (stat((path + "/iamchert").c_str(), &sb) == 0) {
#ifndef XAPIAN_HAS_CHERT_BACKEND
	(void)opts;
	(void)out;
	throw Xapian::FeatureUnavailableError("Chert database support isn't enabled");
#else
	// Check a whole chert database directory.
	// If we can't read the last docid, set it to its maximum value
	// to suppress errors.
	Xapian::docid db_last_docid = static_cast<Xapian::docid>(-1);
	chert_revision_number_t rev;
	chert_revision_number_t * rev_ptr = NULL;
	try {
	    Xapian::Database db = Xapian::Chert::open(path);
	    db_last_docid = db.get_lastdocid();
	    reserve_doclens(doclens, db_last_docid, out);
	    ChertDatabase * chert_db =
		static_cast<ChertDatabase*>(db.internal[0].get());
	    rev = chert_db->get_revision_number();
	    rev_ptr = &rev;
	} catch (const Xapian::Error & e) {
	    // Ignore so we can check a database too broken to open.
	    out << "Database couldn't be opened for reading: "
		<< e.get_description()
		<< "\nContinuing check anyway" << endl;
	    ++errors;
	}

	size_t pre_table_check_errors = errors;

	// This is a chert directory so try to check all the btrees.
	// Note: it's important to check termlist before postlist so
	// that we can cross-check the document lengths.
	const char * tables[] = {
	    "record", "termlist", "postlist", "position",
	    "spelling", "synonym"
	};
	for (const char **t = tables;
	     t != tables + sizeof(tables)/sizeof(tables[0]); ++t) {
	    string table(path);
	    table += '/';
	    table += *t;
	    out << *t << ":\n";
	    if (strcmp(*t, "record") != 0 && strcmp(*t, "postlist") != 0) {
		// Other tables are created lazily, so may not exist.
		if (!file_exists(table + ".DB")) {
		    if (strcmp(*t, "termlist") == 0) {
			out << "Not present.\n";
		    } else {
			out << "Lazily created, and not yet used.\n";
		    }
		    out << endl;
		    continue;
		}
	    }
	    errors += check_chert_table(*t, table, rev_ptr, opts, doclens,
					db_last_docid, out);
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
#endif
    } else if (stat((path + "/iambrass").c_str(), &sb) == 0) {
#ifndef XAPIAN_HAS_BRASS_BACKEND
	(void)opts;
	(void)out;
	throw Xapian::FeatureUnavailableError("Brass database support isn't enabled");
#else
	// Check a whole brass database directory.
	// If we can't read the last docid, set it to its maximum value
	// to suppress errors.
	Xapian::docid db_last_docid = static_cast<Xapian::docid>(-1);
	brass_revision_number_t rev;
	brass_revision_number_t * rev_ptr = NULL;
	try {
	    Xapian::Database db = Xapian::Brass::open(path);
	    db_last_docid = db.get_lastdocid();
	    reserve_doclens(doclens, db_last_docid, out);
	    BrassDatabase * brass_db =
		static_cast<BrassDatabase*>(db.internal[0].get());
	    rev = brass_db->get_revision_number();
	    rev_ptr = &rev;
	} catch (const Xapian::Error & e) {
	    // Ignore so we can check a database too broken to open.
	    out << "Database couldn't be opened for reading: "
		<< e.get_description()
		<< "\nContinuing check anyway" << endl;
	    ++errors;
	}
	// This is a brass directory so try to check all the btrees.
	// Note: it's important to check termlist before postlist so
	// that we can cross-check the document lengths.
	const char * tables[] = {
	    "record", "termlist", "postlist", "position",
	    "spelling", "synonym"
	};
	for (const char **t = tables;
	     t != tables + sizeof(tables)/sizeof(tables[0]); ++t) {
	    string table(path);
	    table += '/';
	    table += *t;
	    out << *t << ":\n";
	    if (strcmp(*t, "record") != 0 && strcmp(*t, "postlist") != 0) {
		// Other tables are created lazily, so may not exist.
		if (!file_exists(table + ".DB")) {
		    if (strcmp(*t, "termlist") == 0) {
			out << "Not present.\n";
		    } else {
			out << "Lazily created, and not yet used.\n";
		    }
		    out << endl;
		    continue;
		}
	    }
	    errors += check_brass_table(*t, table, rev_ptr, opts, doclens,
					db_last_docid, out);
	}
#endif
    } else {
	if (stat((path + "/iamflint").c_str(), &sb) == 0) {
	    // Flint is no longer supported as of Xapian 1.3.0.
	    throw Xapian::FeatureUnavailableError("Flint database support was removed in Xapian 1.3.0");
	}
	if (stat((path + "/record_DB").c_str(), &sb) == 0) {
	    // Quartz is no longer supported as of Xapian 1.1.0.
	    throw Xapian::FeatureUnavailableError("Quartz database support was removed in Xapian 1.1.0");
	}
	// Just check a single Btree.  If it ends with "." or ".DB"
	// already, trim that so the user can do xapian-check on
	// "foo", "foo.", or "foo.DB".
	string filename = path;
	if (endswith(filename, '.'))
	    filename.resize(filename.size() - 1);
	else if (endswith(filename, ".DB"))
	    filename.resize(filename.size() - 3);

	size_t p = filename.find_last_of('/');
#if defined __WIN32__ || defined __EMX__
	if (p == string::npos) p = 0;
	p = filename.find_last_of('\\', p);
#endif
	if (p == string::npos) p = 0; else ++p;

	string dir(filename, 0, p);

	string tablename;
	while (p != filename.size()) {
	    tablename += tolower(static_cast<unsigned char>(filename[p++]));
	}

	// If we're passed a "naked" table (with no accompanying files)
	// assume it is chert.
	if (file_exists(dir + "iambrass")) {
#ifndef XAPIAN_HAS_BRASS_BACKEND
	    throw Xapian::FeatureUnavailableError("Brass database support isn't enabled");
#else
	    // Set the last docid to its maximum value to suppress errors.
	    Xapian::docid db_last_docid = static_cast<Xapian::docid>(-1);
	    errors = check_brass_table(tablename.c_str(), filename, NULL, opts,
				       doclens, db_last_docid, out);
#endif
	} else if (file_exists(dir + "iamflint")) {
	    // Flint is no longer supported as of Xapian 1.3.0.
	    throw Xapian::FeatureUnavailableError("Flint database support was removed in Xapian 1.3.0");
	} else {
#ifndef XAPIAN_HAS_CHERT_BACKEND
	    throw Xapian::FeatureUnavailableError("Chert database support isn't enabled");
#else
	    // Set the last docid to its maximum value to suppress errors.
	    Xapian::docid db_last_docid = static_cast<Xapian::docid>(-1);
	    errors = check_chert_table(tablename.c_str(), filename, NULL, opts,
				       doclens, db_last_docid, out);
#endif
	}
    }
    return errors;
}

size_t
Database::check(const string & path, int opts)
{
    return check(path, opts, cout);
}

}
