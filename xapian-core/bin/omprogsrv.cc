/* omprogsrv.cc: Match server to be used with ProgClient
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003 Olly Betts
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
#include <algorithm>
#include <xapian/error.h>
#include "netutils.h"
#include "progserver.h"
#include "omerr_string.h"
#include "backendmanager.h"

using namespace std;

int main(int argc, char **argv) {
    if (argc < 3) {
	cerr << "Wrong number of arguments" << endl;
	cout << "ERROR" << endl;
	exit(-1);
    }

    /* variables needed in both try/catch blocks */
    Xapian::Database dbs;
    unsigned int timeout = 60000;

    /* Trap exceptions related to setting up the database */
    try {
	// open the database to return results
	BackendManager backendmanager;
	backendmanager.set_datadir(argv[1]);
	backendmanager.set_dbtype("inmemory");

	vector<string> paths;

	for (int i = 2; i < argc; ++i) {
	    string arg = argv[i];
	    if (arg.length() >= 2 && arg[0] == '-') {
		if (arg[1] == 't') {
		    timeout = atoi(arg.c_str() + 2);
		    continue;
		}
		if (arg[1] == 'e') {
		    if (arg == "-e") {
			backendmanager.set_dbtype("inmemoryerr");
			continue;
		    }
		    if (arg == "-e2") {
			backendmanager.set_dbtype("inmemoryerr2");
			continue;
		    }
		    if (arg == "-e3") {
			backendmanager.set_dbtype("inmemoryerr3");
			continue;
		    }
		}
		// FIXME: better to complain about unknown options that to
		// treat them as paths?
	    }
	    paths.push_back(argv[i]);
	}

	Xapian::Database db = backendmanager.get_database(paths);
	dbs.add_database(db);
    } catch (const Xapian::Error &e) {
	cout << 'E' << omerror_to_string(e) << endl;
    } catch (...) {
	cout << "EUNKNOWN" << endl;
    }

    /* Catch exceptions from running the server, but don't pass them
     * on to the remote end, as SocketServer will do that itself.
     */
    try {
	ProgServer server(dbs, 0, 1, timeout, timeout);
	// If you have defined your own weighting scheme, register it here
	// like so:
	// server.register_weighting_scheme(FooWeight());

	server.run();
    } catch (...) {
    }
}
