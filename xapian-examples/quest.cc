/* quest.cc - Command line search tool using Xapian::QueryParser.
 *
 * Copyright 2004 Olly Betts
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <config.h>

#include <xapian.h>
#include <xapian/queryparser.h>

#include <iostream>

#include "getopt.h"

using namespace std;

static void
show_help(const char * argv0)
{
    cout << "Usage: " << argv0 << " [OPTIONS] 'QUERY'\n"
	 << "NB: query should be quoted to protect it from the shell.\n"
	 << "\n"
	 << "  -d, --db=DIRECTORY  database to search (multiple databases may be specified)\n"
	 << "  -m, --msize=MSIZE   maximum number of matches to return\n"
	 << "  -h, --help          display this help and exit\n"
	 << "  -v, --version       output version information and exit\n";
}

int
main(int argc, char **argv)
{
    static const struct option long_opts[] = {
	{ "db",		required_argument, 0, 'd' },
	{ "msize",	required_argument, 0, 'm' },
	{ "help",	no_argument, 0, 'h' },
	{ "version",	no_argument, 0, 'v' },
	{ NULL,		0, 0, 0}
    };

    int msize = 10;

    bool have_database = false;

    try {
	Xapian::Database db;

	int c;
	while ((c = getopt_long(argc, argv, "hvm:d:", long_opts, 0)) != EOF) {
	    switch (c) {
		case 'm':
		    msize = atoi(optarg);
		    break;
		case 'd':
		    db.add_database(Xapian::Auto::open(optarg));
		    have_database = true;
		    break;
		case 'v':
		    cout << argv[0] << " (" << PACKAGE_NAME << ") "
			 << VERSION << endl;
		    exit(0);
		case 'h':
		    show_help(argv[0]);
		    exit(0);
		case ':': // missing parameter
		case '?': // unknown option
		    show_help(argv[0]);
		    exit(1);
	    }
	}

	if (!have_database || argc - optind != 1) {
	    show_help(argv[0]);
	    exit(1);
	}

	Xapian::Enquire enquire(db);

	try {
	    Xapian::QueryParser parser;
	    parser.set_database(db);
	    parser.set_default_op(Xapian::Query::OP_OR);
	    // FIXME: pass Stopper instead of NULL...
	    parser.set_stemming_options("english", false, NULL);
	    enquire.set_query(parser.parse_query(argv[optind]));
	} catch (const char * error_msg) {
	    cout << "Couldn't parse query: " << error_msg << endl;
	    exit(1);
	}

	Xapian::MSet mset = enquire.get_mset(0, msize);
	for (Xapian::MSetIterator i = mset.begin(); i != mset.end(); i++) {
	    Xapian::Document doc = i.get_document();
	    string data = doc.get_data();
	    cout << *i << " [" << i.get_percent() << "%]\n" << data << "\n";
	}
	cout << flush;
    } catch (const Xapian::Error & err) {
	cout << err.get_msg() << endl;
	exit(1);
    }
}
