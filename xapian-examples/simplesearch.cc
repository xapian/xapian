/* simplesearch.cc: Simple command-line search program
 * (See quest for a more complete example with sophisticated query parsing).
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

#include <xapian.h>

#include <iostream>
#include <string>
#include <vector>

using namespace Xapian;
using namespace std;

int main(int argc, char **argv)
{
    // Simplest possible options parsing: we just require two or more
    // parameters.
    if (argc < 3) {
	cout << "usage: " << argv[0] <<
		" <path to database> <search terms>" << endl;
	exit(1);
    }
    
    // Catch any Error exceptions thrown
    try {
	// Open the database
	Database db(Auto::open(argv[1]));

	// Start an enquire session
	Enquire enquire(db);

	Stem stemmer("english");
	vector<string> stemmed_terms;

	argv += 2;
	while (*argv) {
	    stemmed_terms.push_back(stemmer.stem_word(*argv++));
	}
	    
	// Build a query by OR-ing together all the terms
	Query query(Query::OP_OR, stemmed_terms.begin(), stemmed_terms.end());
	cout << "Performing query `" << query.get_description() << "'" << endl;

	// Give the query object to the enquire session
	enquire.set_query(query);

	// Get the top 10 results of the query
	MSet matches = enquire.get_mset(0, 10);

	// Display the results
	cout << matches.get_matches_estimated() << " results found" << endl;

	for (MSetIterator i = matches.begin(); i != matches.end(); ++i) {
	    cout << "ID " << *i << " " << i.get_percent() << "% ["
		 << i.get_document().get_data() << "]" << endl;
	}
    } catch (const Error &error) {
	cout << "Exception: "  << error.get_msg() << endl;
	exit(1);
    }
}
