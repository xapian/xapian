/** @file
 * @brief Perform a document-by-document copy of one or more Xapian databases.
 */
/* Copyright (C) 2006-2022 Olly Betts
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

#include <initializer_list>
#include <iomanip>
#include <iostream>

#include <cmath> // For log10().
#include <cstdlib> // For exit().
#include <cstring> // For strcmp() and strrchr().

using namespace std;

#define PROG_NAME "copydatabase"
#define PROG_DESC "Perform a document-by-document copy of one or more Xapian databases"

static void
show_usage(int rc)
{
    cout << "Usage: " PROG_NAME " SOURCE_DATABASE... DESTINATION_DATABASE\n\n"
"Options:\n"
"  --no-renumber    Preserve the numbering of document ids (useful if you have\n"
"                   external references to them, or have set them to match\n"
"                   unique ids from an external source).  If multiple source\n"
"                   databases are specified and the same docid occurs in more\n"
"                   one, the last occurrence will be the one which ends up in\n"
"                   the destination database.\n"
"  --help           display this help and exit\n"
"  --version        output version information and exit\n";
    exit(rc);
}

int
main(int argc, char **argv)
try {
    bool renumber = true;
    if (argc > 1 && argv[1][0] == '-') {
	if (strcmp(argv[1], "--help") == 0) {
	    cout << PROG_NAME " - " PROG_DESC "\n\n";
	    show_usage(0);
	}
	if (strcmp(argv[1], "--version") == 0) {
	    cout << PROG_NAME " - " PACKAGE_STRING "\n";
	    exit(0);
	}
	if (strcmp(argv[1], "--no-renumber") == 0) {
	    renumber = false;
	    argv[1] = argv[0];
	    ++argv;
	    --argc;
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
	string src = argv[i];
	if (!src.empty()) {
	    // Remove any trailing directory separator.
	    char ch = src.back();
	    for (char dir_sep : DIR_SEPS_LIST) {
		if (ch == dir_sep) {
		    src.resize(src.size() - 1);
		    break;
		}
	    }
	}

	// Open the source database.
	Xapian::Database db_in(src);

	// Find the leaf-name of the database path for reporting progress.
	//
	// If we found a directory separator, + 1 advances to the next
	// character; If we didn't, incrementing string::npos will give us 0,
	// so we use the whole of src as the leaf-name.
	const char * leaf = src.c_str() + (src.find_last_of(DIR_SEPS) + 1);

	// Iterate over all the documents in db_in, copying each to db_out.
	Xapian::doccount dbsize = db_in.get_doccount();
	if (dbsize == 0) {
	    cout << leaf << ": empty!\n";
	} else {
	    // Calculate how many decimal digits there are in dbsize.
	    int width = static_cast<int>(log10(double(dbsize))) + 1;

	    Xapian::doccount c = 0;
	    Xapian::PostingIterator it = db_in.postlist_begin(string());
	    while (it != db_in.postlist_end(string())) {
		Xapian::docid did = *it;
		if (renumber) {
		    db_out.add_document(db_in.get_document(did));
		} else {
		    db_out.replace_document(did, db_in.get_document(did));
		}

		// Update for the first 10, and then every 13th document
		// counting back from the end (this means that all the
		// digits "rotate" and the counter ends up on the exact
		// total.
		++c;
		if (c <= 10 || (dbsize - c) % 13 == 0) {
		    cout << '\r' << leaf << ": ";
		    cout << setw(width) << c << '/' << dbsize << flush;
		}

		++it;
	    }

	    cout << '\n';
	}

	cout << "Copying spelling data..." << flush;
	Xapian::TermIterator spellword = db_in.spellings_begin();
	while (spellword != db_in.spellings_end()) {
	    db_out.add_spelling(*spellword, spellword.get_termfreq());
	    ++spellword;
	}
	cout << " done.\n";

	cout << "Copying synonym data..." << flush;
	Xapian::TermIterator synkey = db_in.synonym_keys_begin();
	while (synkey != db_in.synonym_keys_end()) {
	    string key = *synkey;
	    Xapian::TermIterator syn = db_in.synonyms_begin(key);
	    while (syn != db_in.synonyms_end(key)) {
		db_out.add_synonym(key, *syn);
		++syn;
	    }
	    ++synkey;
	}
	cout << " done.\n";

	cout << "Copying user metadata..." << flush;
	Xapian::TermIterator metakey = db_in.metadata_keys_begin();
	while (metakey != db_in.metadata_keys_end()) {
	    string key = *metakey;
	    db_out.set_metadata(key, db_in.get_metadata(key));
	    ++metakey;
	}
	cout << " done.\n";
    }

    cout << "Committing..." << flush;
    // Commit explicitly so that any error is reported.
    db_out.commit();
    cout << " done.\n";
} catch (const Xapian::Error & e) {
    cerr << '\n' << argv[0] << ": " << e.get_description() << '\n';
    exit(1);
}
