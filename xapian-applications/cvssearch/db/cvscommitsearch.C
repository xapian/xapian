// cvsminesearch.C
//
// (c) 2001 Amir Michail (amir@users.sourceforge.net)

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
 

#include <math.h>
#include <algorithm>


// it is sufficient to consider search by commit
// if our system works properly, because we can be assured that every line of code
// is involved in at least one commit, so at least one commit will come up 
// even if the query words show up only in the code

// also, observe that we now index using both comments & code words
// the data mining will pick up new things now; unfortunately, things are also
// slower now

//
// should now have query words => query word with value infinity; need to fix this
// it actually doesn't show up as infinity but a large number; why? stemming?
// No.  This one is simple.  For code words, you will always find them
// in the code. But for comment words, they may or may not be in the code.

// however, even so, we should probably not count these convinction values but just
// use idf


/* test konqueror searches right now, get these numbers from offset file */
/*
#define FIRST_COMMIT 5550
#define LAST_COMMIT 7593
*/

/* test kword */
/*
#define FIRST_COMMIT 41590
#define LAST_COMMIT 42920
*/


// global
#define FIRST_COMMIT 5550
#define LAST_COMMIT 7593




#define MIN_SUPPORT 1 
#define MAX_QUERY_VECTOR_TERMS 25


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




// determine # transactions in which we find this code symbol
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

double compute_magnitude( const map< string, double >& v ) {
  double f = 0.0;
  for( map<string, double>::const_iterator i = v.begin(); i != v.end(); i++ ) {
    f += (i->second)*(i->second);
  }
  f = sqrt(f);
  return f;
}

double compute_cosine_similarity( const map< string, double >& v1,
				  const map< string, double >& v2 ) {

  double dot = 0.0;

  for( map<string, double>::const_iterator i1 = v1.begin(); i1 != v1.end(); i1++ ) {
    map<string, double>::const_iterator i2 = v2.find(i1->first);
    if ( i2 != v2.end() ) {
      cerr << "shared term " << (i1->first) << " with weights " << (i1->second) << " and " << (i2->second) << endl;
      dot += (i1->second)*(i2->second);
    }
  }

  double m1 = compute_magnitude(v1);
  double m2 = compute_magnitude(v2);

  return dot / (m1*m2);
  
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
    set<string> query_term_set;
    map< string, double > query_vector;

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
	query_term_set.insert(term);
	query_vector[ term ] = 1.0;
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

    //    map< int, set<string> > transaction_all_words;
    map< int, set<string> > transaction_code_words;
    map< pair<int, string>, int > transaction_code_word_count;
    map< int, set<string> > transaction_comment_words;
    map< pair<int, string>, int > transaction_comment_word_count;

    cerr << "analyzing results from omseek" << endl;
    for (OmMSetIterator i = matches.begin(); i != matches.end(); i++) {
      unsigned int sim = matches.convert_to_percent(i);
      OmDocument doc = i.get_document();
      string data = doc.get_data().value;

      //cerr << "Found " << data << endl;

      list<string> symbols;
      split( data, " \n", symbols );
      //      set<string> S_code, S_comment;

      // the commit number is also stored in data now, so we need to check for it below
      int commit_id = -1;
      for( list<string>::iterator s = symbols.begin(); s != symbols.end(); s++ ) {
	assert( s->length() >= 1 );
	if ( isdigit((*s)[0]) ) {
	  if( commit_id != -1 ) {
	    //	    cerr << "warning:  found " << (*s) << " in code symbol terms" << endl;
	    continue;
	  }
	  
	  commit_id = atoi( s->c_str() );
	  //	  cerr << "** commit " << commit_id << endl;
	  continue;
	}
	assert( commit_id != -1 );
	if ( (*s)[0] == '+' ) {
	  string s2( *s, 1, s->length()-1 );
	  if ( isdigit(s2[0]) ) {
	    //	    cerr << "warning:  found " << s2 << " in code comment terms" << endl;
	    continue;
	  }

	  if ( s2.length() == 1 ) {
	    continue;
	  }

	  //	  cerr << "converted -" << (*s) << "- to " << "-" << s2 <<"-" << endl;
	  transaction_comment_word_count[ make_pair( commit_id, s2 ) ] ++;
	  transaction_comment_words[commit_id].insert(s2);
	  //	  cerr << "... comment word " << s2 << endl;
	} else {

	  if ( s->length() == 1 ) {
	    continue;
	  }

	  transaction_code_word_count[ make_pair( commit_id, *s ) ] ++;
	  transaction_code_words[commit_id].insert(*s);
	  //	  cerr << "... code word " << (*s) << endl;
	}
      }
      assert( commit_id != -1 );
    }

    cerr << "done" << endl;

    assert( total_commit_transactions > 0 );

    if ( ! ranking_system.empty() ) {

      // the consequent of our rules contains only one item
      // so we need only count single items

      //////////////////////////////////// count single items

      map< string, int > relative_item_count; // the count is relative to the transactions returned


      cerr << "counting single items" << endl;
      count_single_items( transaction_code_words, relative_item_count );

      // at this point we can generate rules
      map< double, set<string> > code_term_ranking;

      cerr << "generating rules" << endl;
      generate_rules( db, ranking_system, query_symbols, relative_item_count,
		      transaction_code_words.size(),
		      total_commit_transactions, code_term_ranking );

      cerr << "outputing items" << endl;
      output_items( code_term_ranking, max_results );








      // construct query vector

      map< string, double > expanded_query_vector;

      set< string > query_expansion;

      //      int rank = 0;

      cerr << "creating expanded vector" << endl;
      for( map< double, set<string> >::const_iterator i = code_term_ranking.begin(); i != code_term_ranking.end(); i++ ) {
	set<string> W = i->second;
	for( set<string>::const_iterator w = W.begin(); w != W.end(); w++ ) {

	  double idf = compute_idf( db, *w, total_commit_transactions ); 
	  //	  rank++;
	  expanded_query_vector[ *w ] = idf; //*(-(i->first)); //pow(2.0,pow(2.0, -(i->first)));

	  if ( query_term_set.find(*w) != query_term_set.end() ) {
	    // this has infinite convinction
	    expanded_query_vector[*w] *= 5.0; // 10 times as much for query words
	  }

	  cerr << "word " << *w << " has score " << expanded_query_vector[*w] << endl;
	  query_expansion.insert(*w);
	  if ( expanded_query_vector.size() == MAX_QUERY_VECTOR_TERMS ) {
	    goto done;
	  }
	}
      }
    done: ;

      /////////////// okay, now we go over every commit and compute the cosine similarity with the query vector



      // we compute two cosine similarities

      // cosine similarity 1:  query words with comment vector

      // cosine similarity 2:  query expansion words with code vector


      map< double, set<int> > final_ranking;

      for( map< int, set<string> >::iterator t = transaction_code_words.begin(); t != transaction_code_words.end(); t++ ) {
	// we construct a binary vector for the document

	int commit_id = t->first;
	set<string> T = t->second;

	if ( T.empty() ) {
	  //	  cerr << "... skipping commit with no code words" << endl;
	  continue; // skip empty commits
	}


	if ( commit_id < FIRST_COMMIT || commit_id > LAST_COMMIT ) {
	  //	  cerr << "... skipping commit " << commit_id << " not in app range" << endl;
	  continue;
	}

	
	// build code vector

	map< string, double > code_vector;
	cerr << ".. building code vector for commit " << commit_id << endl;

	int total_commit_code_words = 0;
	for( set<string>::iterator j = T.begin(); j != T.end(); j++ ) {
	  int c = transaction_code_word_count[ make_pair(commit_id, *j) ];
	  code_vector[ *j ] = (double)c*compute_idf( db, *j, total_commit_transactions );;
	  total_commit_code_words += c;
	}

	// build comment vector
	cerr << ".. building comment vector for commit " << commit_id << endl;
	map< string, double > comment_vector;
	set<string> C = transaction_comment_words[commit_id];
	for( set<string>::iterator c = C.begin(); c != C.end(); c++ ) {
	  comment_vector[ *c ] = (double)transaction_comment_word_count[ make_pair(commit_id, *c) ] * compute_idf( db, *c, total_commit_transactions );
	}


	cerr << "Transaction " << (t->first-FIRST_COMMIT) << " has # code terms = " << total_commit_code_words << endl;
	cerr << "...query expansion size " << query_expansion.size() << endl;

	// cosine similarity 1:  query words with comment vector
	double cosine1 = compute_cosine_similarity( query_vector, comment_vector );	

	cerr << "... comment cosine similarity = " << cosine1 << endl;

	// cosine similarity 2:  query expansion words with code vector

	double cosine2 = compute_cosine_similarity( expanded_query_vector, code_vector );


	cerr << ".... code cosine similarity = " << cosine2 << endl;

	// a low value (e.g., 0.01) makes each factor really important
	double final_score = (0.01+cosine1)*(0.01+cosine2)*log((double)total_commit_code_words);


	/***
	cerr << "... multiplying by " << log((double)total_commit_code_words) << endl;
	final_score = final_score * log((double)total_commit_code_words);

	cerr << "... dividing by " <<  log(1.0+transaction_comment_word_count[ commit_id ]) << endl;
	final_score = final_score / log(1.0+transaction_comment_word_count[ commit_id ]);
	***/


	final_ranking[-final_score].insert( t->first );
      }


      cerr << "query expansion size = " << query_expansion.size() << endl;

      for( map< double, set<int> >::iterator i = final_ranking.begin(); i != final_ranking.end(); i++ ) {
	double score = i->first;
	set<int> S = i->second;
	for ( set<int>::iterator j = S.begin(); j != S.end(); j++ ) {
	  cerr << "commit " << ((*j)-FIRST_COMMIT) << " of size " << transaction_code_words[*j].size() << " has score " << -score << endl;
	}
      }






    } else {

      int c = 0;
      // no ranking system specified, so return list of commit ids
      for( map< int, set<string> >::iterator i = transaction_code_words.begin(); i != transaction_code_words.end(); i++ ) {
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
