/* quartzcheck.cc: use Btree::check to check consistency of a quartz btree
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002 Olly Betts
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

#include <om/om.h>
#include <iostream>

using namespace std;

#include "btree.h"

int
main(int argc, char **argv)
{
    if (argc < 2 || argc > 3) {
	cout << "usage: " << argv[0]
	     << " <path to btree and prefix> [[t][f][b][v][+]]\n"
	        " t = short tree printing\n"
		" f = full tree printing\n"
		" b = show bitmap\n"
		" v = show stats about B-tree\n"
		" + = same as tbv\n"
	        " e.g. " << argv[0] << " /var/spool/xapian/mydb/postlist_ +"
	     << endl;
	exit(1);
    }

    const char *opts = argv[2];
    if (!opts) opts = "+";

    try {
	Btree::check(argv[1], opts);
    }
    catch (const OmError &error) {
	cerr << argv[0] << ": " << error.get_msg() << endl;
	exit(1);
    }
    return 0;
}
