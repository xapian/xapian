/* enquiretest.cc
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

#include <stdio.h>

#include "om.h"

#include <vector>
#include <string>

int
main(int argc, char *argv[])
{
    OMEnquire enquire;
    char *progname = argv[0];

    string type;
    if(argc >= 2) {
	type = argv[1];
	argv++;
	argc--;
    }

    vector<string> dbparams;
    while(argc >= 2) {
	if(string(argv[1]) == "--") break;
	dbparams.push_back(argv[1]);
	argv++;
	argc--;
    }

    try {
	enquire.set_database(type, dbparams);
    } catch (OmError e) {
	cout << progname << ": OmError " << e.get_msg() << endl;
    }
}
