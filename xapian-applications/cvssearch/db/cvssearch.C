// cvssearch.C
//
// (c) 2001 Amir Michail (amir@users.sourceforge.net)

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

//
// Usage:  cvssearch package (# results) query_word1 query_word2 ... 
//
// Example:  cvssearch kdeutils_kfind 10 ftp nfs
//
//     Returns the top 10 lines with both ftp and nfs.
//
// (package is the directory with the quartz database inside)
//


#include <om/om.h>
#include <stdio.h>
#include <math.h>

#include "util.h"

int main(int argc, char *argv[]) {
     if(argc < 3) {
        cout << "Usage: " << argv[0] <<
                " <path to database> <search terms>" << endl;
        exit(1);
    }

     try {
       // code which accesses Omsee
       OmDatabase databases;
       OmSettings db_parameters;
       db_parameters.set("backend", "quartz");
       db_parameters.set("quartz_dir", argv[1]);
       databases.add_database(db_parameters); // can search multiple databases at once

       // start an enquire session
       OmEnquire enquire(databases);

       vector<om_termname> queryterms;
       
       OmStem stemmer("english");

       for (int optpos = 3; optpos < argc; optpos++) {
	 om_termname term = argv[optpos];
	 lowercase_term(term);
	 term = stemmer.stem_word(term);
	 queryterms.push_back(term);
       }

       OmQuery query(OmQuery::OP_AND, queryterms.begin(), queryterms.end());

       //       cerr << "Performing query `" << query.get_description() << "'" << endl;

       enquire.set_query(query); // copies query object

       int num_results = atoi( argv[2] );
       assert( num_results > 0 );

       OmMSet matches = enquire.get_mset(0, num_results); // get top 10 matches

       //       cout << matches.items.size() << " results found" << endl;

       vector<OmMSetItem>::const_iterator i;
       for (i = matches.items.begin(); i != matches.items.end(); i++) {
	 //	 cout << "Document ID " << i->did << "\t";
	 int sim = matches.convert_to_percent(*i);
	 cout << sim << " ";
	 OmDocument doc = enquire.get_doc(*i);
	 string data = doc.get_data().value;
	 cout << data << endl; // data includes newline
       }

     }
     catch(OmError & error) {
       cout << "Exception: " << error.get_msg() << endl;
     }
     
}
