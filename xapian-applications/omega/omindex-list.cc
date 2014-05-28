/** @file omindex-list.cc
 * @brief List URLs of documents indexed by omindex
 */
/* Copyright 2014 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include <xapian.h>

#include <cstdlib>
#include <string>
#include <iostream>

#include "gnu_getopt.h"
#include "hashterm.h"
#include "common/stringutils.h"

using namespace std;

#define PROG_NAME "omindex-list"
#define PROG_DESC "List URLs of documents indexed by omindex"

#define OPT_HELP 1
#define OPT_VERSION 2

static void show_usage() {
    cout << "Usage: "PROG_NAME" [OPTIONS] DATABASE...\n\n"
"Options:\n"
"  --help           display this help and exit\n"
"  --version        output version information and exit" << endl;
}

int
main(int argc, char **argv)
try {
    const struct option long_opts[] = {
	{"help",	no_argument, 0, OPT_HELP},
	{"version",	no_argument, 0, OPT_VERSION},
	{NULL,		0, 0, 0}
    };

    int c;
    while ((c = gnu_getopt_long(argc, argv, "", long_opts, 0)) != -1) {
	switch (c) {
	    case OPT_HELP:
		cout << PROG_NAME" - "PROG_DESC"\n\n";
		show_usage();
		exit(0);
	    case OPT_VERSION:
		cout << PROG_NAME" - "PACKAGE_STRING << endl;
		exit(0);
	    default:
		show_usage();
		exit(1);
	}
    }

    if (argc - optind < 1) {
	show_usage();
	exit(1);
    }

    Xapian::Database db;
    while (argv[optind]) {
	db.add_database(Xapian::Database(argv[optind++]));
    }

    for (Xapian::TermIterator t = db.allterms_begin("U");
	 t != db.allterms_end("U");
	 ++t) {
	const string & term = *t;
	string url;
	if (term.size() < MAX_SAFE_TERM_LENGTH) {
	    url.assign(term, 1, string::npos);
	} else {
	    Xapian::PostingIterator p = db.postlist_begin(term);
	    if (p == db.postlist_end(term)) {
		cerr << "Unique term '" << term << "' has no postings!" << endl;
		continue;
	    }
	    Xapian::docid did = *p;
	    ++p;
	    if (p != db.postlist_end(term)) {
		cerr << "warning: Unique term '" << term << "' occurs "
		     << t.get_termfreq() << " times!" << endl;
	    }
	    const string & data = db.get_document(did).get_data();
	    size_t start;
	    if (startswith(data, "url=")) {
		start = CONST_STRLEN("url=");
	    } else {
		start = data.find("\nurl=");
		if (start == string::npos) {
		    cerr << "No 'url' field in document data for unique term '"
			 << term << "'" << endl;
		    continue;
		}
		start += CONST_STRLEN("\nurl=");
	    }
	    url.assign(data, start, data.find('\n', start) - start);
	}
	cout << url << endl;
    }
} catch (const Xapian::Error &error) {
    cerr << argv[0] << ": " << error.get_description() << endl;
    exit(1);
}
