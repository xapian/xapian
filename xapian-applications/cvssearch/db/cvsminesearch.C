// cvsminesearch.C
//
// (c) 2001 Amir Michail (amir@users.sourceforge.net)

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

#define MIN_SUPPORT 5

// try convinction instead of interest measure (convinction is directional)
//
// See:
//
// http://citeseer.nj.nec.com/brin97dynamic.html
//

// confidence: P(A&B)/P(A)

// interest: P(A&B)/(P(A)*P(B) [completely symmetric]

// convinction: P(A)P(not B) / P(A, not B)
//
// Intuition:  logically, A=>B can be rewritten as ~(A & ~B), so we can see
//                 how far A&~B deviates from independence, and invert the ratio
//                 to take care of the outside negation.
//
//                 Unlike confidence, convinction factors in both P(A) and P(B) and 
//                 always has a value of 1 when the relvant items are completely unrelated
//
//                 Unlike interest, rules which hold 100% of the time have the highest possible
//                 convinction value of infinity.  (While confidence has this property, interest does
//                 not.)

// Basically, we proceed as before finding frequent item sets.
//
// Say we are considering A=>B.  We want to know:
//
// * % of transactions without B 
//
// * % of transactions with A 
//
// * % of transactions with A but not B (count of transactions with A - (A,B) count )
//
// 


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


/////////// TODO:  output commit information along with every time you print


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

int find_symbol_count( Db& db, const string& k ) {
  Dbt key( (void*) k.c_str(), k.length()+1 );
  Dbt data;
  int rc = db.get( 0, &key, &data, 0 );
  if ( rc != DB_NOTFOUND ) {
    return atoi( (char*) data.get_data() );
  }
  cerr << "*** NOT FOUND " << k << endl;
  assert(0);
}

void generate_rules( Db& db, const string& rule_antecedent, const string& rule_consequent,
		     map<string, int>& relative_item_count,
		     int transactions_returned,
		     int total_commit_transactions,
		     map<double, set<string> >& class_ranking,
		     map<double, set<string> >& function_ranking ) {
  
  string required_symbol;
  if ( rule_antecedent != "" ) {
    required_symbol = rule_antecedent;
  } else {
    required_symbol = rule_consequent;
  }

  for( map<string, int>::iterator i = relative_item_count.begin(); i != relative_item_count.end(); i++ ) {

    string symbol = i->first;
    int a_and_b_count = i->second;

    if ( a_and_b_count < MIN_SUPPORT ) {
      continue;
    }

    if ( required_symbol != "" && symbol == required_symbol ) {
      continue;
    }

    // generate rule of the form a=>b
    int a_count = 0;
    int b_count = 0;
    int not_b_count = 0;
    int a_and_not_b_count = 0;

    int symbol_count = find_symbol_count( db, symbol );

    if ( rule_consequent != "" ) {
      a_count = symbol_count;
      b_count = transactions_returned;
    } else {
      // normally, we would be here...
      a_count = transactions_returned; // if only query terms specified, this case applies also
      b_count = symbol_count;
    }
    
    not_b_count = total_commit_transactions - b_count;
    a_and_not_b_count = a_count - a_and_b_count;

    double a_and_not_b_prob = (double) a_and_not_b_count / (double) total_commit_transactions;
    double a_prob = (double) a_count / (double) total_commit_transactions;
    double not_b_prob = (double) not_b_count / (double) total_commit_transactions;

    double convinction = (a_prob * not_b_prob) / ( a_and_not_b_prob );

    string item;
    if ( rule_consequent != "" ) {
      item = "<=";
    } else {
      item = "=>";
    }

    item += symbol;

    if ( item.find("()") != -1 ) {
      function_ranking[ -convinction ].insert(item);
    } else {
      class_ranking[ -convinction ].insert(item);
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
	item_count[*s]++;
      }
    }
}

void output_items( map< double, set<string> >& class_ranking, int max_results ) {
  int count = 0;
  for( map< double, set<string> >::iterator i = class_ranking.begin(); i != class_ranking.end(); i++ ) {
    double score =  -(i->first);
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
         
    for( set<string>::iterator i = packages.begin(); i != packages.end(); i++ ) {
      OmSettings db_parameters;
      db_parameters.set("backend", "quartz");
      db_parameters.set("quartz_dir", cvsdata+"/"+(*i));
      database.add_database(db_parameters); // can search multiple databases at once
    }

    // start an enquire session
    OmEnquire enquire(database);
         
    vector<om_termname> queryterms;
         
    OmStem stemmer("english");

    string rule_antecedent = "";
    string rule_consequent = "";
         
    for (int optpos = qpos; optpos < argc; optpos++) {

      string s = argv[optpos];

      if ( s.find("=>") != -1 ) {
	rule_antecedent = s.substr( 0, s.length()-2 );
	queryterms.push_back("$"+rule_antecedent);
      } else if ( s.find("<=") != -1 ) {
	rule_consequent = s.substr( 0, s.length()-2 );
	queryterms.push_back("$"+rule_consequent);
      } else {
	om_termname term = s;
	lowercase_term(term);
	term = stemmer.stem_word(term);
	queryterms.push_back(term);
	cout << term << " ";
      }
    }
    cout << endl;
     
    OmMSet matches;

    if ( ! queryterms.empty() ) {

      OmQuery query(OmQuery::OP_AND, queryterms.begin(), queryterms.end());
      enquire.set_query(query); 
      int num_results = 10000000;
      matches = enquire.get_mset(0, num_results); 
      cerr <<  matches.size() << " results found" << endl;
    } else {
      assert( rule_antecedent == "" && rule_consequent == "" );
      map< double, set<string> > function_ranking;
      map< double, set<string> > class_ranking;
      rank_all_items(db, total_commit_transactions, class_ranking, function_ranking);
      output_items( class_ranking, max_results );
      output_items( function_ranking, max_results );
      db.close(0);
      return 0;
    }

    list< set<string> > transactions_returned;
    for (OmMSetIterator i = matches.begin(); i != matches.end(); i++) {
      int sim = matches.convert_to_percent(i);
      OmDocument doc = i.get_document();
      string data = doc.get_data().value;
      //      cerr << endl << data << endl;
      list<string> symbols;
      split( data, " \n", symbols );
      set<string> S;
      for( list<string>::iterator s = symbols.begin(); s != symbols.end(); s++ ) {
	S.insert(*s);
      }	    
      transactions_returned.push_back( S );
    }

    assert( total_commit_transactions > 0 );

    // the consequent of our rules contains only one item
    // so we need only count single items

    //////////////////////////////////// count single items

    map< string, int > relative_item_count; // the count is relative to the transactions returned
    count_single_items( transactions_returned, relative_item_count );

    // at this point we can generate rules
    map< double, set<string> > function_ranking;
    map< double, set<string> > class_ranking;

    generate_rules( db, rule_antecedent, rule_consequent, relative_item_count, 
		    transactions_returned.size(),
		    total_commit_transactions, class_ranking, function_ranking );

    output_items( class_ranking, max_results );
    output_items( function_ranking, max_results );
    
    db.close(0);
    
         
  }
  catch(OmError & error) {
    cout << "Exception: " << error.get_msg() << endl;
  }  catch( DbException& e ) {
    cerr << "Exception:  " << e.what() << endl;
  }

}


































