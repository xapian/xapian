// cvsminesearch.C
//
// (c) 2001 Amir Michail (amir@users.sourceforge.net)

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

//
// Usage:  cvsminesearch package (# results) query_word1 query_word2 ... 
//
//               cvsminesearch (# results) query_word1 query_word2 ... takes list of packages from stdin
//
// Example:  cvssearch root0/db/kdeutils_kfind 10 ftp nfs
//
//     Returns the top 10 lines with both ftp and nfs.
//
// ($CVSDATA/package is the directory with the quartz database inside)
//

// For rules, you need to supply something like:  double buffering QPixmap=>
//
// This will look for rules starting with QPixmap.  
// 

#define ONLY_Q_AND_K_CLASSES 1

// actually, divide seems to work better for both

#warning "using diff for rules and div for single items"

#include <db_cxx.h>
#include <om/om.h>
#include <stdio.h>
#include <math.h>

#include "util.h"

void considerRule( Db& db, string rule_antecedent,  map< double, set< pair<string, string> > >& rules, const string& ant, int ant_count, const string& con, int con_count, int pair_supp, int total_transactions ) {
  //  cerr << "Considering " << ant << " => " << con << " with supp " << pair_supp << endl;
  //  cerr << "... ant has count " << ant_count << " and con has count " << con_count << endl;

  assert ( rule_antecedent != "" );

  if ( ant != rule_antecedent ) {
    return;
  }

  if ( ant.find("()") != -1 ) {
    return;
  }

  if ( ant+"()" == con || con+"()" == ant ) {
    return; // not too surprising
  }


  int importance = pair_supp;

  double conf = 100.0*(double)pair_supp / (double)ant_count;

  double surprise = 0.0;

  cerr << "Considering rule Query ^ " << ant << "=>" << con << " with conf " << conf << endl;

  cerr << "... importance " << importance << endl;

  Dbt key( (void*) con.c_str(), con.length()+1 );
  Dbt data;
  int rc = db.get( 0, &key, &data, 0 );
  if ( rc != DB_NOTFOUND ) {
    double conf_a = atof( (char*) data.get_data() );
#warning "surprise is diff"
    surprise = conf / conf_a;
    cerr << "... surprise " << surprise << endl;
    //      if ( surprise3 < MIN_SURPRISE ) {
    //	return; // skip rule
    //      }
  } else {
    cerr << "Could not find confidence for " << con << endl;
    assert(0);
  }




  //  double score = (1.0+surprise1)*(1.0+surprise2)*(1.0+surprise3); //*importance; // surprise * importance; // * importance; // surprise2 * importance;

  double score = surprise * importance;

  cerr << "... resulting score = " << score << endl;

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


    Db db(0,0);
    db.open( (cvsdata +"/root0/db/mining.db").c_str(),  0 , DB_HASH, DB_RDONLY, 0 );
    


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

    string rule_antecedent = "";
         
    for (int optpos = qpos; optpos < argc; optpos++) {

      string s = argv[optpos];
      if ( s.find("=>") != -1 ) {
	rule_antecedent = s.substr( 0, s.length()-2 );
	cerr << "** antecedent -" << rule_antecedent << "-" << endl;
	continue;
      }

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
    cerr << "... counting items" << endl;
    map< string, int > item_count;

    map< double, set<string> > function_ranking;
    map< double, set<string> > class_ranking;

    for( list< set<string> >::iterator t = transactions.begin(); t != transactions.end(); t++ ) {
      set<string> S = *t;
      for( set<string>::iterator s = S.begin(); s != S.end(); s++ ) {
	item_count[*s]++;
      }
    }

    for( map<string, int>::iterator i = item_count.begin(); i != item_count.end(); i++ ) {
      string k = i->first;

      /// compute confidence of Q => c/f for each class/function
      /// observe though that the number of transactions with Q is fixed
      /// across these rules..., so strictly speaking, it's not really required

      double c = 100.0*(double) i->second / (double) transactions.size();

      cerr << k << endl;
      cerr << "...conf " << c << endl;

      // divide by confidence of consequent alone
      Dbt key( (void*) k.c_str(), k.length()+1 );
      Dbt data;
      int rc = db.get( 0, &key, &data, 0 );
      if ( rc != DB_NOTFOUND ) {
	double conf_a = atof( (char*) data.get_data() );
	cerr << "...conf of consequent " << conf_a << endl;
	c = c / conf_a;
	cerr << "...conf imp " << c << endl;
      } else {
	cerr << "*** NOT FOUND " << k << endl;
	assert(0);
      }

      cerr << "... supp " << i->second << endl;
      c = c * (i->second); // multiply by support of (Q, con)

      cerr << "... score " << c << endl;

      if ( k.find("()") != -1 ) {
	function_ranking[-c].insert(k);
      } else {
	class_ranking[-c].insert(k);
      }
    }


    ///// print out item counts

    //// show only those items that we found in the database




    for( map< double, set<string> >::iterator i = class_ranking.begin(); i != class_ranking.end(); i++ ) {
      double score = - (i->first);
      set<string> S = i->second;
      for( set<string>::iterator s = S.begin(); s != S.end(); s++ ) {
	if ( ONLY_Q_AND_K_CLASSES ) {
	  if ( s->find("K") != 0 && s->find("Q") != 0 ) {
	    continue;
	  }
	}
	cerr << "class " << (*s) << " has score " << score << endl;
      }
    }

    for( map< double, set<string> >::iterator i = function_ranking.begin(); i != function_ranking.end(); i++ ) {
      double score = - (i->first);
      set<string> S = i->second;
      for( set<string>::iterator s = S.begin(); s != S.end(); s++ ) {
	cerr << "function " << (*s) << " has score " << score << endl;
      }
    }

    
    if ( rule_antecedent != "" ) {


    
      /// now, we count item pairs
      cerr << "... counting pairs" << endl;
      map< pair<string, string>, int> pair_count;

      for( list< set<string> >::iterator t = transactions.begin(); t != transactions.end(); t++ ) {
	set<string> S = *t;

	for( set<string>::iterator i1 = S.begin(); i1 != S.end(); i1++) {

	  if ( ONLY_Q_AND_K_CLASSES ) {
	    if ( i1->find("()") == -1 && i1->find("K") != 0 && i1->find("Q") != 0 ) {
	      continue;
	    }
	  }



	  for( set<string>::iterator i2 = i1; i2 != S.end(); i2++ ) {
	    if ( i1 == i2 ) {
	      continue;
	    }

	    if ( ONLY_Q_AND_K_CLASSES ) {
	      if ( i2->find("()") == -1 && i2->find("K") != 0 && i2->find("Q") != 0 ) {
		continue;
	      }
	    }


	    if ( rule_antecedent != "" ) {
	      if  ( (*i1) != rule_antecedent && (*i2) != rule_antecedent ) {
		continue;
	      }
	    }

	    assert( (*i1) < (*i2) );
	    pair_count[ make_pair(*i1, *i2) ] ++;
	  }
	}
      }


      map< double, set<pair<string, string> > > rules;

      cerr << "... generating rules" << endl;
      // okay, now generate rules
      for( map< pair<string, string >, int>::iterator p = pair_count.begin(); p != pair_count.end(); p++ ) {
	considerRule( db, rule_antecedent, rules, p->first.first, item_count[p->first.first], p->first.second, item_count[p->first.second], p->second, transactions.size() );
	considerRule( db, rule_antecedent, rules, p->first.second, item_count[p->first.second], p->first.first, item_count[p->first.first], p->second, transactions.size() );
      }

      for ( map< double, set< pair< string, string > > >::iterator i = rules.begin(); i != rules.end(); i++ ) {
	double score = -(i->first);
	//      cerr << "*** Score " << score << endl;
	set< pair< string, string> > S = i->second;
	for( set< pair< string, string > >::iterator p = S.begin(); p != S.end(); p++ ) {
	  pair<string, string> pair = *p;
	  cerr << score << ":  " << pair.first << " => " << pair.second << endl;
	}
      }

    }

    db.close(0);
    
         
  }
  catch(OmError & error) {
    cout << "Exception: " << error.get_msg() << endl;
  }  catch( DbException& e ) {
    cerr << "Exception:  " << e.what() << endl;
  }

}
