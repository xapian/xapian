/* simpleexpand.cc: Simplest possible query expansion
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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

int main(int argc, char *argv[])
{
    // Simplest possible options parsing: we just require two or more
    // parameters.
    if(argc < 3) {
	std::cout << "usage: " << argv[0] <<
		" <path to database> <search terms> -- <relevant docids>" <<
		std::endl;
	exit(1);
    }
    
    // Catch any OmError exceptions thrown
    try {
	// Make the database
	OmSettings settings;
	settings.set("backend", "quartz");
	settings.set("quartz_dir", argv[1]);
	OmDatabase db(settings);

	// Start an enquire session
	OmEnquire enquire(db);

	// Prepare the query terms
	std::vector<om_termname> queryterms;
	int optpos;
	for (optpos = 2; optpos < argc; optpos++) {
	    if(std::string(argv[optpos]) == "--") {
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
	    std::cout << "Performing query `" << query.get_description() <<
		    "'" << std::endl;

	    // Give the query object to the enquire session
	    enquire.set_query(query);

	    // Get the top 10 results of the query
	    matches = enquire.get_mset(0, 10, &reldocs);

	    // Display the results
	    std::cout << matches.get_matches_estimated() <<
		    " results found" << std::endl;

	    for (OmMSetIterator i = matches.begin();
		 i != matches.end();
		 i++) {
		std::cout << "Document ID " << *i << "\t" <<
			i.get_percent() << "% [" <<
			i.get_document().get_data().value << "]" << std::endl;
	    }
	}

	// Put the top 5 into the rset if rset is empty
	if(reldocs.size() == 0) {
	    OmMSetIterator i;
	    int j;
	    for (i = matches.begin(), j = 0;
		 (i != matches.end()) && (j < 5);
		 i++, j++) {
		reldocs.add_document(*i);
	    }
	}
	
	// Get the suggested expand terms
	OmESet eterms = enquire.get_eset(10, reldocs);

	// Display the expand terms
	std::cout << eterms.size() << " suggested additional terms" << std::endl;

	OmESetIterator k;
	for (k = eterms.begin(); k != eterms.end(); k++) {
	    std::cout << "Term `" << *k << "'\t " <<
		    "(weight " << k.get_weight() << ")" << std::endl;
	}
    }
    catch (const OmError &error) {
	std::cout << "Exception: "  << error.get_msg() << std::endl;
    }
}
