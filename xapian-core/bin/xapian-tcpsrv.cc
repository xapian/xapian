/* xapian-tcpsrv.cc: tcp daemon for use with Xapian's remote backend
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2004,2006,2007 Olly Betts
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

#include "safeerrno.h"

#include <iostream>
#include <string>

#include "gnu_getopt.h"

#include <xapian/error.h>
#include <xapian/enquire.h>
#include "tcpserver.h"

using namespace std;

static void register_user_weighting_schemes(TcpServer &server) {
    (void)server; // Suppress "unused parameter" warning.
    // If you have defined your own weighting scheme, register it here
    // like so:
    // server.register_weighting_scheme(FooWeight());
}

const int MSECS_IDLE_TIMEOUT_DEFAULT = 60000;
const int MSECS_ACTIVE_TIMEOUT_DEFAULT = 15000;

#define PROG_NAME "xapian-tcpsrv"
#define PROG_DESC "TCP daemon for use with Xapian's remote backend"

#define OPT_HELP 1
#define OPT_VERSION 2

static const struct option opts[] = {
    {"port",		required_argument,	0, 'p'},
    {"active-timeout",	required_argument,	0, 'a'},
    {"idle-timeout",	required_argument,	0, 'i'},
    {"timeout",		required_argument,	0, 't'},
    {"one-shot",	no_argument,		0, 'o'},
    {"quiet",		no_argument,		0, 'q'},
    {"writable",	no_argument,		0, 'w'},
    {"help",		no_argument,		0, OPT_HELP},
    {"version",		no_argument,		0, OPT_VERSION},
    {NULL, 0, 0, 0}
};

static void show_usage() {
    cout << "Usage: "PROG_NAME" [OPTIONS] DATABASE_DIRECTORY...\n\n"
"Options:\n"
"  --port PORTNUM          listen on port PORTNUM for connections (no default)\n"
"  --idle-timeout MSECS    set timeout for idle connections (default " << MSECS_IDLE_TIMEOUT_DEFAULT << "ms)\n"
"  --active-timeout MSECS  set timeout for active connections (default " << MSECS_ACTIVE_TIMEOUT_DEFAULT << "ms)\n"
"  --timeout MSECS         set both timeout values\n"
"  --one-shot              serve a single connection and exit\n"
"  --quiet                 disable information messages to stdout\n"
"  --writable              allow updates (only one database directory allowed)\n"
"  --help                  display this help and exit\n"
"  --version               output version information and exit" << endl;
}

int main(int argc, char **argv) {
    int port = 0;
    int msecs_active_timeout = MSECS_ACTIVE_TIMEOUT_DEFAULT;
    int msecs_idle_timeout   = MSECS_IDLE_TIMEOUT_DEFAULT;

    bool one_shot = false;
    bool verbose = true;
    bool writable = false;
    bool syntax_error = false;

    int c;
    while ((c = gnu_getopt_long(argc, argv, "p:a:i:t:oq", opts, NULL)) != EOF) {
	switch (c) {
	    case OPT_HELP:
		cout << PROG_NAME" - "PROG_DESC"\n\n";
		show_usage();
		exit(0);
	    case OPT_VERSION:
		cout << PROG_NAME" - "PACKAGE_STRING << endl;
		exit(0);
	    case 'p':
                port = atoi(optarg);
	        break;
	    case 'a':
                msecs_active_timeout = atoi(optarg);
	        break;
            case 'i':
	        msecs_idle_timeout   = atoi(optarg);
	        break;
	    case 't':
		msecs_active_timeout = msecs_idle_timeout = atoi(optarg);
		break;
	    case 'o':
		one_shot = true;
		break;
	    case 'q':
		verbose = false;
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

    if (port <= 0 || port >= 65536) {
	cerr << "Error: must specify a valid port number (between 1 and 65535)."
		" We actually got " << port << endl;
	exit(1);
    }

    if (writable && (argc - optind) != 1) {
	cerr << "Error: only one database directory allowed with '--writable'." << endl;
	exit(1);
    }

    try {
	if (writable) {
	    Xapian::WritableDatabase db(argv[optind], Xapian::DB_CREATE_OR_OPEN);

	    if (verbose)
		cout << "Starting writable server on port " << port << endl;

	    TcpServer server(db, port, msecs_active_timeout,
			     msecs_idle_timeout, verbose);

	    if (verbose)
		cout << "Listening..." << endl;

	    register_user_weighting_schemes(server);

	    if (one_shot) {
		server.run_once();
	    } else {
		server.run();
	    }
	} else {
	    Xapian::Database db;
	    while (argv[optind]) {
		db.add_database(Xapian::Database(argv[optind++]));
	    }

	    if (verbose)
		cout << "Starting server on port " << port << endl;

	    TcpServer server(db, port, msecs_active_timeout,
			     msecs_idle_timeout, verbose);

	    if (verbose)
		cout << "Listening..." << endl;

	    register_user_weighting_schemes(server);

	    if (one_shot) {
		server.run_once();
	    } else {
		server.run();
	    }
	}
    } catch (const Xapian::Error &e) {
	cerr << e.get_type() << ": " << e.get_msg();
	int my_errno = e.get_errno();
	if (my_errno) cerr << " (errno:" << strerror(my_errno) << ')';
	cerr << endl;
	if (my_errno == EADDRINUSE) exit(69); // EX_UNAVAILABLE
	exit(1);
    } catch (const exception &e) {
	cerr << "Caught standard exception: " << e.what() << endl;
	exit(1);
    } catch (...) {
	cerr << "Caught unknown exception" << endl;
	exit(1);
    }
}
