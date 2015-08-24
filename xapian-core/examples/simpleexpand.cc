/** @file simpleexpand.cc
 * @brief Simple example program demonstrating query expansion.
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
    // We require at least two command line arguments.
    if (argc < 3) {
	int rc = 1;
	if (argv[1]) {
	    if (strcmp(argv[1], "--version") == 0) {
		cout << "simpleexpand" << endl;
		exit(0);
	    }
	    if (strcmp(argv[1], "--help") == 0) {
		rc = 0;
	    }
	}
	cout << "Usage: " << argv[0] << " PATH_TO_DATABASE QUERY [-- [DOCID...]]" << endl;
	exit(rc);
    }

    // Open the database for searching.
    Xapian::Database db(argv[1]);

    // Start an enquire session.
    Xapian::Enquire enquire(db);

    // Combine command line arguments up to "--" with spaces between
    // them, so that simple queries don't have to be quoted at the shell
    // level.
    string query_string(argv[2]);
    argv += 3;
    while (*argv && strcmp(*argv, "--") != 0) {
	query_string += ' ';
	query_string += *argv++;
    }

    // Create an RSet with the listed docids in.
    Xapian::RSet rset;
    if (*argv) {
	while (*++argv) {
	    rset.add_document(atoi(*argv));
	}
    }

    // Parse the query string to produce a Xapian::Query object.
    Xapian::QueryParser qp;
    Xapian::Stem stemmer("english");
    qp.set_stemmer(stemmer);
    qp.set_database(db);
    qp.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    Xapian::Query query = qp.parse_query(query_string);
    cout << "Parsed query is: " << query.get_description() << endl;

    // Find the top 10 results for the query.
    enquire.set_query(query);
    Xapian::MSet matches = enquire.get_mset(0, 10, &rset);

    // Display the results.
    cout << matches.get_matches_estimated() << " results found:" << endl;

    for (Xapian::MSetIterator i = matches.begin(); i != matches.end(); ++i) {
	cout << i.get_rank() + 1 << ": " << i.get_percent() << "% docid=" << *i
	     << " [" << i.get_document().get_data() << "]\n\n";
    }

    // If no relevant docids were given, invent an RSet containing the top 5
    // matches (or all the matches if there are less than 5).
    if (rset.empty()) {
	int c = 5;
	Xapian::MSetIterator i = matches.begin();
	while (c-- && i != matches.end()) {
	    rset.add_document(*i);
	    ++i;
	}
    }

    // Generate an ESet containing terms that the user might want to add to
    // the query.
    Xapian::ESet eset = enquire.get_eset(10, rset);

    // List the terms.
    Xapian::ESetIterator t;
    for (t = eset.begin(); t != eset.end(); ++t) {
	cout << *t << ": weight = " << t.get_weight() << endl;
    }
} catch (const Xapian::Error &e) {
    cout << e.get_description() << endl;
    exit(1);
}
