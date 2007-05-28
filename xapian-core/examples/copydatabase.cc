/** @file copydatabase.cc
 * @brief Perform a document-by-document copy of one or more Xapian databases.
 */
/* Copyright (C) 2006,2007 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include <xapian.h>

#include <iomanip>
#include <iostream>

#include <math.h> // For log10().
#include <stdlib.h> // For exit().
#include <string.h> // For strcmp() and strrchr().

using namespace std;

#define PROG_NAME "copydatabase"
#define PROG_DESC "Perform a document-by-document copy of one or more Xapian databases"

static void
show_usage(int rc)
{
    cout << "Usage: "PROG_NAME" SOURCE_DATABASE... DESTINATION_DATABASE\n\n"
"Options:\n"
"  --help           display this help and exit\n"
"  --version        output version information and exit" << endl;
    exit(rc);
}

int
main(int argc, char **argv)
try {
    if (argc > 1 && argv[1][0] == '-') {
	if (strcmp(argv[1], "--help") == 0) {
	    cout << PROG_NAME" - "PROG_DESC"\n\n";
	    show_usage(0);
	}
	if (strcmp(argv[1], "--version") == 0) {
	    cout << PROG_NAME" - "PACKAGE_STRING << endl;
	    exit(0);
	}
    }

    // We expect two or more arguments: at least one source database path
    // followed by the destination database path.
    if (argc < 3) show_usage(1);

    // Create the destination database, using DB_CREATE so that we don't
    // try to overwrite or update an existing database in case the user
    // got the command line argument order wrong.
    const char *dest = argv[argc - 1];
    Xapian::WritableDatabase db_out(dest, Xapian::DB_CREATE);

    for (int i = 1; i < argc - 1; ++i) {
	const char * src = argv[i];

	// Open the source database.
	Xapian::Database db_in(src);

	// Find the leaf-name of the database path for reporting progress.
	const char * leaf = strrchr(src, '/');
	if (!leaf) leaf = strrchr(src, '\\');
	if (leaf) ++leaf; else leaf = src;

	// Iterate over all the documents in db_in, copying each to db_out.
	Xapian::doccount dbsize = db_in.get_doccount();
	if (dbsize == 0) {
	    cout << leaf << ": empty!" << endl;
	    continue;
	}

	// Calculate how many decimal digits there are in dbsize.
	int width = static_cast<int>(log10(dbsize)) + 1;

	Xapian::doccount c = 0;
	Xapian::PostingIterator it = db_in.postlist_begin("");
	while (it != db_in.postlist_end("")) {
	    db_out.add_document(db_in.get_document(*it));

	    ++c;
	    // Update for the first 10, and then every 10th document.
	    if (c <= 10 || c % 10 == 0) {
		cout << '\r' << leaf << ": ";
		cout << setw(width) << c << '/' << dbsize << flush;
	    }

	    ++it;
	}

	cout << endl;
    }

    cout << "Flushing..." << flush;
    // Flush explicitly so that any error is reported.
    db_out.flush();
    cout << " done." << endl;
} catch (const Xapian::Error & e) {
    cerr << '\n' << argv[0] << ": " << e.get_description() << endl;
    exit(1);
}
