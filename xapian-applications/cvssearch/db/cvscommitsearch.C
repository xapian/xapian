// cvsminesearch.C
//
// (c) 2001 Amir Michail (amir@users.sourceforge.net)

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

#include <math.h>
#include <algorithm>


/**

koffice/kchart 0
koffice/kformula 198
koffice/killustrator 316
koffice/kimageshop 778
koffice/kivio 972
koffice/koshell 1077
koffice/kpresenter 1132
koffice/kspread 1894
koffice/kword 2999
koffice/lib 4330


**/

/* test kword searches right now, get these numbers from offset file */
#define FIRST_COMMIT 2999
#define LAST_COMMIT 4329




#define MIN_SUPPORT 1 
#define MAX_QUERY_VECTOR_TERMS 50


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
//   cvsminesearch root0/db/commit.om 10
//   cvsminesearch root0/db/commit.om 10 drag drop =>
//   cvsminesearch root0/db/commit.om 10 drag drop (without arrow)
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








#include <db_cxx.h>
#include <om/om.h>
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

  /**
  cerr << "b_count " << b_count << endl;
  cerr << "not_b_count " << not_b_count << endl;
  cerr << "total_commit_transactions " << total_commit_transactions << endl;
  cerr << "... a_prob " << a_prob << endl;
  cerr << "... not_b_prob " << not_b_prob << endl;
  cerr << "... a_prob*not_b_prob " << a_prob*not_b_prob << endl;
  cerr << "... a_and_not_b_prob " << a_and_not_b_prob << endl;
  cerr << "... convinction " << convinction << endl;
  **/

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
                     map<double, set<string> >& code_term_ranking )
{
  assert( ! ranking_system.empty() );

  for( map<string, int >::const_iterator i = relative_item_count.begin();
       i != relative_item_count.end(); ++i)
    {
      string symbol = i->first;
      //cerr << "Considering " << symbol << endl;
      unsigned int a_and_b_count = i->second;

      if ( a_and_b_count < MIN_SUPPORT ) {
	continue;
      }

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
	  assert(0);
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
	  assert(0);
	  score = score * convinction;
	  prefix += "*<=";
	}

      }

      string item = /*prefix +*/ symbol;

      code_term_ranking[ -score ].insert(item);

    }
}



#warning "don't double count here"
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

void output_items( const map< double, set<string> >& code_term_ranking, unsigned int max_results )
{
  unsigned int count = 0;
  for( map< double, set<string> >::const_iterator i = code_term_ranking.begin();
       i != code_term_ranking.end(); ++i)
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
    ifstream in( (cvsdata +"/root0/db/commit.count").c_str() );
    in >> total_commit_transactions;
    in.close();
    cerr << "TOTAL COMMIT TRANSACTIONS " << total_commit_transactions << endl;

    Db db(0,0);
    db.open( (cvsdata +"/root0/db/commit.db").c_str(),  0 , DB_HASH, DB_RDONLY, 0 );



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
    set<string> query_symbols;

    OmStem stemmer("english");

    set<string> ranking_system;

    for (unsigned int optpos = qpos; optpos < argc; optpos++) {

      string s = argv[optpos];

      if ( s.find(":") == 0 ) {
	queryterms.push_back(s); // symbol, put as is
	query_symbols.insert(s); // no stemming, no lc
      } else if ( s == "=>" || s == "<=" || s == "<=>" ) {
	ranking_system.insert(s);
      } else {
	om_termname term = s;
	lowercase_term(term);
	term = stemmer.stem_word(term);
	queryterms.push_back(term);
	cout << term << " ";
      }
    }
    cout << endl; // empty line if no query words

    OmMSet matches;

    if ( ! queryterms.empty() ) {

      OmQuery query(OmQuery::OP_AND, queryterms.begin(), queryterms.end());
      enquire.set_query(query);
      unsigned int num_results = 10000000;
      matches = enquire.get_mset(0, num_results);
      cerr <<  matches.size() << " results found" << endl;
    } else {
      cerr << "... no query words specified" << endl;
      assert(0);
    }

    map< int, set<string> > transactions_returned;
    map< pair<int, string>, int > transaction_term_count;

    for (OmMSetIterator i = matches.begin(); i != matches.end(); i++) {
      unsigned int sim = matches.convert_to_percent(i);
      OmDocument doc = i.get_document();
      string data = doc.get_data().value;

      //cerr << "Found " << data << endl;

      list<string> symbols;
      split( data, " \n", symbols );
      set<string> S;

      // the commit number is also stored in data now, so we need to check for it below
      int commit_id = -1;
      for( list<string>::iterator s = symbols.begin(); s != symbols.end(); s++ ) {
	if ( isdigit((*s)[0]) ) {
	  if( commit_id != -1 ) {
	    cerr << "warning:  found " << (*s) << " in code symbol terms" << endl;
	    continue;
	  }
	  commit_id = atoi( s->c_str() );
	  continue;
	}
	assert( commit_id != -1 );
	transaction_term_count[ make_pair( commit_id, *s ) ] ++;
	S.insert(*s);
	//  cerr << "..." << (*s) << endl;
      }
      assert( commit_id != -1 );


      /////////////////////////////// here's the key part:  commit_id => transaction items (code terms)
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
      map< double, set<string> > code_term_ranking;

      generate_rules( db, ranking_system, query_symbols, relative_item_count,
		      transactions_returned.size(),
		      total_commit_transactions, code_term_ranking );

      output_items( code_term_ranking, max_results );

      // construct query vector

      map< string, double > query_vector;
      set< string > query_expansion;

      for( map< double, set<string> >::const_iterator i = code_term_ranking.begin(); i != code_term_ranking.end(); i++ ) {
	set<string> W = i->second;
	for( set<string>::const_iterator w = W.begin(); w != W.end(); w++ ) {
	  query_vector[ *w ] = pow(2.0,pow(2.0, -(i->first)));
	  cerr << "word " << *w << " has score " << query_vector[*w] << endl;
	  query_expansion.insert(*w);
	  if ( query_vector.size() == MAX_QUERY_VECTOR_TERMS ) {
	    goto done;
	  }
	}
      }
    done: ;

      /////////////// okay, now we go over every commit and compute the cosine similarity with the query vector

      // compute |V_q|

      double f = 0.0;
      for( map< string, double>::iterator i = query_vector.begin(); i != query_vector.end(); i++ ) {
	f += (i->second*i->second);
      }
      f = sqrt(f);

      map< double, set<int> > final_ranking;

      for( map< int, set<string> >::iterator t = transactions_returned.begin(); t != transactions_returned.end(); t++ ) {
	// we construct a binary vector for the document

	int commit_id = t->first;
	set<string> T = t->second;

	if ( T.empty() ) {
	  //	  cerr << "... skipping commit with no code words" << endl;
	  continue; // skip empty commits
	}


	if ( commit_id < FIRST_COMMIT || commit_id > LAST_COMMIT ) {
	  //	  cerr << "... skipping commit not in app range" << endl;
	  continue;
	}

	

	int total_commit_code_words = 0;
	double f2 = 0.0;
	for( set<string>::iterator j = T.begin(); j != T.end(); j++ ) {
	  int c = transaction_term_count[ make_pair(commit_id, *j) ];
	  f2 += (double)c*(double)c;
	  total_commit_code_words += c;
	}
	f2 = sqrt(f2);


	cerr << "Transaction " << (t->first-FIRST_COMMIT) << " has # code terms = " << total_commit_code_words << endl;
	cerr << "...query expansion size " << query_expansion.size() << endl;



	// intersect T with query_expansion

	set<string> result;
	set_intersection( T.begin(), T.end(), query_expansion.begin(), query_expansion.end(), inserter(result, result.begin()) );
	//	cerr << "... intersection size " << result.size() << endl;
	
	//	assert( result.size() == T.size() ); // if no limit # query expansion

	// compute dot product

	double dot = 0.0;
	for ( set<string>::iterator i = result.begin(); i != result.end(); i++ ) {
	  cerr << "... shared term " << *i << " has count " << transaction_term_count[make_pair(commit_id, *i)] << " and query vector weight = " << query_vector[*i] << endl;
	  dot += query_vector[*i]*transaction_term_count[make_pair(commit_id, *i)]; // multiply by binary doc vector
	}
	//	cerr << "..... dot = " << dot << endl;

	// divide by |V_d|, since V_d is binary, this is simply sqrt(T.size())

	double cosine = dot / (f*f2);

	cerr << ".... cosine similarity = " << cosine << endl;

#warning "score is a combination of similarity & size"
	double final_score = cosine * log((double)total_commit_code_words);

	final_ranking[-final_score].insert( t->first );
      }


      cerr << "query expansion size = " << query_expansion.size() << endl;

      for( map< double, set<int> >::iterator i = final_ranking.begin(); i != final_ranking.end(); i++ ) {
	double score = i->first;
	set<int> S = i->second;
	for ( set<int>::iterator j = S.begin(); j != S.end(); j++ ) {
	  cerr << "commit " << ((*j)-FIRST_COMMIT) << " of size " << transactions_returned[*j].size() << " has score " << -score << endl;
	}
      }






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
  catch(OmError & error) {
    cerr << "Exception: " << error.get_msg() << endl;
  }  catch( DbException& e ) {
    cerr << "Exception:  " << e.what() << endl;
  }
}
