/** @file
 * @brief Replicate a database from a master server to a local copy.
 */
/* Copyright (C) 2008,2011,2012,2015 Olly Betts
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

#include "net/replicatetcpclient.h"

#include <xapian.h>

#include "gnu_getopt.h"
#include "stringutils.h"
#include "safeunistd.h"

#include <iostream>

using namespace std;

#define PROG_NAME "xapian-replicate"
#define PROG_DESC "Replicate a database from a master server to a local copy"

#define OPT_HELP 1
#define OPT_VERSION 2

// Wait DEFAULT_INTERVAL seconds between updates unless --interval is passed.
#define DEFAULT_INTERVAL 60

// Number of seconds before we assume that a reader will be closed.
#define READER_CLOSE_TIME 30

// Socket level timeout (in seconds).
#define DEFAULT_TIMEOUT 0

static void show_usage() {
    cout << "Usage: " PROG_NAME " [OPTIONS] DATABASE\n\n"
"Options:\n"
"  -h, --host=HOST     host to connect to (required)\n"
"  -p, --port=PORT     port to connect to (required)\n"
"  -m, --master=DB     replicate database DB from the master (default: DATABASE)\n"
"  -i, --interval=N    wait N seconds between each connection to the master\n"
"                      (default: " STRINGIZE(DEFAULT_INTERVAL) ")\n"
"  -r, --reader-time=N wait N seconds to allow readers time to close before\n"
"                      applying repeated changesets (default: " STRINGIZE(READER_CLOSE_TIME) ")\n"
"  -t, --timeout=N     set socket timeouts (if supported) to N seconds; N=0 for\n"
"                      no timeout (default: " STRINGIZE(DEFAULT_TIMEOUT) ")\n"
"  -f, --force-copy    force a full copy of the database to be sent (and then\n"
"                      replicate as normal)\n"
"  -o, --one-shot      replicate only once and then exit\n"
"  -q, --quiet         only report errors\n"
"  -v, --verbose       be more verbose\n"
"  --help              display this help and exit\n"
"  --version           output version information and exit\n";
}

int
main(int argc, char **argv)
{
    const char * opts = "h:p:m:i:r:t:ofqv";
    static const struct option long_opts[] = {
	{"host",	required_argument,	0, 'h'},
	{"port",	required_argument,	0, 'p'},
	{"master",	required_argument,	0, 'm'},
	{"interval",	required_argument,	0, 'i'},
	{"reader-time",	required_argument,	0, 'r'},
	{"timeout",	required_argument,	0, 't'},
	{"one-shot",	no_argument,		0, 'o'},
	{"force-copy",	no_argument,		0, 'f'},
	{"quiet",	no_argument,		0, 'q'},
	{"verbose",	no_argument,		0, 'v'},
	{"help",	no_argument, 0, OPT_HELP},
	{"version",	no_argument, 0, OPT_VERSION},
	{NULL,		0, 0, 0}
    };

    string host;
    int port = 0;
    string masterdb;
    int interval = DEFAULT_INTERVAL;
    bool one_shot = false;
    enum { NORMAL, VERBOSE, QUIET } verbosity = NORMAL;
    bool force_copy = false;
    int reader_close_time = READER_CLOSE_TIME;
    int timeout = DEFAULT_TIMEOUT;

    int c;
    while ((c = gnu_getopt_long(argc, argv, opts, long_opts, 0)) != -1) {
	switch (c) {
	    case 'h':
		host.assign(optarg);
		break;
	    case 'p':
		port = atoi(optarg);
		break;
	    case 'm':
		masterdb.assign(optarg);
		break;
	    case 'i':
		interval = atoi(optarg);
		break;
	    case 'r':
		reader_close_time = atoi(optarg);
		break;
	    case 't':
		timeout = atoi(optarg);
		break;
	    case 'f':
		force_copy = true;
		break;
	    case 'o':
		one_shot = true;
		break;
	    case 'q':
		verbosity = QUIET;
		break;
	    case 'v':
		verbosity = VERBOSE;
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

    if (host.empty()) {
	cout << "Host required - specify with --host=HOST\n\n";
	show_usage();
	exit(1);
    }

    if (port == 0) {
	cout << "Port required - specify with --port=PORT\n\n";
	show_usage();
	exit(1);
    }

    // Path to the database to create/update.
    string dbpath(argv[optind]);

    if (masterdb.empty())
	masterdb = dbpath;

    while (true) {
	try {
	    if (verbosity == VERBOSE) {
		cout << "Connecting to " << host << ":" << port << '\n';
	    }
	    ReplicateTcpClient client(host, port, 10.0, timeout);
	    if (verbosity == VERBOSE) {
		cout << "Getting update for " << dbpath << " from "
		     << masterdb << '\n';
	    }
	    Xapian::ReplicationInfo info;
	    client.update_from_master(dbpath, masterdb, info,
				      reader_close_time, force_copy);
	    if (verbosity == VERBOSE) {
		cout << "Update complete: "
		     << info.fullcopy_count << " copies, "
		     << info.changeset_count << " changesets, "
		     << (info.changed ? "new live database"
				      : "no changes to live database")
		     <<	'\n';
	    }
	    if (verbosity != QUIET) {
		if (info.fullcopy_count > 0 && !info.changed) {
		    cout <<
"Replication using a full copy failed.  This usually means that the master\n"
"database is changing too frequently.  Ensure that sufficient changesets are\n"
"present by setting XAPIAN_MAX_CHANGESETS on the master.\n";
		}
	    }
	    force_copy = false;
	} catch (const Xapian::NetworkError &error) {
	    // Don't stop running if there's a network error - just log to
	    // stderr and retry at next timeout.  This should make the client
	    // robust against temporary network failures.
	    cerr << argv[0] << ": " << error.get_description() << '\n';

	    // If we were running as a one-shot client though, we're going to
	    // exit anyway, so let's make the return value reflect that there
	    // was a failure.
	    if (one_shot)
		exit(1);
	} catch (const Xapian::Error &error) {
	    cerr << argv[0] << ": " << error.get_description() << '\n';
	    exit(1);
	} catch (const exception &e) {
	    cerr << "Caught standard exception: " << e.what() << '\n';
	    exit(1);
	} catch (...) {
	    cerr << "Caught unknown exception\n";
	    exit(1);
	}
	if (one_shot) break;
	sleep(interval);
    }
}
