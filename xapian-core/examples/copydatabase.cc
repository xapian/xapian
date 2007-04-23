/* copydatabase.cc: Document-by-document copy of one or more Xapian databases
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2006 Olly Betts
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

#include <xapian.h>

#include <stdlib.h>

#include <iostream>

using namespace Xapian;
using namespace std;

#define PROG_NAME "copydatabase"
#define PROG_DESC "Perform a document-by-document copy of one or more Xapian databases"

static void show_usage() {
    cout << "Usage: "PROG_NAME" SOURCE_DATABASE... DESTINATION_DATABASE\n\n"
"Options:\n"
"  --help           display this help and exit\n"
"  --version        output version information and exit" << endl;
}

int
main(int argc, char **argv)
{
    if (argc > 1 && argv[1][0] == '-') {
	if (strcmp(argv[1], "--help") == 0) {
	    cout << PROG_NAME" - "PROG_DESC"\n\n";
	    show_usage();
	    exit(0);
	}
	if (strcmp(argv[1], "--version") == 0) {
	    cout << PROG_NAME" - "PACKAGE_STRING << endl;
	    exit(0);
	}
    }

    // We take at least two arguments - one or more paths to source databases
    // and the path to the database to create
    if (argc < 3) {
	show_usage();
	exit(1);
    }

    const char *dest = argv[argc - 1];
    try {
	// Create the destination database
	WritableDatabase dest_database(dest, DB_CREATE);

	for (int i = 1; i < argc - 1; ++i) {
	    // Open the source database
	    Database src_database(argv[i]);

	    // Copy each document across

	    // At present there's no way to iterate across all documents
	    // so we have to test each docid in turn until we've found all
	    // the documents or reached the highest numbered one.
	    doccount count = src_database.get_doccount();
	    docid did = 1;
	    docid max_did = src_database.get_lastdocid();
	    while (count) {
		try {
		    Document document = src_database.get_document(did);
		    dest_database.add_document(document);
		    --count;
		    cout << '\r' << argv[i] << ": " << count
			 << " docs to go " << flush;
		} catch (const DocNotFoundError &) {
		    // that document must have been deleted
		}
		if (did == max_did) break;
		++did;
	    }
	    cout << '\r' << argv[i] << ": Done                  " << endl;
	}
    } catch (const Error &error) {
	cerr << argv[0] << ": " << error.get_msg() << endl;
	exit(1);
    }
}
