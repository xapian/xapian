/** @file copydatabase.cc
 * @brief Perform a document-by-document copy of one or more Xapian databases.
 */
/* Copyright (C) 2006,2007,2008,2009,2010,2011 Olly Betts
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

#include <cctype>
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
"  --help           display this help and exit\n"
"  --version        output version information and exit" << endl;
    exit(rc);
}

int
main(int argc, char **argv)
try {
    bool verbose = true;
    if (argc > 1 && argv[1][0] == '-') {
	if (strcmp(argv[1], "--help") == 0) {
	    cout << PROG_NAME " - " PROG_DESC "\n\n";
	    show_usage(0);
	}
	if (strcmp(argv[1], "--version") == 0) {
	    cout << PROG_NAME " - " PACKAGE_STRING << endl;
	    exit(0);
	}
    }

    if (argc < 2) show_usage(1);

    const char *dest = argv[argc - 1];
    Xapian::WritableDatabase db(dest, Xapian::DB_OPEN);

    // FIXME: Add this method?  Or some other name (clear_synonyms() clears
    // them for just one term; or maybe clear_synonyms() to clear them all?
    // db.clear_spellings();
    Xapian::TermIterator spellword = db.spellings_begin();
    while (spellword != db.spellings_end()) {
	db.remove_spelling(*spellword, spellword.get_termfreq());
	++spellword;
    }

    double total_len = db.get_avlength() * db.get_doccount();
    Xapian::termcount cf_threshold = Xapian::termcount(1e-6 * total_len);
    size_t c_added = 0, c_skipped = 0;
    for (auto t = db.allterms_begin(); t != db.allterms_end(); ++t) {
	const string& term = *t;
	unsigned char firstch = term[0];
	if (!isupper(firstch)) {
	    // Unprefixed term.
	    Xapian::termcount cf = db.get_collection_freq(term);
	    if (cf > cf_threshold) {
		++c_added;
		db.add_spelling(term, cf);
		if (verbose)
		    cout << "\"" << term << "\" added from body: "
			 << double(cf) / cf_threshold << '\n';
	    } else {
		++c_skipped;
		if (verbose)
		    cout << "\"" << term << "\" skipped from body: "
			 << double(cf) / cf_threshold << '\n';
	    }
	} else if (firstch == 'S') {
	    // Always add words from title.
	    Xapian::termcount cf = db.get_collection_freq(term);
	    db.add_spelling(term.substr(1), cf);
	    if (verbose) cout << "\"" << term << "\" added from title\n";
	}
    }

    cout << "Added " << c_added << " body terms, skipped " << c_skipped
	 << " body terms." << endl;

    cout << "Committing..." << flush;
    // Commit explicitly so that any error is reported.
    db.commit();
    cout << " done." << endl;
} catch (const Xapian::Error & e) {
    cerr << '\n' << argv[0] << ": " << e.get_description() << endl;
    exit(1);
}
