/* simpleexpand.cc: Simplest possible query expansion
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

int main(int argc, char *argv[])
{
    // Simplest possible options parsing: we just require two or more
    // parameters.
    if(argc < 3) {
	cout << "usage: " << argv[0] <<
		" <path to database> <search terms> -- <relevant docids>" <<
		endl;
	exit(1);
    }
    
    // Catch any OmError exceptions thrown
    try {
	// Make the database group
	OmSettings settings;
	settings.set("backend", "sleepycat");
	settings.set("sleepy_dir", argv[1]);
	OmDatabaseGroup databases;
	databases.add_database(settings);

	// Start an enquire session
	OmEnquire enquire(databases);

	// Prepare the query terms
	vector<om_termname> queryterms;
	int optpos;
	for (optpos = 2; optpos < argc; optpos++) {
	    if(string(argv[optpos]) == "--") {
		optpos++;
		break;
	    }
	    queryterms.push_back(argv[optpos]);
	}

	// Prepare the relevant document list
	OmRSet reldocs;
	for(; optpos < argc; optpos++) {
	    om_docid rdid = atoi(argv[optpos]);
	    if(rdid != 0) {
		reldocs.add_document(rdid);
	    }
	}

	// Build the query object
	OmQuery query(OmQuery::OP_OR, queryterms.begin(), queryterms.end());

	OmMSet matches;
	if(query.is_defined()) {
	    cout << "Performing query `" << query.get_description() <<
		    "'" << endl;

	    // Give the query object to the enquire session
	    enquire.set_query(query);

	    // Get the top 10 results of the query
	    matches = enquire.get_mset(0, 10);

	    // Display the results
	    cout << matches.items.size() << " results found" << endl;

	    for (vector<OmMSetItem>::const_iterator i = matches.items.begin();
		 i != matches.items.end();
		 i++) {
		OmDocument doc = enquire.get_doc(*i);
		cout << "Document ID " << i->did << "\t" <<
			matches.convert_to_percent(*i) << "% [" <<
			doc.get_data().value << "]" << endl;
	    }
	}

	// Put the top 5 into the rset if rset is empty
	if(reldocs.items.size() == 0) {
	    vector<OmMSetItem>::const_iterator i;
	    int j;
	    for (i = matches.items.begin(),
		 j = 0;
		 (i != matches.items.end()) && (j < 5);
		 i++, j++) {
		reldocs.add_document(i->did);
	    }
	}
	
	// Get the suggested expand terms
	OmESet eterms = enquire.get_eset(10, reldocs);

	// Display the expand terms
	cout << eterms.items.size() << " suggested additional terms" << endl;

	for (vector<OmESetItem>::const_iterator k = eterms.items.begin();
	     k != eterms.items.end();
	     k++) {
	    cout << "Term `" << k->tname << "'\t " <<
		    "(weight " << k->wt << ")" << endl;
	}
    }
    catch(OmError &error) {
	cout << "Exception: "  << error.get_msg() << endl;
    }
}
