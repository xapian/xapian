/* simpleexpand.cc: Simplest possible query expansion
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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

#include <om/om.h>
#include <vector>

using namespace std;

int main(int argc, char *argv[])
{
    // Simplest possible options parsing: we just require two or more
    // parameters.
    if (argc < 3) {
	cout << "usage: " << argv[0]
	     << " <path to database> [<search terms>] [-- <relevant docids>]"
	     <<	endl;
	exit(1);
    }
    
    // Catch any OmError exceptions thrown
    try {
	// Make the database
	OmSettings settings;
	settings.set("backend", "auto");
	settings.set("auto_dir", argv[1]);
	OmDatabase db(settings);

	// Start an enquire session
	OmEnquire enquire(db);

	// Prepare the query terms
	vector<om_termname> queryterms;
	int optpos;
	for (optpos = 2; optpos < argc; optpos++) {
	    if (string(argv[optpos]) == "--") {
		optpos++;
		break;
	    }
	    queryterms.push_back(argv[optpos]);
	}

	// Prepare the relevant document set
	OmRSet reldocs;
	for (; optpos < argc; ++optpos) {
	    om_docid rdid = atoi(argv[optpos]);
	    if (rdid != 0) {
		reldocs.add_document(rdid);
	    }
	}

	OmMSet matches;
	if (!queryterms.empty()) {
	    // Build the query object
	    OmQuery query(OmQuery::OP_OR, queryterms.begin(), queryterms.end());

	    cout << "Performing query `" << query.get_description() << "'"
		 << endl;

	    // Give the query object to the enquire session
	    enquire.set_query(query);

	    // Get the top 10 results of the query
	    matches = enquire.get_mset(0, 10, &reldocs);

	    // Display the results
	    cout << matches.get_matches_estimated() << " results found" << endl;

	    for (OmMSetIterator i = matches.begin(); i != matches.end(); ++i) {
		cout << "ID " << *i << " " << i.get_percent() << "% ["
		     << i.get_document().get_data() << "]" << endl;
	    }
	}

	// Put the top 5 (at most) docs into the rset if rset is empty
	if (reldocs.empty()) {
	    OmMSetIterator i = matches.begin();
	    for (int j = 1; j < 5; ++j) {
		reldocs.add_document(*i);
		if (++i == matches.end()) break;
	    }
	}
	
	// Get the suggested expand terms
	OmESet eterms = enquire.get_eset(10, reldocs);

	// Display the expand terms
	cout << eterms.size() << " suggested additional terms" << endl;

	for (OmESetIterator k = eterms.begin(); k != eterms.end(); ++k) {
	    cout << "Term `" << *k << "'\t "
		 << "(weight " << k.get_weight() << ")" << endl;
	}
    }
    catch (const OmError &error) {
	cout << "Exception: "  << error.get_msg() << endl;
    }
}
