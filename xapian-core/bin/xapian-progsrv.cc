/* xapian-progsrv.cc: Remote server to be used with ProgClient
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2006 Olly Betts
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
#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <xapian/error.h>
#include "gnu_getopt.h"
#include "remoteserver.h"
#include "serialise.h"

using namespace std;

#define PROG_NAME "xapian-progsrv"
#define PROG_DESC "Piped server for use with Xapian's remote backend"

#define OPT_HELP 1
#define OPT_VERSION 2

static const struct option opts[] = {
    {"timeout",		required_argument,	0, 't'},
    {"writable",	no_argument,		0, 'w'},
    {"help",		no_argument,		0, OPT_HELP},
    {"version",		no_argument,		0, OPT_VERSION},
    {NULL, 0, 0, 0}
};

static void show_usage() {
    cout << "Usage: "PROG_NAME" [OPTIONS] DATABASE_DIRECTORY...\n\n"
"Options:\n"
"  --timeout MSECS         set timeout\n"
"  --writable              allow updates (only one database directory allowed)\n"
"  --help                  display this help and exit\n"
"  --version               output version information and exit" << endl;
}

int main(int argc, char **argv) {
    /* variables needed in both try/catch blocks */
    Xapian::WritableDatabase wdb;
    Xapian::Database dbs;
    unsigned int timeout = 60000;
    bool writable = false;
    bool syntax_error = false;

    int c;
    while ((c = gnu_getopt_long(argc, argv, "t:", opts, NULL)) != EOF) {
	switch (c) {
	    case OPT_HELP:
		cout << PROG_NAME" - "PROG_DESC"\n\n";
		show_usage();
		exit(0);
	    case OPT_VERSION:
		cout << PROG_NAME" - "PACKAGE_STRING << endl;
		exit(0);
	    case 't':
		timeout = atoi(optarg);
		break;
	    case 'w':
		writable = true;
		break;
	    default:
		syntax_error = true;
	}
    }

    if (syntax_error || argv[optind] == NULL) {
	show_usage();
	exit(1);
    }

    if (writable && (argc - optind) != 1) {
	cerr << "Error: only one database directory allowed with '--writable'." << endl;
	exit(1);
    }

    /* Trap exceptions related to setting up the database */
    try {
	if (writable) {
	    wdb = Xapian::WritableDatabase(argv[optind], Xapian::DB_CREATE_OR_OPEN);
	} else {
	    while (argv[optind]) {
		dbs.add_database(Xapian::Database(argv[optind++]));
	    }
	}
    } catch (const Xapian::Error &e) {
	// FIXME: we shouldn't build messages by hand here.
	string msg = serialise_error(e);
	cout << char(REPLY_EXCEPTION) << encode_length(msg.size()) << msg << flush;
    } catch (...) {
	// FIXME: we shouldn't build messages by hand here.
	cout << char(REPLY_EXCEPTION) << encode_length(0) << flush;
    }

    /* Catch exceptions from running the server, but don't pass them
     * on to the remote end, as the RemoteServer will do that itself.
     */
    try {
	if (writable) {
	    RemoteServer server(&wdb, 0, 1, timeout, timeout);
	    // If you have defined your own weighting scheme, register it here
	    // like so:
	    // server.register_weighting_scheme(FooWeight());

	    server.run();
	} else {
	    RemoteServer server(&dbs, 0, 1, timeout, timeout);
	    // If you have defined your own weighting scheme, register it here
	    // like so:
	    // server.register_weighting_scheme(FooWeight());

	    server.run();
	}
    } catch (...) {
    }
}
