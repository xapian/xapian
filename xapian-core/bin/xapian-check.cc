/** @file
 * @brief Tool to check the consistency of a database or table.
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2011,2012,2014,2016 Olly Betts
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

#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <iostream>

using namespace std;

#define PROG_NAME "xapian-check"
#define PROG_DESC "Check the consistency of a database or table"

static void show_usage() {
    cout << "Usage: " PROG_NAME " DATABASE_DIRECTORY|PATH_TO_BTREE [[F][t][f][b][v][+]]\n\n"
"If a whole database is checked, then additional cross-checks between\n"
"the tables are performed.\n\n"
"The btree(s) is/are always checked - control the output verbosity with:\n"
" F = attempt to fix a broken database (implemented for chert currently)\n"
" t = short tree printing\n"
" f = full tree printing\n"
" b = show free blocks\n"
" v = show stats about B-tree (default)\n"
" + = same as tbv\n"
" e.g. " PROG_NAME " /var/lib/xapian/data/default\n"
"      " PROG_NAME " /var/lib/xapian/data/default/postlist fbv\n";
}

int
main(int argc, char **argv)
{
    if (argc > 1 && argv[1][0] == '-') {
	if (strcmp(argv[1], "--help") == 0) {
	    cout << PROG_NAME " - " PROG_DESC "\n\n";
	    show_usage();
	    exit(0);
	}
	if (strcmp(argv[1], "--version") == 0) {
	    cout << PROG_NAME " - " PACKAGE_STRING "\n";
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
	    case 't': opts |= Xapian::DBCHECK_SHORT_TREE; break;
	    case 'f': opts |= Xapian::DBCHECK_FULL_TREE; break;
	    case 'b': opts |= Xapian::DBCHECK_SHOW_FREELIST; break;
	    case 'v': opts |= Xapian::DBCHECK_SHOW_STATS; break;
	    case '+':
		opts |= Xapian::DBCHECK_SHORT_TREE;
		opts |= Xapian::DBCHECK_SHOW_FREELIST;
		opts |= Xapian::DBCHECK_SHOW_STATS;
		break;
	    case 'F':
		opts |= Xapian::DBCHECK_FIX;
		break;
	    default:
		cerr << "option " << opt_string << " unknown\n";
		cerr << "use t,f,b,v and/or + in the option string\n";
		exit(1);
	}
    }

    try {
	size_t errors = Xapian::Database::check(argv[1], opts, &cout);
	if (errors > 0) {
	    cout << "Total errors found: " << errors << '\n';
	    exit(1);
	}
	cout << "No errors found\n";
    } catch (const Xapian::Error &error) {
	cerr << argv[0] << ": " << error.get_description() << '\n';
	exit(1);
    } catch (...) {
	cerr << argv[0] << ": Unknown exception\n";
	exit(1);
    }
}
