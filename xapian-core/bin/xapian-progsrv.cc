/** @file
 * @brief Remote server for use with ProgClient.
 */
/* Copyright (C) 2002,2003,2006,2007,2008,2010,2011,2023 Olly Betts
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

#include "net/remoteserver.h"

#include "gnu_getopt.h"

#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;

#define PROG_NAME "xapian-progsrv"
#define PROG_DESC "Piped server for use with Xapian's remote backend"

#define OPT_HELP 1
#define OPT_VERSION 2

static const char * opts = "t:w";
static const struct option long_opts[] = {
    {"timeout",		required_argument,	0, 't'},
    {"writable",	no_argument,		0, 'w'},
    {"help",		no_argument,		0, OPT_HELP},
    {"version",		no_argument,		0, OPT_VERSION},
    {NULL, 0, 0, 0}
};

static void show_usage() {
    cout << "Usage: " PROG_NAME " [OPTIONS] DATABASE_DIRECTORY...\n\n"
"Options:\n"
"  --timeout MSECS         set timeout\n"
"  --writable              allow updates\n"
"  --help                  display this help and exit\n"
"  --version               output version information and exit\n";
}

int main(int argc, char **argv)
{
    double timeout = 60.0;
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
	    case 't':
		timeout = atoi(optarg) * 1e-3;
		break;
	    case 'w':
		writable = true;
		break;
	    default:
		syntax_error = true;
	}
    }

    if (syntax_error || optind == argc) {
	show_usage();
	exit(1);
    }

    /* Unlike xapian-tcpsrv, xapian-progsrv only has a single 'connection'
     * which is established immediately.  So, there is no point in attempting
     * to open the database(s) to check they work - if they can't be opened the
     * client will get an exception right away anyway.
     */
    vector<string> dbnames(argv + optind, argv + argc);

    try {
	// We communicate with the client via stdin (fd 0) and stdout (fd 1).
	// Note that RemoteServer closes these fds.
	RemoteServer server(dbnames, 0, 1, timeout, timeout, writable);

	// If you have defined your own weighting scheme, register it here
	// like so:
	// server.register_weighting_scheme(FooWeight());

	server.run();
    } catch (...) {
	/* Catch and ignore any exceptions thrown by RemoteServer, since the
	 * RemoteServer will have passed the error to the client to be rethrown
	 * there.
	 *
	 * Our stdout is the (now closed) communication channel to the client,
	 * and our stderr is probably a closed fd so we don't have anywhere to
	 * send error messages to anyway!
	 */
    }
}
