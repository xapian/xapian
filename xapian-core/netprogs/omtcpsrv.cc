/* omtcpsrv.cc: Match server to be used with TcpClient
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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
#include <memory>
#include <algorithm>
#include <strstream.h>
#include <iomanip.h>
#include "database.h"
#include "omdatabaseinterface.h"
#include "database_builder.h"
#include <om/omerror.h>
#include <om/omenquire.h>
#include "tcpserver.h"

char *progname = 0;

int main(int argc, char *argv[]) {
    vector<vector<string> > dbargs;
    vector<string> dbtypes;
    int port = 0;

    progname = argv[0];

    bool one_shot = false;

    bool syntax_error = false;
    argv++;
    argc--;

    while (argc && argv[0][0] == '-') {
	if (argc >= 2 && strcmp(argv[0], "--da-flimsy") == 0) {
	    vector<string> args;
	    args.push_back(argv[1]);
	    dbargs.push_back(args);
	    dbtypes.push_back("da_flimsy");
	    argc -= 2;
	    argv += 2;
	} else if (argc >= 2 && strcmp(argv[0], "--da-heavy") == 0) {
	    vector<string> args;
	    args.push_back(argv[1]);
	    dbargs.push_back(args);
	    dbtypes.push_back("da_heavy");
	    argc -= 2;
	    argv += 2;
	} else if (argc >= 2 && strcmp(argv[0], "--db-flimsy") == 0) {
	    vector<string> args;
	    args.push_back(argv[1]);
	    dbargs.push_back(args);
	    dbtypes.push_back("db_flimsy");
	    argc -= 2;
	    argv += 2;
	} else if (argc >= 2 && strcmp(argv[0], "--db-heavy") == 0) {
	    vector<string> args;
	    args.push_back(argv[1]);
	    dbargs.push_back(args);
	    dbtypes.push_back("db_heavy");
	    argc -= 2;
	    argv += 2;
	} else if (argc >= 2 && strcmp(argv[0], "--im") == 0) {
	    vector<string> args;
	    args.push_back(argv[1]);
	    dbargs.push_back(args);
	    dbtypes.push_back("inmemory");
	    argc -= 2;
	    argv += 2;
	} else if (argc >= 2 && strcmp(argv[0], "--sleepycat") == 0) {
	    vector<string> args;
	    args.push_back(argv[1]);
	    dbargs.push_back(args);
	    dbtypes.push_back("sleepycat");
	    argc -= 2;
	    argv += 2;
	} else if (argc >= 2 && strcmp(argv[0], "--port") == 0) {
	    port = atoi(argv[1]);
	    argc -= 2;
	    argv += 2;
	} else if (strcmp(argv[0], "--one-shot") == 0) {
	    one_shot = true;
	    argc -= 1;
	    argv += 1;
	} else {
	    syntax_error = true;
	    break;
	}
    }
	
    if (syntax_error || argc > 0 || !dbtypes.size()) {
	cerr << "Syntax: " << progname << " [OPTIONS]" << endl <<
		"\t--[da-flimsy|da-heavy|db-flimsy|db-heavy] DIRECTORY\n" <<
		"\t--im INMEMORY\n" <<
		"\t--port NUM" << endl;
	exit(1);
    }

    if (port <= 0 || port >= 65536) {
	cerr << "Error: must specify a valid port." << endl; 
	exit(1);
    }

    cout << "Opening server on port " << port << "..." << endl;

    try {
        OmDatabaseGroup mydbs;

	vector<string>::const_iterator p;
	vector<vector<string> >::const_iterator q;
	for(p = dbtypes.begin(), q = dbargs.begin();
	    p != dbtypes.end();
	    p++, q++) {

	    mydbs.add_database(*p, *q);
	}

	OmRefCntPtr<MultiDatabase> mdb(
	    OmDatabaseGroup::InternalInterface::get_multidatabase(mydbs));

	TcpServer server(mdb, port);

	if (one_shot) {
	    server.run_once();
	} else {
	    server.run();
	}
    } catch (OmError &e) {
	cerr << "OmError exception (" << typeid(e).name()
	     << "): " << e.get_msg() << endl;
    } catch (std::exception &e) {
	cerr << "Caught standard exception: " << typeid(e).name();
    } catch (...) {
	cerr << "Caught unknown exception" << endl;
    }
}
