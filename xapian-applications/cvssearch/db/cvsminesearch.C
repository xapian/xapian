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


/////////// usage

// (1) no query words, no antecedent or consequent
//
//  simply returns most used classes & functions
//
// SCORE: % of commits that they are part of
//
//  SEE:  rank_all_items 

// (2) no query words, but have antecedent or consequent
//
// in this case, we consider rules A => B
//
//  SCORE:  log( conf(A=>B) / conf(B) ) x support(A,B)
//
//  Here conf(B) and support are computed with respect to the entire commit list.
//
//  SEE:  considerRule

// (3) query words, no antecedent or consequent
//
// returns classes/functions that tend to be used given the query Q
//
// SCORE:  log ((% of commits containing Q) / (% of all commits containing the item)) x support(A,Q)
//
// SEE:  rank_single_items


// (4) query words, but have antecedent or consequent
//
// in this case, we consider rules A => B
//
//  SCORE:  log( conf(A=>B) / conf(B) ) x support(A,B)
//
//  Here conf(B) is computed with respect to the entire commit list.
//  support(A,B) is computed only with respect to the commits containing Q.
//
//  SEE:  considerRule
//
//  NOTE:  We do not require confidence of rule to increase in presence of Q.







#include <db_cxx.h>
#include <om/om.h>
#include <stdio.h>
#include <math.h>

#include "util.h"



// For rules, you need to supply something like:  double buffering QPixmap=>
//
// This will look for rules starting with QPixmap.  
// 

#define ONLY_Q_AND_K_CLASSES 1


void rank_single_items(        map< string, int>& item_count,
			list< set<string > >& transactions,
			       int total_commit_transactions,
			 Db& db,
			map< double, set<string> >& class_ranking,
			map< double, set<string> >& function_ranking ) {
  for( map<string, int>::iterator i = item_count.begin(); i != item_count.end(); i++ ) {
      string k = i->first;

      /// compute confidence of Q => c/f for each class/function
      /// observe though that the number of transactions with Q is fixed
      /// across these rules..., so strictly speaking, it's not really required

      double c = 100.0*(double) i->second / (double) transactions.size();

      //      cerr << k << endl;
      //      cerr << "...conf " << c << endl;

      // divide by confidence of consequent alone
      Dbt key( (void*) k.c_str(), k.length()+1 );
      Dbt data;
      int rc = db.get( 0, &key, &data, 0 );
      bool significant = false;

      if ( rc != DB_NOTFOUND ) {
	int con_count = atoi( (char*) data.get_data() );
	double conf_a = 100.0*(double) con_count / (double) total_commit_transactions;
	//	cerr << "...conf of consequent " << conf_a << endl;
	c = log(c / conf_a);
	//	cerr << "...conf imp " << c << endl;
	
	/////// compute statistical significance
	//
	// n is total_commit_transactions
	// k is supp (i->second), that is # transactions containing Q & b
	// 
	// px = % of transactions containing Q
	// py = % of transactions containing b
	//
      } else {
	cerr << "*** NOT FOUND " << k << endl;
	assert(0);
      }



      c = c * (i->second); // multiply by support of (Q, con)
	
      if ( k.find("()") != -1 ) {
	function_ranking[-c].insert(k);
      } else {
	class_ranking[-c].insert(k);
      }
      //      } else {
      //      cerr << "SKIPPING " << k << "; not statistically significant" << endl;
      //      }

    }

}
void considerRule( int total_commit_transactions, Db& db, const string& rule_antecedent, const string& rule_consequent, map< double, set< pair<string, string> > >& rules, const string& ant, int ant_count, const string& con, int con_count, int pair_supp, int total_transactions ) {
  //  cerr << "Considering " << ant << " => " << con << " with supp " << pair_supp << endl;
  //  cerr << "... ant has count " << ant_count << " and con has count " << con_count << endl;

  //  cerr << "... rule antecedent " << rule_antecedent << endl;
  //  cerr << "... rule consequent " << rule_consequent << endl;


  assert ( rule_antecedent != "" || rule_consequent != "" );
  assert ( rule_antecedent == "" || rule_consequent == "" );

  if ( rule_antecedent != "" && ant != rule_antecedent ) {
    return;
  }

  if ( rule_consequent != "" && con != rule_consequent) {
    return;
  }

  

  if ( ant+"()" == con || con+"()" == ant ) {
    return; // not too surprising
  }


  int importance = pair_supp;

  double conf = 100.0*(double)pair_supp / (double)ant_count;

  double surprise = 0.0;

  //  cerr << "Considering rule Query ^ " << ant << "=>" << con << " with conf " << conf << endl;

  //  cerr << "... importance " << importance << endl;


  // computing statistical significance of rule
  
  // we need to estimate probability of finding consequent b in a transaction
  // (this we can get from our database)
  // 
  // we also need to estimate the probability of finding Q^a in a transcation
  // (this one actually requires us to know the total number of transactions, which
  // we do not know!)
  // 

  



  Dbt key( (void*) con.c_str(), con.length()+1 );
  Dbt data;
  int rc = db.get( 0, &key, &data, 0 );
  if ( rc != DB_NOTFOUND ) {
    int count = atoi( (char*) data.get_data() );
    double conf_a = 100.0* (double) count / (double) total_commit_transactions;
    //    cerr << "read conf_a = " << conf_a << endl;
    surprise = log(conf / conf_a);
    //    cerr << "... surprise " << surprise << endl;
    //      if ( surprise3 < MIN_SURPRISE ) {
    //	return; // skip rule
    //      }
  } else {
    cerr << "Could not find confidence for " << con << endl;
    assert(0);
  }




  //  double score = (1.0+surprise1)*(1.0+surprise2)*(1.0+surprise3); //*importance; // surprise * importance; // * importance; // surprise2 * importance;

  double score = surprise * importance;

  //  cerr << "... resulting score = " << score << endl;

  rules[ -score ].insert( make_pair( ant, con ) );

}

void generate_and_rank_rules( const string& rule_antecedent, const string& rule_consequent, map<string, int>& item_count, map<pair<string,string>, int>& pair_count, list<set<string> >& transactions,  int total_commit_transactions, Db& db, map< double, set<pair<string, string> > >& class_rule_ranking, map< double, set<pair<string, string> > >& function_rule_ranking ) {
   cerr << "... generating rules" << endl;
      // okay, now generate rules
      for( map< pair<string, string >, int>::iterator p = pair_count.begin(); p != pair_count.end(); p++ ) {
	string item1 = p->first.first;
	string item2 = p->first.second;
	int count1 = item_count[item1];
	int count2 = item_count[item2];

	if ( rule_antecedent != "" ) {
	  
	  if ( item1 == rule_antecedent ) {
	    // consider rule_antecedent => item2
	    if ( item2.find("()") == -1 ) {
	      // rule_antecedent => class
	      considerRule( total_commit_transactions, db, rule_antecedent, rule_consequent, class_rule_ranking, item1, count1, item2, count2, p->second, transactions.size() );
	    } else {
	      // rule_antecedent => function
	      considerRule( total_commit_transactions, db, rule_antecedent, rule_consequent, function_rule_ranking, item1, count1, item2, count2, p->second, transactions.size() );
	    }

	  } else {
	    // consider rule rule_antecedent => item1
	    if ( item1.find("()") == -1 ) {
	      // rule_antecedent => class
	      considerRule( total_commit_transactions, db, rule_antecedent, rule_consequent, class_rule_ranking, item2, count2, item1, count1, p->second, transactions.size() );
	    } else {
	      // rule_antecedent => function
	      considerRule( total_commit_transactions, db, rule_antecedent, rule_consequent, function_rule_ranking, item2, count2, item1, count1, p->second, transactions.size() );
	    }
	  }

	} else if ( rule_consequent != "" ) {
	  
	  if ( item1 == rule_consequent ) {
	    // consider item2 => rule_consequent
	    if ( item2.find("()") == -1 ) {
	      // class => rule_consequent
	      considerRule( total_commit_transactions, db, rule_antecedent, rule_consequent, class_rule_ranking, item2, count2, item1, count1, p->second, transactions.size() );
	    } else {
	      // function => rule_consequent
	      considerRule( total_commit_transactions, db, rule_antecedent, rule_consequent, function_rule_ranking, item2, count2, item1, count1, p->second, transactions.size() );
	    }

	  } else {
	    // consider rule item1 => rule_consequent
	    if ( item1.find("()") == -1 ) {
	      // class => rule_consequent
	      considerRule( total_commit_transactions, db, rule_antecedent, rule_consequent, class_rule_ranking, item1, count1, item2, count2, p->second, transactions.size() );
	    } else {
	      // function => rule_consequent 
	      considerRule( total_commit_transactions, db, rule_antecedent, rule_consequent, function_rule_ranking, item1, count1, item2, count2, p->second, transactions.size() );
	    }
	  }

	} else {
	  assert(0);
	}

      }

}



void rank_all_items( Db& db, int total_commit_transactions, map< double, set<string> >& class_ranking, map<double, set<string> >& function_ranking ) {
  
  // just iterate through all (K,I) pairs

  Dbc *cursor;
  db.cursor( NULL, &cursor, 0 );
  
  Dbt key;
  Dbt data;
  
  
  while(  cursor->get( &key, &data, DB_NEXT ) != DB_NOTFOUND ) {
    string k = (char*)key.get_data();
    double c = 100.0*(double)atoi((char*)data.get_data()) / (double) total_commit_transactions;
    //    cerr << k << " has count " << c << endl;

    if ( k.find("()") == -1 ) {
      class_ranking[-c].insert(k);
    } else {
      function_ranking[-c].insert(k);
    }
  }
  
  cursor->close();

}

void count_single_items( list< set<string> >& transactions, map<string, int>& item_count ) {
     cerr << "... counting items" << endl;
    for( list< set<string> >::iterator t = transactions.begin(); t != transactions.end(); t++ ) {
      set<string> S = *t;
      for( set<string>::iterator s = S.begin(); s != S.end(); s++ ) {
	if ( ONLY_Q_AND_K_CLASSES && s->find("()") == -1 ) {
	  if ( s->find("Q") != 0 && s->find("K") != 0 ) {
	    continue;
	  }
	}
	item_count[*s]++;
      }
    }
}

void count_item_pairs( list< set<string> >& transactions, const string& rule_antecedent, const string& rule_consequent,  map<pair<string,string>, int>& pair_count ) {

      cerr << "... counting pairs" << endl;


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

	    if ( rule_consequent != "" ) {
	      if  ( (*i1) != rule_consequent && (*i2) != rule_consequent ) {
		continue;
	      }
	    }

	    assert( (*i1) < (*i2) );
	    pair_count[ make_pair(*i1, *i2) ] ++;
	  }
	}
      }
}

void output_items( map< double, set<string> >& class_ranking, int max_results ) {
  int count = 0;
  for( map< double, set<string> >::iterator i = class_ranking.begin(); i != class_ranking.end(); i++ ) {
    double score = - (i->first);
    set<string> S = i->second;
    for( set<string>::iterator s = S.begin(); s != S.end(); s++ ) {
      cout << score << " " << (*s) << endl;
      count ++;
      if ( count == max_results ) {
	return;
      }
    }
  }
}

void output_rules( map< double, set<pair< string, string > > >& rule_ranking, int max_results ) {
  int count = 0;
  for ( map< double, set< pair< string, string > > >::iterator i = rule_ranking.begin(); i != rule_ranking .end(); i++ ) {
    double score = -(i->first);
    set< pair< string, string> > S = i->second;
    for( set< pair< string, string > >::iterator p = S.begin(); p != S.end(); p++ ) {
      pair<string, string> pair = *p;
      cout << score << " " <<  pair.first << "=>" << pair.second << endl;
      count ++;
      if ( count == max_results ) {
	return;
      }
    }
  }
}


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

  int max_results = atoi( argv[npos] );


  try {

    int total_commit_transactions = 0;
    ifstream in( (cvsdata +"/root0/db/mining.count").c_str() );
    in >> total_commit_transactions;
    in.close();
    cerr << "TOTAL COMMIT TRANSACTIONS " << total_commit_transactions << endl;

    Db db(0,0);
    db.open( (cvsdata +"/root0/db/mining.db").c_str(),  0 , DB_HASH, DB_RDONLY, 0 );
    


    // ----------------------------------------
    // code which accesses Omsee
    // ----------------------------------------
    OmDatabase database;
    OmDatabase database2;
         
    for( set<string>::iterator i = packages.begin(); i != packages.end(); i++ ) {
      OmSettings db_parameters;
      db_parameters.set("backend", "quartz");
      db_parameters.set("quartz_dir", cvsdata+"/"+(*i));
      database.add_database(db_parameters); // can search multiple databases at once
    }

    for( set<string>::iterator i = packages.begin(); i != packages.end(); i++ ) {
      OmSettings db_parameters;
      db_parameters.set("backend", "quartz");
      db_parameters.set("quartz_dir", cvsdata+"/"+(*i)+"2");
      database2.add_database(db_parameters); // can search multiple databases at once
    }
         
    // start an enquire session
    OmEnquire enquire(database);
    OmEnquire enquire2(database2);
         
    vector<om_termname> queryterms;
         
    OmStem stemmer("english");

    string rule_antecedent = "";
    string rule_consequent = "";
         
    for (int optpos = qpos; optpos < argc; optpos++) {

      string s = argv[optpos];

      if ( s.find("=>") != -1 ) {
	rule_antecedent = s.substr( 0, s.length()-2 );
	continue;
      }

      if ( s.find("<=") != -1 ) {
	rule_consequent = s.substr( 0, s.length()-2 );
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
     
    OmMSet matches;

    if ( ! queryterms.empty() ) {
      OmQuery query(OmQuery::OP_AND, queryterms.begin(), queryterms.end());
      
      //       cerr << "Performing query `" << query.get_description() << "'" << endl;
      
      enquire.set_query(query); // copies query object
      
      int num_results = 10000000; // = atoi( argv[npos] );

      matches = enquire.get_mset(0, num_results); // get top 10 matches
         
      cerr <<  matches.size() << " results found" << endl;

    } else {

      // no antecedent or consequent specified, no query terms specified
      if ( rule_antecedent == "" && rule_consequent == "" ) {
	map< double, set<string> > function_ranking;
	map< double, set<string> > class_ranking;
	rank_all_items(db, total_commit_transactions, class_ranking, function_ranking);
	output_items( class_ranking, max_results );
	output_items( function_ranking, max_results );
	db.close(0);
	return 0;
      }
      
      vector<om_termname> q;
      if ( rule_antecedent != "" ) {
	assert( rule_consequent == "" );
	q.push_back(rule_antecedent);
      } else if ( rule_consequent != "" ) {
	q.push_back(rule_consequent);
      } else {
	assert(0);
      }

      OmQuery query(OmQuery::OP_AND, q.begin(), q.end());
      
      //       cerr << "Performing query `" << query.get_description() << "'" << endl;
      
      enquire2.set_query(query); // copies query object
      
      int num_results = 10000000; // = atoi( argv[npos] );
      
      matches = enquire2.get_mset(0, num_results); // get top 10 matches
      
      cerr <<  matches.size() << " results found" << endl;
      
    }

    list< set<string> > transactions;

    //vector<OmMSetItem>::const_iterator i;
    for (OmMSetIterator i = matches.begin(); i != matches.end(); i++) {
      //	 cout << "Document ID " << i->did << "\t";
      int sim = matches.convert_to_percent(i);
      //      cout << sim << " ";
      OmDocument doc = i.get_document();
      string data = doc.get_data().value;
	    
      //      cout << data << endl; // data includes newline
	    
      list<string> symbols;
      split( data, " \n", symbols );

      set<string> S;
      for( list<string>::iterator s = symbols.begin(); s != symbols.end(); s++ ) {
	S.insert(*s);
      }	    
      transactions.push_back( S );
    }



    assert( total_commit_transactions > 0 );




    //////////////////////////////////// count single items

    map< string, int > item_count;
    count_single_items( transactions, item_count );

    if ( rule_antecedent == "" && rule_consequent == "" ) {

      // just show ranking of functions and classes, no rules

      map< double, set<string> > function_ranking;
      map< double, set<string> > class_ranking;

      rank_single_items( item_count, transactions, total_commit_transactions, db, class_ranking, function_ranking );

      output_items( class_ranking, max_results );
      output_items( function_ranking, max_results );
      
    } else {
      // show rules

      ///////////////////////////////// count item pairs
      map< pair<string, string>, int> pair_count;      
      
      count_item_pairs( transactions, rule_antecedent, rule_consequent, pair_count );

      map< double, set<pair<string,string> > > class_rule_ranking;
      map< double, set<pair<string,string> > > function_rule_ranking;
      generate_and_rank_rules( rule_antecedent, rule_consequent, item_count, pair_count, transactions, total_commit_transactions, db, class_rule_ranking, function_rule_ranking );
      
      output_rules( class_rule_ranking, max_results );
      output_rules( function_rule_ranking, max_results );
    }


    db.close(0);
    
         
  }
  catch(OmError & error) {
    cout << "Exception: " << error.get_msg() << endl;
  }  catch( DbException& e ) {
    cerr << "Exception:  " << e.what() << endl;
  }

}

















































///////////////////////////////////////////////////////////////////////////////////////////////
// stuff that was taken out

// importance of statistical significance:
//  key problem here is that np and nq may be too small for binomial approximation
// to work well and it is to expensive to compute the binomial coefficients
//
// right now, we compute surprise as c / c_0 
// however, if a pattern occurs only one transaction, and the consequent
// is very rare, then the surprise is very high
//
// two ways to get around this:
//
// (1) we require minimum support 
// 
// (2) we compute statistical significance of the rule
//
// The advantage of (2) is we require no arbitrary minimum support and
// we can be sure of statistical significance.
//

// To compute, statistical significance, we need to compute binomial
// coefficients.  
//
// See:  http://www.pads.uwaterloo.ca/Bruno.Preiss/books/opus5/html/page460.html
// Or search google for computing binomial coefficients
// 

// It is possible to use dynamic programming to compute (n choose m) in O(n^2) time

map< pair<int, int>, double > binom_cache;

double compute_binom(int n, int m) {
  pair<int, int> p(n, m);

  if ( binom_cache.find( p ) != binom_cache.end() ) {
    return binom_cache[p];
  }

  // not in cache, so compute it

  double *b = new double [n+1];

  b[0] = 1.0;

  for( int i = 1; i <= n; i++ ) {
    b[i] = 1.0;
    for ( int j = i-1; j > 0; j-- ) {
      b[j] += b[j-1];
    }
  }
  double ans = b[m];
  delete [] b;
  binom_cache[p] = ans;
  return ans;
}


// computing the cumulative normal distribution function
//
// http://www.math.nyu.edu/fellows_fin_math/laud/fall2000/ex0/ex0.html
//
// 


bool statistically_significant (int n, int k, double px, double py ) {
  //x  cerr << "compute significance called with n = " << n << " k = " << k << " px = " << px << " py = " << py << endl;

  // Let's try using the normal approximation.  We don't need the table, just
  // the cutoff value for the significance we want.

  double p = px*py;

  double mean = n*p; // Expected number of times for finding both items

  cerr << "np = " << n*p << " and nq = " << n*(1-p) << endl;

  if ( n*p <= 5 || (n*(1-p)) <= 5 ) {
    cerr << "WARNING:  normal not good here" << endl;


#if 0
    // if n*p <= 5, then we assume that the items do not cooccur sufficiently
    // often to be of interest, so we skip it...

    assert( n*(1-p) > 5 );

    return false;
#endif

  } else {
    cerr << "SUCCESS:  normal is good here" << endl;
  }

  double standard_deviation = sqrt( (double)n*p*(1-p) );
  
  //  cerr << "... mean " << mean << endl;
  //  cerr <<" ... standard deviation " << standard_deviation << endl;

  // let's calculate the area under the curve from 

  // P( X >= k-0.5 )

  double kc = (double)k-0.5;

  double skc = ( kc - mean ) / standard_deviation;
  
  //  cerr << "... skc = " << skc << endl;

  // compute area under the curve from skc nowards...
  // this probability is 0.5 - area from 0 .. skc

  // since we want the final results to be <= 0.05
  // this means that we want the area from 0..skc to be at least 0.45

  // to get that area, z must be at least 1.65

  if ( skc < 1.65 ) {
    //    cerr << "not statistically significant, skc < 1.65" << endl;
    return false;
  }
  
  //  cerr << "statistically significant, k >= 1.65" << endl;
  return true;
}


