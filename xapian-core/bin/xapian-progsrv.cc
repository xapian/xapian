/* omprogsrv.cc: Match server to be used with ProgClient
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2006 Olly Betts
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
#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <xapian/error.h>
#include "remoteserver.h"
#include "serialise.h"

using namespace std;

int main(int argc, char **argv) {
    /* variables needed in both try/catch blocks */
    Xapian::WritableDatabase wdb;
    Xapian::Database dbs;
    unsigned int timeout = 60000;
    bool writable = false;

    /* Trap exceptions related to setting up the database */
    try {
	vector<string> paths;

	for (int i = 1; i < argc; ++i) {
	    string arg = argv[i];
	    if (arg.length() >= 2 && arg[0] == '-') {
		if (arg == "--writable") {
		    writable = true;
		    continue;
		}
		if (arg[1] == 't') {
		    timeout = atoi(arg.c_str() + 2);
		    continue;
		}
		// FIXME: better to complain about unknown options that to
		// treat them as paths?
	    }
	    paths.push_back(arg);
	}

	if (paths.empty()) {
	    cerr << "Error: No database directories specified." << endl;
	    exit(1);
	}

	if (writable) {
	    if (paths.size() > 1) {
		cerr << "Error: at most one database directory allowed with '--writable'." << endl;
		exit(1);
	    }
	    wdb = Xapian::WritableDatabase(paths[0], Xapian::DB_CREATE_OR_OPEN);
	} else {
	    vector<string>::const_iterator i;
	    for (i = paths.begin(); i != paths.end(); ++i) {
		dbs.add_database(Xapian::Database(*i));
	    }
	}
    } catch (const Xapian::Error &e) {
	// FIXME: we shouldn't build messages by hand here.
	string msg = serialise_error(e);
	cout << char(REPLY_EXCEPTION) << encode_length(msg.size()) << msg << flush;
    } catch (...) {
	// FIXME: we shouldn't build messages by hand here.
	cout << char(REPLY_EXCEPTION) << encode_length(0) << flush;
    }

    /* Catch exceptions from running the server, but don't pass them
     * on to the remote end, as the RemoteServer will do that itself.
     */
    try {
	if (writable) {
	    RemoteServer server(&wdb, 0, 1, timeout, timeout);
	    // If you have defined your own weighting scheme, register it here
	    // like so:
	    // server.register_weighting_scheme(FooWeight());

	    server.run();
	} else {
	    RemoteServer server(&dbs, 0, 1, timeout, timeout);
	    // If you have defined your own weighting scheme, register it here
	    // like so:
	    // server.register_weighting_scheme(FooWeight());

	    server.run();
	}
    } catch (...) {
    }
}
