/* quartzcompact.cc: Make a quartzdatabase with full compaction.  This makes
 * it smaller and faster to search if you aren't going to be updating it.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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

#include <fstream>
#include <iostream>

using namespace std;

#include <errno.h>
#include <stdio.h> // for rename()
#include <string.h>
#include <sys/stat.h>

#include "btree.h"
#include "bcursor.h"

int
main(int argc, char **argv)
{
    // We take two arguments - the path to source databases
    // and the path to the database to create
    if (argc != 3) {
	cout << "usage: " << argv[0]
	     << " <path to source database> <path to destination database>"
	     << endl;
	exit(1);
    }

    const char *destdir = argv[2];
    try {
	// Create the directory for the database, if it doesn't exist already
	if (mkdir(destdir, 0755) == -1) {
	    // Check if mkdir failed because there's already a directory there
	    // or for some other reason - we also get EEXIST if there's a file
	    // with that name.
	    if (errno == EEXIST) {
		struct stat sb;
		if (stat(destdir, &sb) == 0 && S_ISDIR(sb.st_mode))
		    errno = 0;
		else
		    errno = EEXIST; // stat might have changed it
	    }
	    if (errno) {
		cerr << argv[0] << ": couldn't create directory `"
		     << destdir << "': " << strerror(errno) << endl;
		exit(1);
	    }
	}

	const char * tables[] = {
	    "record", "postlist", "termlist", "position", "value"
	};
	for (const char **t = tables;
	     t != tables + sizeof(tables)/sizeof(tables[0]); ++t) {
	    string src(argv[1]);
	    src += '/';
	    src += *t;
	    src += '_';
	    Btree in;
	    in.open_to_read(src);

	    string dest = destdir;
	    dest += '/';
	    dest += *t;
	    dest += '_';
	    Btree::create(dest, 8192);

	    Btree out;
	    out.open_to_write(dest);
	    out.set_full_compaction(true);

	    string key, tag;
	    if (in.item_count) {
		Bcursor BC(&in);
		BC.find_key(key);
		while (BC.get_key(&key)) {
		    BC.get_tag(&tag);
		    out.add(key, tag);
		}
	    }
	    out.commit(1);
	    cout << '\r' << *t << ": Done                  " << endl;
	}
	// Copy meta file
	string src(argv[1]);
	src += "/meta";
	string dest = destdir;
	dest += "/meta.tmp";
	{
	    ifstream metain(src.c_str());
	    ofstream metaout(dest.c_str());
	    char buf[2048];
	    while (!metain.eof()) {
		// FIXME check for errors
		metain.read(buf, sizeof(buf));
		metaout.write(buf, metain.gcount());
	    }
	}
	string meta = destdir;
	meta += "/meta";
	if (rename(dest.c_str(), meta.c_str()) == -1) {
	    cerr << argv[0] << ": couldn't rename `" << dest << "' to `"
		 << meta << "': " << strerror(errno) << endl;
	    exit(1);
	}
    } catch (const Xapian::Error &error) {
	cerr << argv[0] << ": " << error.get_msg() << endl;
	exit(1);
    }
}
