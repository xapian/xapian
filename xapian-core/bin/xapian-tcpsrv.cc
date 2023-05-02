/** @file
 * @brief tcp daemon for use with Xapian's remote backend
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2004,2006,2007,2008,2009,2010,2011,2013,2015,2023 Olly Betts
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

#include <cstdlib>

#include <iostream>
#include <string>

#include "gnu_getopt.h"

#include "xapian/constants.h"
#include "xapian/error.h"
#include "net/remotetcpserver.h"
#include "net/remoteserver.h"
#include "stringutils.h"

using namespace std;

static void register_user_weighting_schemes(RemoteTcpServer &server) {
    Xapian::Registry reg;
    // If you have defined your own weighting scheme, register it here
    // like so:
    // reg.register_weighting_scheme(FooWeight());
    server.set_registry(reg);
}

#define MSECS_IDLE_TIMEOUT_DEFAULT 60000
#define MSECS_ACTIVE_TIMEOUT_DEFAULT 15000

#define PROG_NAME "xapian-tcpsrv"
#define PROG_DESC "TCP daemon for use with Xapian's remote backend"

#define OPT_HELP 1
#define OPT_VERSION 2

static const char * opts = "I:p:a:i:t:oqw";
static const struct option long_opts[] = {
    {"interface",	required_argument,	0, 'I'},
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
    cout << "Usage: " PROG_NAME " [OPTIONS] DATABASE_PATH...\n\n"
"Options:\n"
"  --port PORTNUM          listen on port PORTNUM for connections (no default)\n"
"  --interface ADDRESS     listen on the interface associated with name or\n"
"                          address ADDRESS (default is all interfaces)\n"
"  --idle-timeout MSECS    set timeout for idle connections (default: "
    STRINGIZE(MSECS_IDLE_TIMEOUT_DEFAULT) "ms)\n"
"  --active-timeout MSECS  set timeout for active connections (default: "
    STRINGIZE(MSECS_ACTIVE_TIMEOUT_DEFAULT) "ms)\n"
"  --timeout MSECS         set both timeout values\n"
"  --one-shot              serve a single connection and exit\n"
"  --quiet                 disable information messages to stdout\n"
"  --writable              allow updates\n"
"  --help                  display this help and exit\n"
"  --version               output version information and exit\n";
}

int main(int argc, char **argv) {
    string host;
    int port = 0;
    double active_timeout = MSECS_ACTIVE_TIMEOUT_DEFAULT * 1e-3;
    double idle_timeout   = MSECS_IDLE_TIMEOUT_DEFAULT * 1e-3;

    bool one_shot = false;
    bool verbose = true;
    bool writable = false;
    bool syntax_error = false;

    int c;
    while ((c = gnu_getopt_long(argc, argv, opts, long_opts, NULL)) != -1) {
	switch (c) {
	    case OPT_HELP:
		cout << PROG_NAME " - " PROG_DESC "\n\n";
		show_usage();
		exit(0);
	    case OPT_VERSION:
		cout << PROG_NAME " - " PACKAGE_STRING "\n";
		exit(0);
	    case 'I':
		host.assign(optarg);
		break;
	    case 'p':
		port = atoi(optarg);
		if (port <= 0 || port >= 65536) {
		    cerr << "Error: must specify a valid port number "
			    "(between 1 and 65535). "
			    "We actually got " << port << '\n';
		    exit(1);
		}
		break;
	    case 'a':
		active_timeout = atoi(optarg) * 1e-3;
		break;
	    case 'i':
		idle_timeout = atoi(optarg) * 1e-3;
		break;
	    case 't':
		active_timeout = idle_timeout = atoi(optarg) * 1e-3;
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

    if (port == 0) {
	cerr << "Error: You must specify a port with --port\n";
	exit(1);
    }

    vector<string> dbnames(argv + optind, argv + argc);
    try {
	if (!one_shot) {
	    // Try to open the database(s) so we report problems now instead of
	    // waiting for the first connection.
	    for (auto& dbname : dbnames) {
		if (writable) {
		    Xapian::WritableDatabase db(dbname,
						Xapian::DB_CREATE_OR_OPEN);
		} else {
		    Xapian::Database db(dbname);
		}
	    }
	}

	if (verbose) {
	    cout << "Starting";
	    if (writable)
		cout << " writable";
	    cout << " server on";
	    if (!host.empty())
		cout << " host " << host << ",";
	    cout << " port " << port << '\n';
	}

	RemoteTcpServer server(dbnames, host, port, active_timeout,
			       idle_timeout, writable, verbose);

	if (verbose)
	    cout << "Listening...\n" << flush;

	register_user_weighting_schemes(server);

	if (one_shot) {
	    server.run_once();
	} else {
	    server.run();
	}
    } catch (const Xapian::Error &e) {
	cerr << e.get_description() << '\n';
	exit(1);
    } catch (const exception &e) {
	cerr << "Caught standard exception: " << e.what() << '\n';
	exit(1);
    } catch (...) {
	cerr << "Caught unknown exception\n";
	exit(1);
    }
}
