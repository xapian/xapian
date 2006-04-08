// cvsminesearch.C
//
// (c) 2001 Amir Michail (amir@users.sourceforge.net)

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// support of 1 is ok for =>, but probably not for <=>, <= !!!
// however, might want to keep it at 1 for => so we can handle queries
// with few transactions


#warning "support only applies for <=>; we use 1 for =>"
#define MIN_SUPPORT 10 

// <= and <=> give identical rankings!!

/**

It turns out that both <= and <=> give the same rankings!!

I changed the UI to only have => and <=>.

Here's why they are the same:

Let the query be Q.

We are looking for rules like:

 Q <=> symbol   Q <= symbol

Ranking formulas:

The first has formula P(A&B)/(P(A)*P(B)) where P(A) is the prob of finding Q 
in a commit and P(B) is the probability of finding the symbol in a commit.

The second has formula (P(~A)P(B))/P(~A&B).

Now, since the query is fixed, these formulas reduce to:

P(A&B)/P(B) *

and

P(B)/P(~A&B)

But P(~A&B) = P(B)-P(A&B), so the second formula
becomes 

P(B) / ( P(B) - P(A&B))

dividing by P(B) on top and bottom gives

1 / (1 - P(A&B)/P(B))  **

But * and ** give the same rankings (but different absolute numbers).

To see this observe 0<=P(A&B)/P(B)<=1.

Notice 1/(1-x1) < 1/(1-x2) <=> 1-x2 < 1-x1 <=> x1 < x2  for 0 <= x1 <=1 and 0 
<= x2 <= 1, so the rankings are unchanged.
**/


#warning "DOES NOT USE STOP WORDS"

#warning "test double buffering query; see if it still gives Button, box, etc."
#warning "test if namespace stuff is working, e.g., try searching for plugin or part"

// remaining problems
// 
// parts -> yields "General", doesn't show KParts::..
//
// sound -> yields Options, Buffer, ... where do these come from?


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


// Examples:
//
//   cvsminesearch root0/db/mining.om 10
//   cvsminesearch root0/db/mining.om 10 drag drop =>
//   cvsminesearch root0/db/mining.om 10 :QdropEvent =>
//   cvsminesearch root0/db/mining.om 10 drag drop :QDropEvent =>
//
//   cvsminesearch root0/db/mining.om 10 drag drop (without arrow)
//      just returns commits with drag drop in comments
//


/////////// TODO:  output commit information along with every time you print


/////////// usage (IMPORTANT:  prefix class/function names with :)

// (1) no query words, no antecedent or consequent
//
//  simply returns most used classes & functions
//

// (2) no query words, but have antecedent or consequent
//
// in this case, we consider rules A => B
//

// (3) query words, no antecedent or consequent
//
// returns classes/functions that tend to be used given the query Q
//

// (4) query words, but have antecedent or consequent
//
// in this case, we consider rules Q^A => B
//

#include <xapian.h>
#include <db_cxx.h>
#include <stdio.h>
#include <math.h>

#include "util.h"

unsigned int find_symbol_count( Db& db, const string & k ) {
  Dbt key( (void*) k.c_str(), k.length()+1 );
  Dbt data;
  int rc = db.get( 0, &key, &data, 0 );
  if ( rc != DB_NOTFOUND ) {
    return atoi( (char*) data.get_data() );
  }
  cerr << "*** Not Found " << k << endl;
  assert(0);
}



double compute_idf( Db& db, const string& k, const int N ) {
  unsigned int count = find_symbol_count( db, k );
  //  cerr << "compute idf N=" << N << " and count = " << count << " for " << k << endl;
  double idf = log( (double) N / (double) count - 1 );
  //cerr << "idf = " << idf << endl;
  return idf;
}



double compute_convinction( unsigned int a_count, 
			    unsigned int b_count, 
                            unsigned int a_and_b_count, 
	                    unsigned int total_commit_transactions ) {

  unsigned int not_b_count = 0;
  unsigned int a_and_not_b_count = 0;

  not_b_count = total_commit_transactions - b_count;
  a_and_not_b_count = a_count - a_and_b_count;

  double a_and_not_b_prob = (double) a_and_not_b_count / (double) total_commit_transactions;
  double a_prob = (double) a_count / (double) total_commit_transactions;
  double not_b_prob = (double) not_b_count / (double) total_commit_transactions;

  double b_prob = (double) b_count / (double) total_commit_transactions;

  double convinction = (a_prob * not_b_prob) / ( a_and_not_b_prob );

  //#warning "dividing convinction by prob of consequent"
  //cerr << "..convinction " << convinction << endl;
  //cerr << "..prob b " << b_prob << endl;


  //convinction = convinction / log(1.0+log(1.0+b_prob));

  return convinction;
}


void generate_rules( Db& db,
                     const set<string> & ranking_system,
                     const set<string>& query_symbols,
                     const map<string, int >& relative_item_count,
                     unsigned int transactions_returned,
                     unsigned int total_commit_transactions,
                     map<double, set<string> >& class_ranking,
                     map<double, set<string> >& function_ranking )
{
  assert( ! ranking_system.empty() );

  for( map<string, int >::const_iterator i = relative_item_count.begin();
       i != relative_item_count.end(); ++i)
    {
      string symbol = i->first;
      //cerr << "Considering " << symbol << endl;
      unsigned int a_and_b_count = i->second;

      if ( (ranking_system.find( "<=>" ) != ranking_system.end()) && a_and_b_count < MIN_SUPPORT ) {
	continue;
      }

      assert( a_and_b_count >= 1 );

      if ( query_symbols.find(":"+symbol) != query_symbols.end() ) {
	continue;
      }

      double score = -1.0; // the higher the better
      string prefix = "";

      if ( ranking_system.find( "<=>" ) != ranking_system.end() ) {

	// use interest measure P(A&B)/(P(A)*P(B)) which is symmetric
	unsigned int symbol_count = find_symbol_count( db, symbol );
	double A_prob = (double) symbol_count / (double) total_commit_transactions;
	double B_prob = (double) transactions_returned / (double) total_commit_transactions;
	double A_and_B_prob = (double) a_and_b_count / (double) total_commit_transactions;

	double interest = A_and_B_prob / ( A_prob * B_prob );

	score = interest;
	//#warning "taking log of interest now"
	//            score = log( 1.0 + interest );
	assert( score >= 0.0 );
	prefix = "<=>";

      } 

 
      if ( ranking_system.find( "=>" ) != ranking_system.end() ) {


	// generate rule of the form a=>b
	unsigned int a_count = 0;
	unsigned int b_count = 0;
	unsigned int symbol_count = find_symbol_count( db, symbol );

	// normally, we would be here...
	a_count = transactions_returned; // if only query terms specified, this case applies also
	b_count = symbol_count;

	double convinction = compute_convinction( a_count, b_count, a_and_b_count, total_commit_transactions );

	if ( score < 0.0 ) {
	  score = convinction;
	  prefix = "=>";
	} else {
	  score = score * convinction;
	  prefix +="*=>";
	}

      }

      if ( ranking_system.find( "<=" ) != ranking_system.end() ) {
	// generate rule of the form a<=b
	unsigned int a_count = 0;
	unsigned int b_count = 0;
	unsigned int symbol_count = find_symbol_count( db, symbol );

	a_count = symbol_count;
	b_count = transactions_returned;

	double convinction = compute_convinction( a_count, b_count, a_and_b_count, total_commit_transactions );

	if ( score < 0.0 ) {
	  score = convinction;
	  prefix = "<=";
	} else {
	  score = score * convinction;
	  prefix += "*<=";
	}

      }

      //      cerr << "symbol " << symbol << " has idf " << compute_idf( db, symbol, total_commit_transactions ) << endl;


      string item = prefix + symbol;
	
      if ( item.find("()") != string::npos ) {
	function_ranking[ -score ].insert(item);
      } else {
	class_ranking[ -score ].insert(item);
      }

    }
}


void rank_all_items( Db& db,
                     unsigned int total_commit_transactions,
                     map<double, set<string> >& class_ranking,
                     map<double, set<string> >& function_ranking )
{
  // just iterate through all (K,I) pairs

  Dbc *cursor;
  db.cursor( NULL, &cursor, 0 );

  Dbt key;
  Dbt data;

  //cerr << "... looking through count db" << endl;
  while( cursor->get( &key, &data, DB_NEXT ) != DB_NOTFOUND ) {
    string k = (char*)key.get_data();
    double c = 100.0*(double)atoi((char*)data.get_data()) / (double) total_commit_transactions;
    //cerr << "... found " << k << " with " << c << "%" << endl;
    if ( k.find("()") == string::npos ) {
      class_ranking[-c].insert(k);
    } else {
      function_ranking[-c].insert(k);
    }
  }
  cursor->close();
}

void count_single_items(const map< int, set<string> >& transactions,
                        map<string, int >& item_count )
{
  cerr << "... counting items" << endl;
  for( map< int, set<string> >::const_iterator t = transactions.begin();
       t != transactions.end(); ++t)
    {
      const set<string> & S = t->second;
      for( set<string>::const_iterator s = S.begin(); s != S.end(); ++s)
        {
	  item_count[*s]++;
        }
    }
}

void output_items( const map< double, set<string> >& class_ranking, unsigned int max_results )
{
  unsigned int count = 0;
  for( map< double, set<string> >::const_iterator i = class_ranking.begin();
       i != class_ranking.end(); ++i)
    {
      double score =  -(i->first);
      const set<string> & S = i->second;
      for(set<string>::const_iterator s = S.begin(); s != S.end(); ++s)
        {
	  cout << score << " ";

	  /*
	  // write out commits
	  // strip arrow off
	  string t;
	  for( string::const_iterator c = s->begin(); c != s->end(); c++ ) {
	  if ( (*c) != '=' && (*c) != '<' && (*c) != '>' ) {
	  t += (*c);
	  }
	  }
	  map<string, set<int> >::const_iterator it = item_commits.find(t);
	  if ( it != item_commits.end() ) {
	  set<int> C = it->second; //item_commits[t];
	  for(set<int>::const_iterator i = C.begin(); i != C.end(); i++ ) {
	  cout << (*i) << " ";
	  }
	  }
	  */

	  cout << (*s) << endl;
	  ++count;
	  if ( count == max_results )
            {
	      return;
            }
        }
    }
}

int main(unsigned int argc, char *argv[]) {

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
    while ( cin >> p) {
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

  unsigned int max_results = atoi( argv[npos] );


  try {

    unsigned int total_commit_transactions = 0;
    ifstream in( (cvsdata +"/root0/db/mining.count").c_str() );
    in >> total_commit_transactions;
    in.close();
    cerr << "TOTAL COMMIT TRANSACTIONS " << total_commit_transactions << endl;

    Db db(0,0);
#if DB_VERSION_MAJOR > 4 || (DB_VERSION_MAJOR == 4 && DB_VERSION_MINOR >= 1)
    db.open( NULL, (cvsdata +"/root0/db/mining.db").c_str(),  0 , DB_HASH, DB_RDONLY, 0 );
#else
    db.open( (cvsdata +"/root0/db/mining.db").c_str(),  0 , DB_HASH, DB_RDONLY, 0 );
#endif

    // ----------------------------------------
    // code which accesses Xapian
    // ----------------------------------------
    Xapian::Database database;

    for( set<string>::iterator i = packages.begin(); i != packages.end(); i++ ) {
      database.add_database(Xapian::Quartz::open(cvsdata+"/"+(*i)));
    }

    // start an enquire session
    Xapian::Enquire enquire(database);

    vector<string> queryterms;
    set<string> query_symbols;

    Xapian::Stem stemmer("english");

    set<string> ranking_system;

    for (unsigned int optpos = qpos; optpos < argc; optpos++) {

      string s = argv[optpos];

      if ( s.find(":") == 0 ) {
	queryterms.push_back(s); // symbol, put as is
	query_symbols.insert(s); // no stemming, no lc
        cerr << "QUERY TERM " << s << endl;
      } else if ( s == "=>" || s == "<=" || s == "<=>" ) {
	ranking_system.insert(s);
      } else {
	string term = s;
	lowercase_term(term);
	term = stemmer.stem_word(term);
	queryterms.push_back(term);
	cout << term << " ";
        cerr << "QUERY TERM " << term << endl;
      }
    }
    cout << endl; // empty line if no query words

    Xapian::MSet matches;

    if ( ! queryterms.empty() ) {

      Xapian::Query query(Xapian::Query::OP_AND, queryterms.begin(), queryterms.end());
      enquire.set_query(query);
      unsigned int num_results = 10000000;
      matches = enquire.get_mset(0, num_results);
      cerr <<  matches.size() << " results found" << endl;
    } else {
      cerr << "... simple mode" << endl;
      map< double, set<string> > function_ranking;
      map< double, set<string> > class_ranking;
      rank_all_items(db, total_commit_transactions, class_ranking, function_ranking);

      output_items( class_ranking, max_results );
      output_items( function_ranking, max_results );
      db.close(0);
      return 0;
    }

    map< int, set<string> > transactions_returned;
    for (Xapian::MSetIterator i = matches.begin(); i != matches.end(); i++) {
      unsigned int sim = matches.convert_to_percent(i);
      Xapian::Document doc = i.get_document();
      string data = doc.get_data();

      //cerr << "Found " << data << endl;

      list<string> symbols;
      split( data, " \n", symbols );
      set<string> S;

      // the commit number is also stored in data now, so we need to check for it below
      int commit_id = -1;
      for( list<string>::iterator s = symbols.begin(); s != symbols.end(); s++ ) {
	if ( isdigit((*s)[0]) ) {
	  if ( commit_id == -1 ) { // first number is commit
	    commit_id = atoi( s->c_str() );
	    continue;
	  } 
	}
	S.insert(*s);
	//  cerr << "..." << (*s) << endl;
      }
      assert( commit_id != -1 );
      transactions_returned[commit_id] = S;
    }

    assert( total_commit_transactions > 0 );

    if ( ! ranking_system.empty() ) {

      // the consequent of our rules contains only one item
      // so we need only count single items

      //////////////////////////////////// count single items

      map< string, int > relative_item_count; // the count is relative to the transactions returned
      count_single_items( transactions_returned, relative_item_count );

      // at this point we can generate rules
      map< double, set<string> > function_ranking;
      map< double, set<string> > class_ranking;

      generate_rules( db, ranking_system, query_symbols, relative_item_count,
		      transactions_returned.size(),
		      total_commit_transactions, class_ranking, function_ranking );

      output_items( class_ranking, max_results );
      output_items( function_ranking, max_results );

    } else {

      int c = 0;
      // no ranking system specified, so return list of commit ids
      for( map< int, set<string> >::iterator i = transactions_returned.begin(); i != transactions_returned.end(); i++ ) {
	cout << i->first << endl;
	c++;
	if ( c == max_results ) {
	  break;
	}
      }

    }

    db.close(0);


  }
  catch(const Xapian::Error & error) {
    cerr << "Exception: " << error.get_msg() << endl;
  }  catch( DbException& e ) {
    cerr << "Exception:  " << e.what() << endl;
  }
}
