/** @file xapian-metadata.cc
 * @brief Read and write user metadata
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

#include <config.h>

#include <xapian.h>

#include <iostream>
#include <string>

#include <cstdlib> // For exit().
#include <cstring>

using namespace std;

#define PROG_NAME "xapian-metadata"
#define PROG_DESC "Read and write user metadata"

static void show_usage() {
    cout << "Usage: " PROG_NAME " get PATH_TO_DATABASE KEY\n"
	    "       " PROG_NAME " list PATH_TO_DATABASE [PREFIX]\n"
	    "       " PROG_NAME " set PATH_TO_DATABASE KEY VALUE" << endl;
}

int
main(int argc, char **argv)
try {
    const char * command = argv[1];
    if (!command) {
syntax_error:
	show_usage();
	exit(1);
    }

    if (command[0] == '-') {
	if (strcmp(command, "--help") == 0) {
	    cout << PROG_NAME " - " PROG_DESC "\n\n";
	    show_usage();
	    exit(0);
	}
	if (strcmp(command, "--version") == 0) {
	    cout << PROG_NAME " - " PACKAGE_STRING << endl;
	    exit(0);
	}
    }

    if (strcmp(command, "get") == 0) {
	if (argc != 4) goto syntax_error;
	Xapian::Database db(argv[2]);
	cout << db.get_metadata(argv[3]) << endl;
    } else if (strcmp(command, "list") == 0) {
	if (argc != 3 && argc != 4) goto syntax_error;
	Xapian::Database db(argv[2]);
	string prefix;
	if (argc == 4) prefix = argv[3];
	for (Xapian::TermIterator t = db.metadata_keys_begin(prefix);
	     t != db.metadata_keys_end(prefix);
	     ++t) {
	    cout << *t << '\n';
	}
    } else if (strcmp(command, "set") == 0) {
	if (argc != 5) goto syntax_error;
	Xapian::WritableDatabase db(argv[2], Xapian::DB_CREATE_OR_OPEN);
	db.set_metadata(argv[3], argv[4]);
	db.commit();
    } else {
	goto syntax_error;
    }
} catch (const Xapian::Error &e) {
    cout << e.get_description() << endl;
    exit(1);
}
