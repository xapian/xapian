/** @file xapian-letor-update.cc
 * @brief Update statistics in user meta-data used by letor module.
 */
/* Copyright (C) 2011 Parth Gupta
 * Copyright (C) 2012 Olly Betts
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
#include <iostream>

#include "gnu_getopt.h"
#include "str.h"

using namespace std;

#define PROG_NAME "xapian-letor-update"
#define PROG_DESC "Update statistics in user meta-data used by letor module"

static void
show_usage()
{
    cout << "Usage: "PROG_NAME" [OPTIONS] 'QUERY'\n"
"  -d, --db=DIRECTORY  database to update stats for\n"
"  -h, --help          display this help and exit\n"
"  -v, --version       output version information and exit\n";
}

int
main(int argc, char **argv)
try {
    const char * opts = "d:hv";
    static const struct option long_opts[] = {
	{ "db",		required_argument, 0, 'd' },
	{ "help",	no_argument, 0, 'h' },
	{ "version",	no_argument, 0, 'v' },
	{ NULL,		0, 0, 0}
    };

    Xapian::WritableDatabase db;
    bool have_db = false;

    int c;
    while ((c = gnu_getopt_long(argc, argv, opts, long_opts, 0)) != -1) {
	switch (c) {
	    case 'd':
		db = Xapian::WritableDatabase(optarg, Xapian::DB_OPEN);
		have_db = true;
		break;
	    case 'v':
		cout << PROG_NAME" - "PACKAGE_STRING << endl;
		exit(0);
	    case 'h':
		cout << PROG_NAME" - "PROG_DESC"\n\n";
		show_usage();
		exit(0);
	    case ':': // missing parameter
	    case '?': // unknown option
		show_usage();
		exit(1);
	}
    }

    if (!have_db || argc - optind != 1) {
	show_usage();
	exit(1);
    }

    // Calculate some extra collection statistics used to calculate features
    // used by Letor, and store them as user metadata.

    Xapian::termcount total_title_len = 0;
    Xapian::TermIterator t;
    for (t = db.allterms_begin("S"); t != db.allterms_end("S"); ++t) {
	total_title_len += db.get_collection_freq(*t);
    }

    Xapian::termcount total_len = db.get_avlength() * db.get_doccount();

    db.set_metadata("collection_len_title", str(total_title_len));
    db.set_metadata("collection_len_body", str(total_len - total_title_len));
    db.set_metadata("collection_len_whole", str(total_len));
    db.commit();
} catch (const Xapian::Error & e) {
    cout << e.get_description() << endl;
    exit(1);
}
