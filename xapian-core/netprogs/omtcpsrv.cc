/* omtcpsrv.cc: Match server to be used with TcpClient
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Ananova Ltd
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
 * -----END-LICENCE-----
 */

#include <iostream>
#include <iomanip>
#include <string>
#include <typeinfo>
#include <algorithm>
#include <strstream.h>
#include <iomanip.h>
#include "database.h"
#include "database_builder.h"
#include "om/omerror.h"
#include "om/omenquire.h"
#include "tcpserver.h"

using std::vector;

char *progname = 0;

int main(int argc, char *argv[]) {
    std::vector<OmSettings *> dbs;
    int port = 0;
    int msecs_active_timeout = 15000;
    int msecs_idle_timeout   = 60000;

    progname = argv[0];

    bool one_shot = false;

    bool verbose = true;

#ifdef TIMING_PATCH
    bool timing = false;
    
#endif /* TIMING_PATCH */
    bool syntax_error = false;
    argv++;
    argc--;

    // FIXME: use getopt
    while (argc && argv[0][0] == '-') {
	if (argc >= 2 && strcmp(argv[0], "--port") == 0) {
	    port = atoi(argv[1]);
	    argc -= 2;
	    argv += 2;
	} else if (argc >= 2 && strcmp(argv[0], "--active-timeout") == 0) {
	    msecs_active_timeout = atoi(argv[1]);
	    argc -= 2;
	    argv += 2;
	} else if (argc >= 2 && strcmp(argv[0], "--idle-timeout") == 0) {
	    msecs_idle_timeout = atoi(argv[1]);
	    argc -= 2;
	    argv += 2;
	} else if (argc >= 2 && strcmp(argv[0], "--timeout") == 0) {
	    msecs_idle_timeout = atoi(argv[1]);
	    msecs_active_timeout = msecs_idle_timeout;
	    argc -= 2;
	    argv += 2;
	} else if (strcmp(argv[0], "--one-shot") == 0) {
	    one_shot = true;
	    argc -= 1;
	    argv += 1;
	} else if (strcmp(argv[0], "--quiet") == 0) {
	    verbose = false;
	    argc -= 1;
	    argv += 1;
#ifdef TIMING_PATCH
	} else if (strcmp(argv[0], "--timing") == 0) {
	    timing = true;
	    argc -= 1;
	    argv += 1;
#endif /* TIMING_PATCH */
	} else {
	    syntax_error = true;
	    break;
	}
    }

    if (syntax_error || argc == 0) {
	cerr << "Syntax: " << progname << " [OPTIONS]\n"
		"\t--port NUM\n"
		"\t--idle-timeout MSECS\n"
		"\t--active-timeout MSECS\n"
		"\t--timeout MSECS\n"
		"\t--one-shot\n"
		"\t--quiet\n"
	        "DATABASE_DIRECTORY..." << endl;
	exit(1);
    }
    
    if (port <= 0 || port >= 65536) {
	cerr << "Error: must specify a valid port number (between 1 and 65535)."
	     << endl;
	exit(1);
    }
    
    while (*argv) {
	OmSettings *params = new OmSettings();
	params->set("backend", "auto");
	params->set("auto_dir", *argv);
	dbs.push_back(params);
	++argv;
    }

    if (verbose) cout << "Opening server on port " << port << "..." << endl;

    try {
        OmDatabase mydbs;

	std::vector<OmSettings *>::const_iterator p;
	for (p = dbs.begin(); p != dbs.end(); p++) {
	    mydbs.add_database(**p);
	    delete *p;
	}

#ifndef TIMING_PATCH
	TcpServer server(mydbs, port, msecs_active_timeout,
			 msecs_idle_timeout, verbose);
#else /* TIMING_PATCH */
	TcpServer server(mydbs, port, msecs_active_timeout,
			 msecs_idle_timeout, verbose, timing);
#endif /* TIMING_PATCH */

	if (one_shot) {
	    server.run_once();
	} else {
	    server.run();
	}
    } catch (OmError &e) {
	cerr << "OmError exception (" << typeid(e).name()
	     << "): " << e.get_msg() << endl;
	exit(1);
    } catch (std::exception &e) {
	cerr << "Caught standard exception: " << typeid(e).name();
	exit(1);
    } catch (...) {
	cerr << "Caught unknown exception" << endl;
	exit(1);
    }
}
