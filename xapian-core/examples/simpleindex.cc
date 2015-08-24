/** @file simpleindex.cc
 * @brief Index each paragraph of a text file as a Xapian document.
 */
/* Copyright (C) 2007,2010,2015 Olly Betts
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <xapian.h>

#include <iostream>
#include <string>

#include <cstdlib> // For exit().
#include <cstring>

using namespace std;

int
main(int argc, char **argv)
try {
    if (argc != 2 || argv[1][0] == '-') {
	int rc = 1;
	if (argv[1]) {
	    if (strcmp(argv[1], "--version") == 0) {
		cout << "simpleindex" << endl;
		exit(0);
	    }
	    if (strcmp(argv[1], "--help") == 0) {
		rc = 0;
	    }
	}
	cout << "Usage: " << argv[0] << " PATH_TO_DATABASE\n"
		"Index each paragraph of a text file as a Xapian document." << endl;
	exit(rc);
    }

    // Open the database for update, creating a new database if necessary.
    Xapian::WritableDatabase db(argv[1], Xapian::DB_CREATE_OR_OPEN);

    Xapian::TermGenerator indexer;
    Xapian::Stem stemmer("english");
    indexer.set_stemmer(stemmer);

    string para;
    while (true) {
	string line;
	if (cin.eof()) {
	    if (para.empty()) break;
	} else {
	    getline(cin, line);
	}

	if (line.empty()) {
	    if (!para.empty()) {
		// We've reached the end of a paragraph, so index it.
		Xapian::Document doc;
		doc.set_data(para);

		indexer.set_document(doc);
		indexer.index_text(para);

		// Add the document to the database.
		db.add_document(doc);

		para.resize(0);
	    }
	} else {
	    if (!para.empty()) para += ' ';
	    para += line;
	}
    }

    // Explicitly commit so that we get to see any errors.  WritableDatabase's
    // destructor will commit implicitly (unless we're in a transaction) but
    // will swallow any exceptions produced.
    db.commit();
} catch (const Xapian::Error &e) {
    cout << e.get_description() << endl;
    exit(1);
}
