/* omnetclient.cc: remote match program
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
#include <string>
#include <typeinfo>
#include <memory>
#include "database.h"
#include "database_builder.h"
#include <om/omerror.h>

void run_matcher();

int main() {
    string message;
    getline(cin, message);
    cerr << "omnetclient: read " << message << endl;
    cout << "BOO!" << endl;
    cout.flush();

    try {
	run_matcher();
    } catch (OmError &e) {
	cerr << "OmError exception (" << typeid(e).name()
	     << "): " << e.get_msg() << endl;
    } catch (...) {
	cerr << "Caught exception" << endl;
    }
}

void run_matcher() {
    // open the database to return results
    DatabaseBuilderParams param(OM_DBTYPE_INMEMORY);
    param.paths.push_back("text.txt");
    auto_ptr<IRDatabase> db(DatabaseBuilder::create(param));

    while (1) {
	string message;
	getline(cin, message);

	if (cin == "GETDOCCOUNT") {
	    cout << db->get_doccount() << endl;
	    cout.flush();
	} else {
	    cout << "ERROR" << endl;
	    cout.flush();
	}
    }
}
