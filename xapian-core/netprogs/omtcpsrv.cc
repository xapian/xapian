/* omtcpsrv.cc: Match server to be used with TcpClient
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

using std::vector;

char *progname = 0;

int main(int argc, char *argv[]) {
    std::vector<OmSettings *> dbs;
    int port = 0;

    progname = argv[0];

    bool one_shot = false;

    bool syntax_error = false;
    argv++;
    argc--;

    while (argc && argv[0][0] == '-') {
	if (argc >= 2 && strcmp(argv[0], "--da-flimsy") == 0) {
	    OmSettings *params = new OmSettings();
	    std::string path = argv[1];
	    params->set("backend", "da");
	    params->set("m36_heavyduty", false);	    
	    params->set("m36_record_file", path + "/R");
	    params->set("m36_term_file", path + "/T");
	    dbs.push_back(params);
	    argc -= 2;
	    argv += 2;
	} else if (argc >= 2 && strcmp(argv[0], "--da-heavy") == 0) {
	    OmSettings *params = new OmSettings();
	    std::string path = argv[1];
	    params->set("backend", "da");
	    params->set("m36_heavyduty", true);
	    params->set("m36_record_file", path + "/R");
	    params->set("m36_term_file", path + "/T");
	    params->set("m36_key_file", path + "/keyfile");
	    dbs.push_back(params);
	    argc -= 2;
	    argv += 2;
	} else if (argc >= 2 && strcmp(argv[0], "--db-flimsy") == 0) {
	    OmSettings *params = new OmSettings();
	    std::string path = argv[1];
	    params->set("backend", "db");
	    params->set("m36_heavyduty", false);
	    params->set("m36_db_file", path + "/DB");
	    params->set("m36_key_file", path + "/keyfile");
	    dbs.push_back(params);
	    argc -= 2;
	    argv += 2;
	} else if (argc >= 2 && strcmp(argv[0], "--db-heavy") == 0) {
	    OmSettings *params = new OmSettings();
	    std::string path = argv[1];
	    params->set("backend", "db");
	    params->set("m36_heavyduty", true);
	    params->set("m36_db_file", path + "/DB");
	    params->set("m36_key_file", path + "/keyfile");
	    dbs.push_back(params);
	    argc -= 2;
	    argv += 2;
	} else if (argc >= 1 && strcmp(argv[0], "--im") == 0) {
	    OmSettings *params = new OmSettings();
	    params->set("backend", "inmemory");
	    dbs.push_back(params);
	    argc -= 1;
	    argv += 1;
	} else if (argc >= 2 && strcmp(argv[0], "--sleepycat") == 0) {
	    OmSettings *params = new OmSettings();
	    params->set("backend", "sleepycat");
	    params->set("sleepy_dir", argv[1]);
	    dbs.push_back(params);
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

    if (syntax_error || argc > 0 || !dbs.size()) {
	cerr << "Syntax: " << progname << " [OPTIONS]" << endl <<
		"\t--[da-flimsy|da-heavy|db-flimsy|db-heavy|sleepycat] DIRECTORY\n" <<
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

	std::vector<OmSettings *>::const_iterator p;
	for (p = dbs.begin(); p != dbs.end(); p++) {
	    mydbs.add_database(**p);
	    delete *p;
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
