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
//               cvssearch (# results) query_word1 query_word2 ... takes list of packages from stdin
//
// Example:  cvssearch root0/db/kdeutils_kfind 10 ftp nfs
//
//     Returns the top 10 lines with both ftp and nfs.
//
// ($CVSDATA/package is the directory with the quartz database inside)
//

#define SUPP_FRAC 0.05


#include <om/om.h>
#include <stdio.h>
#include <math.h>

#include "util.h"

void considerRule( map< double, set< pair<string, string> > >& rules, const string& ant, int ant_count, const string& con, int con_count, int pair_supp, int total_transactions ) {
  //  cerr << "Considering " << ant << " => " << con << " with supp " << pair_supp << endl;
  //  cerr << "... ant has count " << ant_count << " and con has count " << con_count << endl;

  int importance = pair_supp;

  double conf = 100.0*(double)pair_supp / (double)ant_count;

  double con_conf = 100.0*(double)con_count / (double) total_transactions;

  double surprise1 = conf / con_conf;

  double surprise2 = conf - con_conf;

  //  cerr << "... conf " << conf << endl;
  //  cerr << "... con_conf " << con_conf << endl;
  //  cerr << "... surprise1 " << surprise1 << endl;
  //  cerr << "... surprise2 " << surprise2 << endl;

  double score = surprise2 * importance; // * importance; // surprise2 * importance;

  rules[ -score ].insert( make_pair( ant, con ) );

}

int main(int argc, char *argv[]) {

  int min_supp = 1;
    
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
         
    int num_results = atoi( argv[npos] );
    assert( num_results > 0 );
         

    OmMSet matches = enquire.get_mset(0, num_results); // get top 10 matches
         
    cerr <<  matches.size() << " results found" << endl;

    min_supp = (int)(SUPP_FRAC * (double)matches.size());

    cerr << "min supp = " << min_supp << endl;


         

    list< set<string> > transactions;

    //vector<OmMSetItem>::const_iterator i;
    for (OmMSetIterator i = matches.begin(); i != matches.end(); i++) {
      //	 cout << "Document ID " << i->did << "\t";
      int sim = matches.convert_to_percent(i);
      cout << sim << " ";
      OmDocument doc = i.get_document();
      string data = doc.get_data().value;
	    
      cout << data << endl; // data includes newline
	    
      list<string> symbols;
      split( data, " ", symbols );

      set<string> S;
      for( list<string>::iterator s = symbols.begin(); s != symbols.end(); s++ ) {
	S.insert(*s);
      }	    
      transactions.push_back( S );
    }


    /// okay, let's count individual items first
    map< string, int > item_count;

    for( list< set<string> >::iterator t = transactions.begin(); t != transactions.end(); t++ ) {
      set<string> S = *t;
      for( set<string>::iterator s = S.begin(); s != S.end(); s++ ) {
	item_count[*s]++;
      }
    }
    
    /// now, we count item pairs
    map< pair<string, string>, int> pair_count;

    for( list< set<string> >::iterator t = transactions.begin(); t != transactions.end(); t++ ) {
      set<string> S = *t;

      for( set<string>::iterator i1 = S.begin(); i1 != S.end(); i1++) {
	if ( item_count[*i1] < min_supp ) {
	  continue;
	}

	for( set<string>::iterator i2 = i1; i2 != S.end(); i2++ ) {
	  if ( i1 == i2 || item_count[*i2] < min_supp ) {
	    continue;
	  }

	  assert( (*i1) < (*i2) );
	  pair_count[ make_pair(*i1, *i2) ] ++;
	}
      }
    }

    map< double, set<pair<string, string> > > rules;

    // okay, now generate rules
    for( map< pair<string, string >, int>::iterator p = pair_count.begin(); p != pair_count.end(); p++ ) {
      considerRule( rules, p->first.first, item_count[p->first.first], p->first.second, item_count[p->first.second], p->second, transactions.size() );
      considerRule( rules, p->first.second, item_count[p->first.second], p->first.first, item_count[p->first.first], p->second, transactions.size() );
    }

    for ( map< double, set< pair< string, string > > >::iterator i = rules.begin(); i != rules.end(); i++ ) {
      double score = -(i->first);
      cerr << "*** Score " << score << endl;
      set< pair< string, string> > S = i->second;
      for( set< pair< string, string > >::iterator p = S.begin(); p != S.end(); p++ ) {
	pair<string, string> pair = *p;
	cerr << pair.first << " => " << pair.second << endl;
      }
    }
    
         
  }
  catch(OmError & error) {
    cout << "Exception: " << error.get_msg() << endl;
  }
}
