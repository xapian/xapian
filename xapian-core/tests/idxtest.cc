/* idxtest.cc:
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

#include "om.h"
#include <algorithm>
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc < 4 || argc > 5) {
	printf("Syntax: %s dbdir term docid [termpos]\n", argv[0]);
	exit(1);
    }

    try {
#if 0
	SleepyDatabase db;
	db.open(argv[1], 0);

	termid tid = db.add_term(argv[2]);
	docid did = atoi(argv[3]);
	termpos tpos = 0;
	if(argc == 5) {
	    tpos = atoi(argv[4]);
	}

	db.add(tid, did, tpos);
#endif
    }
    catch (OmError e) {
	cout << e.get_msg() << endl;
    }
    catch (exception e) {
	cout << "Exception" << endl;
    }

    return 0;
}
