/* omprogsrv.cc: Match server to be used with ProgClient
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
#include "database_builder.h"
#include <om/omerror.h>
#include <om/omenquire.h>
#include "localmatch.h"
#include "omqueryinternal.h"
#include "netutils.h"
#include "progserver.h"

int main(int argc, char *argv[]) {
    string message;
    getline(cin, message);
    cerr << "omnetclient: read " << message << endl;
    cout << "BOO!" << endl;
    cout.flush();

    if (argc != 2) {
	cerr << "Wrong number of arguments" << endl;
	cout << "ERROR" << endl;
	exit(-1);
    }

    try {
	// open the database to return results
	DatabaseBuilderParams param(OM_DBTYPE_INMEMORY);
	param.paths.push_back(argv[1]);

	DatabaseBuilderParams mparam(OM_DBTYPE_MULTI);
	mparam.subdbs.push_back(param);
	auto_ptr<IRDatabase> db(DatabaseBuilder::create(mparam));
	OmRefCntPtr<MultiDatabase> mdb(dynamic_cast<MultiDatabase *>(db.get()));
	if (!mdb.get()) {
	    throw OmDatabaseError("Invalid database");
	} else {
	    // db no longer should have ownership
	    db.release();
	}

	ProgServer server(mdb, 0, 1);

	server.run();
    } catch (OmError &e) {
	cerr << "OmError exception (" << typeid(e).name()
	     << "): " << e.get_msg() << endl;
    } catch (...) {
	cerr << "Caught exception" << endl;
    }
}

