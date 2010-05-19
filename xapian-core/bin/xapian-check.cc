/** @file xapian-check.cc
 * @brief Check the consistency of a database or table.
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010 Olly Betts
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

#include "xapian-check-brass.h"
#include "xapian-check-chert.h"
#include "xapian-check-flint.h"

#include "brass_version.h"
#include "chert_check.h" // For OPT_SHORT_TREE, etc.
#include "stringutils.h"
#include "utils.h"

#include <xapian.h>

#include <iostream>

using namespace std;

#define PROG_NAME "xapian-check"
#define PROG_DESC "Check the consistency of a database or table"

// FIXME: We don't currently cross-check wdf between postlist and termlist.
// It's hard to see how to efficiently.  We do cross-check doclens, but that
// "only" requires (4 * last_docid()) bytes.

static void show_usage() {
    cout << "Usage: "PROG_NAME" <database directory>|<path to btree and prefix> [[t][f][b][v][+]]\n\n"
"The btree(s) is/are always checked - control the output verbosity with:\n"
" t = short tree printing\n"
" f = full tree printing\n"
" b = show bitmap\n"
" v = show stats about B-tree (default)\n"
" + = same as tbv\n"
" e.g. "PROG_NAME" /var/lib/xapian/data/default\n"
"      "PROG_NAME" /var/lib/xapian/data/default/postlist fbv" << endl;
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
	    try {
		Xapian::Database db = Xapian::Flint::open(dir);
		doclens.reserve(db.get_lastdocid() + 1);
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
		errors += check_flint_table(*t, table, opts, doclens);
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
	    try {
		Xapian::Database db = Xapian::Chert::open(dir);
		db_last_docid = db.get_lastdocid();
		doclens.reserve(db_last_docid + 1);
	    } catch (const Xapian::Error & e) {
		// Ignore so we can check a database too broken to open.
		cout << "Database couldn't be opened for reading: "
		     << e.get_description()
		     << "\nContinuing check anyway" << endl;
		++errors;
	    }
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
		errors += check_chert_table(*t, table, opts, doclens,
					    db_last_docid);
	    }
#endif
	} else if (stat((dir + "/postlist."BRASS_TABLE_EXTENSION).c_str(),
			&sb) == 0) {
#ifndef XAPIAN_HAS_BRASS_BACKEND
	    throw "Brass database support isn't enabled";
#else
	    // Check a whole brass database directory.
	    // If we can't read the last docid, set it to its maximum value
	    // to suppress errors.
	    Xapian::docid db_last_docid = static_cast<Xapian::docid>(-1);
	    try {
		Xapian::Database db = Xapian::Brass::open(dir);
		db_last_docid = db.get_lastdocid();
		doclens.reserve(db_last_docid + 1);
	    } catch (const Xapian::Error & e) {
		// Ignore so we can check a database too broken to open.
		cout << "Database couldn't be opened for reading: "
		     << e.get_description()
		     << "\nContinuing check anyway" << endl;
		++errors;
	    }
	    BrassVersion version_file;
	    version_file.open_most_recent(dir);
	    unsigned block_size = version_file.get_block_size();

	    // This is a brass directory so try to check all the tables.
	    // Note: it's important to check termlist before postlist so
	    // that we can cross-check the document lengths.
	    const Brass::table_type table_types[] = {
		Brass::RECORD, Brass::TERMLIST, Brass::POSTLIST,
		Brass::POSITION, Brass::SPELLING, Brass::SYNONYM
	    };
	    const char * tables[] = {
		"record", "termlist", "postlist", "position",
		"spelling", "synonym"
	    };
	    for (int i = 0; i != sizeof(tables)/sizeof(tables[0]); ++i) {
		Brass::table_type type = table_types[i];
		const char * table_name = tables[i];
		string table(dir);
		table += '/';
		table += table_name;
		cout << table_name << ":\n";
		if (type != Brass::POSTLIST) {
		    // Other tables are created lazily, so may not exist.
		    if (!file_exists(table + "."BRASS_TABLE_EXTENSION)) {
			if (type == Brass::TERMLIST) {
			    cout << "Not present.\n";
			} else {
			    cout << "Lazily created, and not yet used.\n";
			}
			cout << endl;
			continue;
		    }
		}
		brass_block_t root = version_file.get_root_block(type);
		errors += check_brass_table(table_name, table, opts, doclens,
					    db_last_docid, root, block_size);
	    }
#endif
	} else {
	    enum { UNKNOWN, NOT_BRASS, FLINT, CHERT, BRASS } backend = UNKNOWN;
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
	    if (endswith(filename, '.')) {
		filename.resize(filename.size() - 1);
	    } else if (endswith(filename, ".DB")) {
		backend = NOT_BRASS;
		filename.resize(filename.size() - 3);
	    } else if (endswith(filename, "."BRASS_TABLE_EXTENSION)) {
		backend = BRASS;
		const size_t ext_len = CONST_STRLEN("."BRASS_TABLE_EXTENSION);
		filename.resize(filename.size() - ext_len);
	    }

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

	    if (backend == UNKNOWN) {
		if (file_exists(filename + "."BRASS_TABLE_EXTENSION)) {
		    backend = BRASS;
		} else {
		    backend = NOT_BRASS;
		}
	    }

	    if (backend == NOT_BRASS) {
		if (file_exists(path + "iamflint")) {
		    backend = FLINT;
		}
	    }

	    // Set the last docid to its maximum value to suppress errors.
	    Xapian::docid db_last_docid = static_cast<Xapian::docid>(-1);

	    // We shouldn't have (backend == UNKNOWN) at this point.  If we have
	    // (backend == NOT_BRASS) then assume chert.
	    switch (backend) {
		case FLINT:
#ifndef XAPIAN_HAS_FLINT_BACKEND
		    throw "Flint database support isn't enabled";
#else
		    errors = check_flint_table(tablename.c_str(), filename,
					       opts, doclens);
		    break;
#endif
		case BRASS:
#ifndef XAPIAN_HAS_BRASS_BACKEND
		    throw "Brass database support isn't enabled";
#else
		    // FIXME: need to look up the root block.
		    throw "Can't check a single brass table currently";
# if 0
		    errors = check_brass_table(tablename.c_str(), filename,
					       opts, doclens, db_last_docid, 0);
		    break;
# endif
#endif
		default:
#ifndef XAPIAN_HAS_CHERT_BACKEND
		    throw "Chert database support isn't enabled";
#else
		    errors = check_chert_table(tablename.c_str(), filename,
					       opts, doclens, db_last_docid);
		    break;
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
