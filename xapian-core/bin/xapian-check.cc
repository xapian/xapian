/** @file xapian-check.cc
 * @brief Check the consistency of a database or table.
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2013 Olly Betts
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

#include "xapian-check-brass.h"
#include "xapian-check-chert.h"
#include "xapian-check-flint.h"

#ifdef XAPIAN_HAS_BRASS_BACKEND
#include "backends/brass/brass_database.h"
#include "backends/brass/brass_types.h"
#endif
#ifdef XAPIAN_HAS_CHERT_BACKEND
#include "backends/chert/chert_database.h"
#include "backends/chert/chert_types.h"
#endif
#ifdef XAPIAN_HAS_FLINT_BACKEND
#include "backends/flint/flint_database.h"
#include "backends/flint/flint_types.h"
#endif

#include "chert_check.h" // For OPT_SHORT_TREE, etc.
#include "stringutils.h"
#include "utils.h"

#include <stdexcept>
#include <iostream>

using namespace std;

#define PROG_NAME "xapian-check"
#define PROG_DESC "Check the consistency of a database or table"

// FIXME: We don't currently cross-check wdf between postlist and termlist.
// It's hard to see how to efficiently.  We do cross-check doclens, but that
// "only" requires (4 * last_docid()) bytes.

static void show_usage() {
    cout << "Usage: "PROG_NAME" <database directory>|<path to btree and prefix> [[t][f][b][v][+]]\n\n"
"If a whole database is checked, then additional cross-checks between\n"
"the tables are performed.\n\n"
"The btree(s) is/are always checked - control the output verbosity with:\n"
" t = short tree printing\n"
" f = full tree printing\n"
" b = show bitmap\n"
" v = show stats about B-tree (default)\n"
" + = same as tbv\n"
" e.g. "PROG_NAME" /var/lib/xapian/data/default\n"
"      "PROG_NAME" /var/lib/xapian/data/default/postlist fbv" << endl;
}

static void
reserve_doclens(vector<Xapian::termcount>& doclens, Xapian::docid last_docid)
{
    if (last_docid >= 0x40000000ul / sizeof(Xapian::termcount)) {
	// The memory block needed by the vector would be >= 1GB.
	cout << "Cross-checking document lengths between the postlist and "
		"termlist tables would use more than 1GB of memory, so "
		"skipping that check" << endl;
	return;
    }
    try {
	doclens.reserve(last_docid + 1);
    } catch (const std::bad_alloc &) {
	// Failed to allocate the required memory.
	cout << "Couldn't allocate enough memory for cross-checking document "
		"lengths between the postlist and termlist tables, so "
		"skipping that check" << endl;
    } catch (const std::length_error &) {
	// There are too many elements for the vector to handle!
	cout << "Couldn't allocate enough elements for cross-checking document "
		"lengths between the postlist and termlist tables, so "
		"skipping that check" << endl;
    }
}

int
main(int argc, char **argv)
{
    if (argc > 1 && argv[1][0] == '-') {
	if (strcmp(argv[1], "--help") == 0) {
	    cout << PROG_NAME" - "PROG_DESC"\n\n";
	    show_usage();
	    exit(0);
	}
	if (strcmp(argv[1], "--version") == 0) {
	    cout << PROG_NAME" - "PACKAGE_STRING << endl;
	    exit(0);
	}
    }
    if (argc < 2 || argc > 3) {
	show_usage();
	exit(1);
    }

    int opts = 0;
    const char * opt_string = argv[2];
    if (!opt_string) opt_string = "v";
    for (const char *p = opt_string; *p; ++p) {
	switch (*p) {
	    case 't': opts |= OPT_SHORT_TREE; break;
	    case 'f': opts |= OPT_FULL_TREE; break;
	    case 'b': opts |= OPT_SHOW_BITMAP; break;
	    case 'v': opts |= OPT_SHOW_STATS; break;
	    case '+':
		opts |= OPT_SHORT_TREE | OPT_SHOW_BITMAP | OPT_SHOW_STATS;
		break;
	    default:
		cerr << "option " << opt_string << " unknown\n";
		cerr << "use t,f,b,v and/or + in the option string\n";
		exit(1);
	}
    }

    try {
	vector<Xapian::termcount> doclens;
	size_t errors = 0;
	struct stat sb;
	string dir(argv[1]);
	if (stat((dir + "/iamflint").c_str(), &sb) == 0) {
#ifndef XAPIAN_HAS_FLINT_BACKEND
	    throw "Flint database support isn't enabled";
#else
	    // Check a whole flint database directory.
	    flint_revision_number_t rev;
	    flint_revision_number_t * rev_ptr = NULL;
	    try {
		Xapian::Database db = Xapian::Flint::open(dir);
		Xapian::docid db_last_docid = db.get_lastdocid();
		reserve_doclens(doclens, db_last_docid);
		FlintDatabase * flint_db =
		    static_cast<FlintDatabase*>(db.internal[0].get());
		rev = flint_db->postlist_table.get_open_revision_number();
		rev_ptr = &rev;
	    } catch (const Xapian::Error & e) {
		// Ignore so we can check a database too broken to open.
		cout << "Database couldn't be opened for reading: "
		     << e.get_description()
		     << "\nContinuing check anyway" << endl;
		++errors;
	    }
	    // This is a flint directory so try to check all the btrees.
	    // Note: it's important to check termlist before postlist so
	    // that we can cross-check the document lengths.
	    const char * tables[] = {
		"record", "termlist", "postlist", "position", "value",
		"spelling", "synonym"
	    };
	    for (const char **t = tables;
		 t != tables + sizeof(tables)/sizeof(tables[0]); ++t) {
		string table(dir);
		table += '/';
		table += *t;
		cout << *t << ":\n";
		if (strcmp(*t, "position") == 0 ||
		    strcmp(*t, "value") == 0 ||
		    strcmp(*t, "spelling") == 0 ||
		    strcmp(*t, "synonym") == 0) {
		    // These are created lazily, so may not exist.
		    if (!file_exists(table + ".DB")) {
			cout << "Lazily created, and not yet used.\n" << endl;
			continue;
		    }
		}
		errors += check_flint_table(*t, table, rev_ptr, opts, doclens);
	    }
#endif
	} else if (stat((dir + "/iamchert").c_str(), &sb) == 0) {
#ifndef XAPIAN_HAS_CHERT_BACKEND
	    throw "Chert database support isn't enabled";
#else
	    // Check a whole chert database directory.
	    // If we can't read the last docid, set it to its maximum value
	    // to suppress errors.
	    Xapian::docid db_last_docid = static_cast<Xapian::docid>(-1);
	    chert_revision_number_t rev;
	    chert_revision_number_t * rev_ptr = NULL;
	    try {
		Xapian::Database db = Xapian::Chert::open(dir);
		db_last_docid = db.get_lastdocid();
		ChertDatabase * chert_db =
		    static_cast<ChertDatabase*>(db.internal[0].get());
		rev = chert_db->postlist_table.get_open_revision_number();
		rev_ptr = &rev;
	    } catch (const Xapian::Error & e) {
		// Ignore so we can check a database too broken to open.
		cout << "Database couldn't be opened for reading: "
		     << e.get_description()
		     << "\nContinuing check anyway" << endl;
		++errors;
	    }
	    reserve_doclens(doclens, db_last_docid);
	    // This is a chert directory so try to check all the btrees.
	    // Note: it's important to check termlist before postlist so
	    // that we can cross-check the document lengths.
	    const char * tables[] = {
		"record", "termlist", "postlist", "position",
		"spelling", "synonym"
	    };
	    for (const char **t = tables;
		 t != tables + sizeof(tables)/sizeof(tables[0]); ++t) {
		string table(dir);
		table += '/';
		table += *t;
		cout << *t << ":\n";
		if (strcmp(*t, "record") != 0 && strcmp(*t, "postlist") != 0) {
		    // Other tables are created lazily, so may not exist.
		    if (!file_exists(table + ".DB")) {
			if (strcmp(*t, "termlist") == 0) {
			    cout << "Not present.\n";
			} else {
			    cout << "Lazily created, and not yet used.\n";
			}
			cout << endl;
			continue;
		    }
		}
		errors += check_chert_table(*t, table, rev_ptr, opts, doclens,
					    db_last_docid);
	    }
#endif
	} else if (stat((dir + "/iambrass").c_str(), &sb) == 0) {
#ifndef XAPIAN_HAS_BRASS_BACKEND
	    throw "Brass database support isn't enabled";
#else
	    // Check a whole brass database directory.
	    // If we can't read the last docid, set it to its maximum value
	    // to suppress errors.
	    Xapian::docid db_last_docid = static_cast<Xapian::docid>(-1);
	    brass_revision_number_t rev;
	    brass_revision_number_t * rev_ptr = NULL;
	    try {
		Xapian::Database db = Xapian::Brass::open(dir);
		db_last_docid = db.get_lastdocid();
		BrassDatabase * brass_db =
		    static_cast<BrassDatabase*>(db.internal[0].get());
		rev = brass_db->postlist_table.get_open_revision_number();
		rev_ptr = &rev;
	    } catch (const Xapian::Error & e) {
		// Ignore so we can check a database too broken to open.
		cout << "Database couldn't be opened for reading: "
		     << e.get_description()
		     << "\nContinuing check anyway" << endl;
		++errors;
	    }
	    reserve_doclens(doclens, db_last_docid);
	    // This is a brass directory so try to check all the btrees.
	    // Note: it's important to check termlist before postlist so
	    // that we can cross-check the document lengths.
	    const char * tables[] = {
		"record", "termlist", "postlist", "position",
		"spelling", "synonym"
	    };
	    for (const char **t = tables;
		 t != tables + sizeof(tables)/sizeof(tables[0]); ++t) {
		string table(dir);
		table += '/';
		table += *t;
		cout << *t << ":\n";
		if (strcmp(*t, "record") != 0 && strcmp(*t, "postlist") != 0) {
		    // Other tables are created lazily, so may not exist.
		    if (!file_exists(table + ".DB")) {
			if (strcmp(*t, "termlist") == 0) {
			    cout << "Not present.\n";
			} else {
			    cout << "Lazily created, and not yet used.\n";
			}
			cout << endl;
			continue;
		    }
		}
		errors += check_brass_table(*t, table, rev_ptr, opts, doclens,
					    db_last_docid);
	    }
#endif
	} else {
	    if (stat((dir + "/record_DB").c_str(), &sb) == 0) {
		// Quartz is no longer supported as of Xapian 1.1.0.
		cerr << argv[0] << ": '" << dir << "' is a quartz database.\n"
			"Support for quartz was dropped in Xapian 1.1.0" << endl;
		exit(1);
	    }
	    // Just check a single Btree.  If it ends with "." or ".DB"
	    // already, trim that so the user can do xapian-check on
	    // "foo", "foo.", or "foo.DB".
	    string filename = dir;
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

	    string path(filename, 0, p);

	    string tablename;
	    while (p != filename.size()) {
		tablename += tolower(static_cast<unsigned char>(filename[p++]));
	    }

	    // If we're passed a "naked" table (with no accompanying files)
	    // assume it is chert.
	    if (file_exists(path + "iamflint")) {
#ifndef XAPIAN_HAS_FLINT_BACKEND
		throw "Flint database support isn't enabled";
#else
		errors = check_flint_table(tablename.c_str(), filename, NULL,
					   opts, doclens);
#endif
	    } else if (file_exists(path + "iambrass")) {
#ifndef XAPIAN_HAS_BRASS_BACKEND
		throw "Brass database support isn't enabled";
#else
		// Set the last docid to its maximum value to suppress errors.
		Xapian::docid db_last_docid = static_cast<Xapian::docid>(-1);
		errors = check_brass_table(tablename.c_str(), filename, NULL, 
					   opts, doclens, db_last_docid);
#endif
	    } else {
#ifndef XAPIAN_HAS_CHERT_BACKEND
		throw "Chert database support isn't enabled";
#else
		// Set the last docid to its maximum value to suppress errors.
		Xapian::docid db_last_docid = static_cast<Xapian::docid>(-1);
		errors = check_chert_table(tablename.c_str(), filename, NULL,
					   opts, doclens, db_last_docid);
#endif
	    }
	}
	if (errors > 0) {
	    cout << "Total errors found: " << errors << endl;
	    exit(1);
	}
	cout << "No errors found" << endl;
    } catch (const char *error) {
	cerr << argv[0] << ": " << error << endl;
	exit(1);
    } catch (const Xapian::Error &error) {
	cerr << argv[0] << ": " << error.get_description() << endl;
	exit(1);
    } catch (...) {
	cerr << argv[0] << ": Unknown exception" << endl;
	exit(1);
    }
}
