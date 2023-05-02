/** @file
 * @brief Service database replication requests from clients.
 */
/* Copyright (C) 2008,2011 Olly Betts
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

#include "net/replicatetcpserver.h"

#include <xapian.h>

#include "gnu_getopt.h"

#include <cstdlib>
#include <iostream>

using namespace std;

#define PROG_NAME "xapian-replicate-server"
#define PROG_DESC "Service database replication requests from clients"

#define OPT_HELP 1
#define OPT_VERSION 2

static void show_usage() {
    cout << "Usage: " PROG_NAME " [OPTIONS] DATABASE_PARENT_DIRECTORY\n\n"
"Options:\n"
"  -I, --interface=ADDR  listen on interface ADDR\n"
"  -p, --port=PORT   port to listen on\n"
"  -o, --one-shot    serve a single connection and exit\n"
"  --help            display this help and exit\n"
"  --version         output version information and exit\n";
}

int
main(int argc, char **argv)
{
    const char * opts = "I:p:o";
    static const struct option long_opts[] = {
	{"interface",	required_argument,	0, 'I'},
	{"port",	required_argument,	0, 'p'},
	{"one-shot",	no_argument,		0, 'o'},
	{"help",	no_argument, 0, OPT_HELP},
	{"version",	no_argument, 0, OPT_VERSION},
	{NULL,		0, 0, 0}
    };

    string host;
    int port = 0;

    bool one_shot = false;

    int c;
    while ((c = gnu_getopt_long(argc, argv, opts, long_opts, 0)) != -1) {
	switch (c) {
	    case 'I':
		host.assign(optarg);
		break;
	    case 'p':
		port = atoi(optarg);
		break;
	    case 'o':
		one_shot = true;
		break;
	    case OPT_HELP:
		cout << PROG_NAME " - " PROG_DESC "\n\n";
		show_usage();
		exit(0);
	    case OPT_VERSION:
		cout << PROG_NAME " - " PACKAGE_STRING "\n";
		exit(0);
	    default:
		show_usage();
		exit(1);
	}
    }

    if (argc - optind != 1) {
	show_usage();
	exit(1);
    }

    // Path to the database to create/update.
    string dbpath(argv[optind]);

    try {
	ReplicateTcpServer server(host, port, dbpath);
	if (one_shot) {
	    server.run_once();
	} else {
	    server.run();
	}
    } catch (const Xapian::Error &error) {
	cerr << argv[0] << ": " << error.get_description() << '\n';
	exit(1);
    }
}
