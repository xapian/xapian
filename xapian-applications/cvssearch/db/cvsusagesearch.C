// cvsusagesearch.C
//
// (c) 2001 Amir Michail (amir@users.sourceforge.net)

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

//
// Usage:  cvsusagesearch package (# results) query_word1 query_word2 ... 
//
//               cvsusagesearch (# results) query_word1 query_word2 ... takes list of packages from stdin
//
// Example:  cvsusagesearch root0/db/kdeutils_kfind 10 ftp nfs
//
//     Returns the top 10 lines with both ftp and nfs.
//
//                  cvsusagesearch global_classes 10 sound 
//
//    Returns the top 10 lines for library matches from global_classes package...
//
// ($CVSDATA/package is the directory with the quartz database inside)
//


#include <om/om.h>
#include <stdio.h>
#include <math.h>
#include <map>

#include "util.h"

int main(int argc, char *argv[]) {
    
  if(argc < 3) {
    cout << "Usage: " << argv[0] <<
      " <path to database> <search terms>" << endl;
    exit(1);
  }
    
  string cvsdata = get_cvsdata();
    
  set<string> packages;

  int qpos;
  int npos;
     
  // ----------------------------------------
  // get packages from cmd line or from file
  // ----------------------------------------
  if ( isdigit(argv[1][0] )) {
    // ----------------------------------------
    // get packages from file
    // ----------------------------------------
    string p;
    while (!cin.eof()) {
      cin >> p;
      packages.insert(p);
    }
    // ----------------------------------------
    // num_output param position 
    // query param position
    // ----------------------------------------
    npos = 1;
    qpos = 2;
  } else {
    // ----------------------------------------
    // get a package from cmd line
    // ----------------------------------------
    packages.insert( argv[1] );
    npos = 2;
    qpos = 3;
  }

  try {
    // ----------------------------------------
    // code which accesses Omsee
    // ----------------------------------------
    OmDatabase databases;
         
    for( set<string>::iterator i = packages.begin(); i != packages.end(); i++ ) {
      OmSettings db_parameters;
      db_parameters.set("backend", "quartz");
      db_parameters.set("quartz_dir", cvsdata+"/"+(*i));
      databases.add_database(db_parameters); // can search multiple databases at once
    }
         
    // start an enquire session
    OmEnquire enquire(databases);
         
    vector<om_termname> queryterms;
         
    OmStem stemmer("english");
         
    for (int optpos = qpos; optpos < argc; optpos++) {
      om_termname term = argv[optpos];
      lowercase_term(term);
      term = stemmer.stem_word(term);
      queryterms.push_back(term);
      cout << term;
      if ( optpos < argc-1 ) {
	cout << " ";
      }
    }
    cout << endl;
         
    OmQuery query(OmQuery::OP_AND, queryterms.begin(), queryterms.end());
         
    //       cerr << "Performing query `" << query.get_description() << "'" << endl;
         
    enquire.set_query(query); // copies query object
         
    int num_results = 999999; // = atoi( argv[npos] );
    assert( num_results > 0 );
         
    OmMSet matches = enquire.get_mset(0, num_results); // get top 10 matches

    map< double, set<string> > results;	
         
    for (OmMSetIterator i = matches.begin(); i != matches.end(); i++) {
      //	 cout << "Document ID " << i->did << "\t";
      int sim = matches.convert_to_percent(i);
      //      cout << sim << " ";
      OmDocument doc = i.get_document();
      string data = doc.get_data().value;

      int imp = 0;
      imp = atoi( data.substr( 0, data.find(" ") ).c_str() );
      assert( imp > 0 );

      static char str[4096];
      sprintf(str, "%d", sim);
      data = string(str)+" " +data;

      //      cout << data << endl; 
      //      cout << "...sim = " << sim << " and imp = " << imp<<  endl;
      //  results[ -sim*log(1.0+imp)].insert(data);
      results[ -sim ].insert(data);
    }


    int results_to_return = atoi(argv[npos]);
    int count = 0;
    for( map<double, set<string> >::iterator i = results.begin(); i != results.end(); i++ ) {
      double score = -(i->first);
      set<string> S = i->second;
      for( set<string>::iterator s = S.begin(); s != S.end(); s++ ) {
	cout << score << " " << *s << endl;
	count++;
	if ( count == results_to_return ) {
	  goto done;
	}
      }
    }
  done: ;
         
  }
  catch(OmError & error) {
    cout << "Exception: " << error.get_msg() << endl;
  }
}
