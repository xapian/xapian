/* omtcpsrv.cc: Match server to be used with TcpClient
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
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

#include <config.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <typeinfo>
#include <algorithm>

#include "getopt.h"

#include "database.h"
#include "database_builder.h"
#include "om/omerror.h"
#include "om/omenquire.h"
#include "tcpserver.h"

using std::vector;
using std::cout;
using std::cerr;
using std::endl;
using std::exception;

char *progname = 0;

int main(int argc, char *argv[]) {
    vector<OmSettings *> dbs;
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

    struct option opts[] = {
	{"port",		required_argument, 0, 'p'},
	{"active-timeout",	required_argument, 0, 'a'},
	{"idle-timeout",	required_argument, 0, 'i'},
	{"timeout",		required_argument, 0, 't'},
	{"one-shot",		no_argument, 0, 'o'},
	{"quiet",		no_argument, 0, 'q'},
#ifdef TIMING_PATCH
	{"timing",		no_argument, 0, 'T'},
#endif /* TIMING_PATCH */
	{NULL,			0, 0, 0}
    };

    int c;
    while ((c = getopt_long(argc, argv, "p:a:i:t:oq", opts, NULL)) != EOF) {
	switch (c) {
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
#ifdef TIMING_PATCH
	    case 'T':
		timing = true;
		break;
#endif /* TIMING_PATCH */
	    default:
		syntax_error = true;
	}
    }

    if (syntax_error || argv[optind] == NULL) {
	cerr << "Syntax: " << progname << " [OPTIONS] DATABASE_DIRECTORY...\n"
		"\t--port NUM\n"
		"\t--idle-timeout MSECS\n"
		"\t--active-timeout MSECS\n"
		"\t--timeout MSECS\n"
		"\t--one-shot\n"
		"\t--quiet" << endl;
	exit(1);
    }
    
    if (port <= 0 || port >= 65536) {
	cerr << "Error: must specify a valid port number (between 1 and 65535)."
		" We actually got " << (port) << endl;
	exit(1);
    }
    
    while (argv[optind]) {
	OmSettings *params = new OmSettings();
	params->set("backend", "auto");
	params->set("auto_dir", argv[optind++]);
	dbs.push_back(params);
    }

    if (verbose) cout << "Opening server on port " << port << "..." << endl;

    try {
        OmDatabase mydbs;

	vector<OmSettings *>::const_iterator p;
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
    } catch (const OmError &e) {
	cerr << "OmError exception (" << typeid(e).name()
	     << "): " << e.get_msg() << endl;
	exit(1);
    } catch (const exception &e) {
	cerr << "Caught standard exception: " << typeid(e).name();
	exit(1);
    } catch (...) {
	cerr << "Caught unknown exception" << endl;
	exit(1);
    }
}
