/* omprogsrv.cc: Match server to be used with ProgClient
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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
#include "autoptr.h"
#include <algorithm>
#include <strstream.h>
#include <iomanip.h>
#include "database_builder.h"
#include "om/omerror.h"
#include "om/omenquire.h"
#include "omqueryinternal.h"
#include "netutils.h"
#include "progserver.h"
#include "omerr_string.h"
#include "backendmanager.h"

int main(int argc, char *argv[]) {
    std::string message;
#if 0
    getline(cin, message);
    cerr << "omnetclient: read " << message << endl;
    cout << "BOO!" << endl;
    cout.flush();
#endif

    if (argc < 3) {
	cerr << "Wrong number of arguments" << endl;
	cout << "ERROR" << endl;
	exit(-1);
    }

    /* variables needed in both try/catch blocks */
    OmDatabase dbgrp;
    unsigned int timeout = 30000;

    /* Trap exceptions related to setting up the database */
    try {
	// open the database to return results
	BackendManager backendmanager;
	backendmanager.set_datadir(argv[1]);
	backendmanager.set_dbtype("inmemory");

	std::vector<std::string> paths;

	for (int i=2; i<argc; ++i) {
	    std::string arg = argv[i];
	    if (arg == "-e") {
		backendmanager.set_dbtype("inmemoryerr");
	    } else if (arg == "-e2") {
		backendmanager.set_dbtype("inmemoryerr2");
	    } else if (arg == "-e3") {
		backendmanager.set_dbtype("inmemoryerr3");
	    } else if (arg.substr(0, 2) == "-t") {
		timeout = atoi(arg.c_str() + 2);
	    } else {
		paths.push_back(argv[i]);
	    }
	}

	OmDatabase db = backendmanager.get_database(paths);
	dbgrp.add_database(db);
    } catch (OmError &e) {
	/*
	cerr << "OmError exception (" << typeid(e).name()
	     << "): " << e.get_msg() << endl;
	 */
	cout << "E" << omerror_to_string(e) << endl;
    } catch (...) {
	/*
	cerr << "Caught exception" << endl;
	 */
	cout << "EUNKNOWN" << endl;
    }

    /* Catch exceptions from running the server, but don't pass them
     * on to the remote end, as SocketServer will do that itself.
     */
    try {
	ProgServer server(dbgrp, 0, 1, timeout, timeout);

	server.run();
    } catch (OmError &e) {
	/*
	cerr << "OmError exception (" << typeid(e).name()
	     << "): " << e.get_msg() << endl;
	 */
    } catch (...) {
	/*
	cerr << "Caught unknown exception" << endl;
	 */
    }
}

