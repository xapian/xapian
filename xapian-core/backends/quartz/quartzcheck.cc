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

#include <config.h>
#include <iostream>

using namespace std;

#include "btreecheck.h"

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
		" + = same as tbv (default)\n"
		" e.g. " << argv[0]
	     << " /var/lib/xapian/data/default/postlist_ fbv"
	     << endl;
	exit(1);
    }

    int opts = 0;
    const char * opt_string = argv[2];
    if (!opt_string) opt_string = "+";
    for (const char *p = opt_string; *p; ++p) {
	switch (*p) {
	    case 't': opts |= OPT_SHORT_TREE; break;
	    case 'f': opts |= OPT_FULL_TREE; break;
	    case 'b': opts |= OPT_SHOW_BITMAP; break;
	    case 'v': opts |= OPT_SHOW_STATS; break;
	    case '+':
		opts |= OPT_SHORT_TREE | OPT_SHOW_BITMAP | OPT_SHOW_STATS;
		break;
	    case '?':
		cerr << "use t,f,b,v or + in the option string\n";
		exit(0);
	    default:
		cerr << "option " << opt_string << " unknown\n";
		exit(1);
	}
    }

    try {
	BtreeCheck::check(argv[1], opts);
    }
    catch (const OmError &error) {
	cerr << argv[0] << ": " << error.get_msg() << endl;
	exit(1);
    }
    return 0;
}
