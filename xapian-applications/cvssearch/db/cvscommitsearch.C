#warning "DOES NOT HANDLE STOP WORDS IN QUERY"
// cvscommitsearch.C
//
// (c) 2001 Amir Michail (amir@users.sourceforge.net)

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
 

#include <math.h>
#include <algorithm>

#warning "*** IN: DOESN'T WORK WELL DUE TO LIMITING # OF SEARCH RESULTS"

// should probably put a limit on # of terms we look at in a commit when
// doing query expansion; currently it may take too long for some queries

//
// Major bug:
//
// ./cvscommitsearch root0/db/commit.om 10 lyx
//
// yields:
//
//  commit 0 in package kmusic/brahms of code size 5534 has score 1.7303  
// 
// which appears to be a bit entry

#define OFFSET_FILE "/root0/commit.offset"


// IT SEEMS LIKE WE ACTUALLY GET MUCH BETTER RESULTS
// BY LOOKING AT FEWER TRANSACTIONS RETURNED FIRST BY Xapian;
// EXAMPLE:  try sound; you get much better results with the top 100
// than the top 500.

// only look at top 100 commits
#define GLOBAL_QUERY_MAX_TRANSACTIONS 100

// to save time, we put a limit on # of transactions outside app
// however, we look at all app transactions
#define LOCAL_QUERY_MAX_OTHER_TRANSACTIONS 100

// global
//#define FIRST_COMMIT 5550
//#define LAST_COMMIT 7593

///////// more on data mining

// we should do data mining at two levels

// at the global level

// and also at the application level


// it is sufficient to consider search by commit
// if our system works properly, because we can be assured that every line of
// code is involved in at least one commit, so at least one commit will come up
// even if the query words show up only in the code

// also, observe that we now index using both comments & code words
// the data mining will pick up new things now; unfortunately, things are also
// slower now

// should now have query words => query word with value infinity; need to fix
// this it actually doesn't show up as infinity but a large number; why?
// stemming?
// No.  This one is simple.  For code words, you will always find them
// in the code. But for comment words, they may or may not be in the code.

// however, even so, we should probably not count these conviction values but
// just use idf


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


#define MIN_SUPPORT 1 
#define MAX_QUERY_VECTOR_TERMS 25


// try conviction instead of interest measure (conviction is directional)
//
// See:
//
// http://citeseer.nj.nec.com/brin97dynamic.html
//

// confidence: P(A&B)/P(A)

// interest: P(A&B)/(P(A)*P(B) [completely symmetric]

// conviction: P(A)P(not B) / P(A, not B)
//
// Intuition:  logically, A=>B can be rewritten as ~(A & ~B), so we can see
//                 how far A&~B deviates from independence, and invert the ratio
//                 to take care of the outside negation.
//
//                 Unlike confidence, conviction factors in both P(A) and P(B) and
//                 always has a value of 1 when the relvant items are completely unrelated
//
//                 Unlike interest, rules which hold 100% of the time have the highest possible
//                 conviction value of infinity.  (While confidence has this property, interest does
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


// Usage:  cvsminesearch package (# results) query_word1 query_word2 ...
//
//               cvsminesearch (# results) query_word1 query_word2 ... takes list of packages from stdin
//
// Example:  cvsminesearch root0/db/kdeutils_kfind 10 ftp nfs
//
//     Returns the top 10 lines with both ftp and nfs.
//
// ($CVSDATA/package is the directory with the quartz database inside)


// Examples:
//
//   cvsminesearch root0/db/commit.om 10
//   cvsminesearch root0/db/commit.om 10 drag drop =>
//   cvsminesearch root0/db/commit.om 10 drag drop (without arrow)
//      just returns commits with drag drop in comments


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
  //cerr << "idf = " << idf << endl;
  return idf;
}

double compute_conviction( unsigned int a_count, 
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

  double conviction = (a_prob * not_b_prob) / ( a_and_not_b_prob );

  /**
     cerr << "b_count " << b_count << endl;
     cerr << "not_b_count " << not_b_count << endl;
     cerr << "total_commit_transactions " << total_commit_transactions << endl;
     cerr << "... a_prob " << a_prob << endl;
     cerr << "... not_b_prob " << not_b_prob << endl;
     cerr << "... a_prob*not_b_prob " << a_prob*not_b_prob << endl;
     cerr << "... a_and_not_b_prob " << a_and_not_b_prob << endl;
     cerr << "... conviction " << conviction << endl;
  **/

  //#warning "dividing conviction by prob of consequent"
  //cerr << "..conviction " << conviction << endl;
  //cerr << "..prob b " << b_prob << endl;


  //conviction = conviction / log(1.0+log(1.0+b_prob));

  return conviction;
}


void generate_rules( map<string, int>& item_count,
                     const set<string>& query_symbols,
                     const map<string, int >& relative_item_count,
                     unsigned int transactions_returned,
                     unsigned int total_commit_transactions,
                     map<double, set<string> >& code_term_ranking )
{

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

      string prefix = "";

      // generate rule of the form a=>b
      unsigned int a_count = 0;
      unsigned int b_count = 0;
      
      unsigned int symbol_count = item_count[symbol]; //find_symbol_count( db, symbol );
      
      // normally, we would be here...
      a_count = transactions_returned; // if only query terms specified, this case applies also
      b_count = symbol_count;
      
      double conviction = compute_conviction( a_count, b_count, a_and_b_count, total_commit_transactions );
      
      code_term_ranking[ -conviction ].insert(symbol);

    }
}




void count_single_items(const map< int, set<string> >& transactions,
                        map<string, int >& item_count )
{
//  cerr << "... counting items" << endl;
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


  if ( v1.size() == 0 || v2.size() == 0 ) {
    return 0.0;
  }

  double dot = 0.0;

  for( map<string, double>::const_iterator i1 = v1.begin(); i1 != v1.end(); i1++ ) {
    map<string, double>::const_iterator i2 = v2.find(i1->first);
    if ( i2 != v2.end() ) {
      //      cerr << "shared term " << (i1->first) << " with weights " << (i1->second) << " and " << (i2->second) << endl;
      dot += (i1->second)*(i2->second);
    }
  }

  double m1 = compute_magnitude(v1);
  double m2 = compute_magnitude(v2);

  return dot / (m1*m2);
  
}


bool commit_of_interest( int commit_id, list<string>& in_opt_list,
			map< int, string >& commit_package,
			map< string, int>& package_last_commit ) {
  if ( in_opt_list.empty() ) {
    return true;
  }

  // check if in_opt is part of substring

  for( list<string>::iterator in_opt = in_opt_list.begin(); in_opt != in_opt_list.end(); in_opt++ ) {

    //    cerr << "checking if -" <<*in_opt<<"- is in -" << commit_package[commit_id] << "-" << endl;

    
    if ( commit_package[ commit_id ].find( *in_opt ) != -1 ) {
      return true;
    }
    

    /**********
    if ( commit_package[ commit_id ] != "" ) {
      return false;
    }
    
    if ( package_last_commit[ *in_opt ] == 0 ) { // hack
      // must mean this is the last package
      return true;
    }
    *********/

  }

  return false;
}

int main(unsigned int argc, char *argv[]) {

  if (argc < 3) {
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
  if (isdigit(argv[1][0])) {
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

  unsigned int max_results = atoi(argv[npos]);

  try {
    unsigned int total_commit_transactions = 0;
    ifstream in( (cvsdata +"/root0/db/commit.count").c_str() );
    in >> total_commit_transactions;
    in.close();
    cerr << "TOTAL COMMIT TRANSACTIONS " << total_commit_transactions << endl;



    // load commit offset info
    map< string, int > package_first_commit;
    map< string, int > package_last_commit;
    map< int, string > commit_package;

    ifstream in2 ((cvsdata+OFFSET_FILE).c_str());
    string line;
    string last_package = "";
    int last_offset = -1;
    if (in2) {
	while (true) {
	  string package;
	  int offset;
	  in2 >> package; 
	  if ( in2.eof() ) {
	    break;
	  }
	  in2 >> offset;
//	  cerr << "read -" << package <<"- at offset " << offset << endl;
	  package_first_commit[package] = offset;
	  if (!last_package.empty()) {
	    package_last_commit[last_package] = offset-1;
	    for( int i = last_offset; i <= offset-1; i++ ) {
	      commit_package[i] = last_package;
	    }
	  }

	  last_package = package;
	  last_offset = offset;
	}
	in2.close();
    }
    
    Db db_cmt(0,0);
#if DB_VERSION_MAJOR > 4 || (DB_VERSION_MAJOR == 4 && DB_VERSION_MINOR >= 1)
    db_cmt.open( NULL, (cvsdata +"/root0/db/commit_cmt.db").c_str(),  0 , DB_HASH, DB_RDONLY, 0 );
#else
    db_cmt.open( (cvsdata +"/root0/db/commit_cmt.db").c_str(),  0 , DB_HASH, DB_RDONLY, 0 );
#endif

    Db db_code(0,0);
#if DB_VERSION_MAJOR > 4 || (DB_VERSION_MAJOR == 4 && DB_VERSION_MINOR >= 1)
    db_code.open( NULL, (cvsdata +"/root0/db/commit_code.db").c_str(),  0 , DB_HASH, DB_RDONLY, 0 );
#else
    db_code.open( (cvsdata +"/root0/db/commit_code.db").c_str(),  0 , DB_HASH, DB_RDONLY, 0 );
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
    set<string> query_term_set;
    map< string, double > query_vector;

    string in_opt = "";
    list<string> in_opt_list;

    Xapian::Stem stemmer("english");

    for (unsigned int optpos = qpos; optpos < argc; optpos++) {

      string s = argv[optpos];

      if ( s.find(":") == 0 ) {
	queryterms.push_back(s); // symbol, put as is
	query_symbols.insert(s); // no stemming, no lc
      } else if ( s.find("in:") == 0 ) {
	in_opt = s.substr(3);
	cerr << "RESTRICTED TO -" << in_opt << "-" << endl;
	split(in_opt, ";", in_opt_list);
      } else if ( s == "=>" || s == "<=" || s == "<=>" ) {
	cerr << "\nranking system no longer required" << endl;
	assert(0);
      } else {
	string term = s;
	lowercase_term(term);
	term = stemmer.stem_word(term);
	queryterms.push_back(term);
	query_term_set.insert(term);
	query_vector[ term ] = 1.0;
	cout << term << " ";
        cerr << "QUERY TERM " << term << endl;
      }
    }
    cout << endl; // empty line if no query words

    assert( query_vector.size() >= 1 );

    Xapian::MSet matches;

    if ( ! queryterms.empty() ) {
      Xapian::Query query(Xapian::Query::OP_AND, queryterms.begin(), queryterms.end());
      enquire.set_query(query);
      unsigned int num_results = 10000000;
      matches = enquire.get_mset(0, num_results);
      cerr <<  matches.size() << " results found" << endl;
    } else {
      cerr << "... no query words specified" << endl;
      assert(0);
    }

    map< int, set<string> > transaction_code_words;
    map< pair<int, string>, int > transaction_code_word_count;
    map< int, set<string> > transaction_comment_words;
    map< pair<int, string>, int > transaction_comment_word_count;

    map< string, int > item_count; // required for mining subset of all transactions

    cerr << "analyzing results from Xapian" << endl;
    int other_transactions_read = 0;
    int total_transactions_read = 0;

    int last_percentage = 100;

    for (Xapian::MSetIterator i = matches.begin(); i != matches.end(); i++) {

      if ( in_opt == "" && total_transactions_read == GLOBAL_QUERY_MAX_TRANSACTIONS ) {
	break;
      }

      unsigned int sim = matches.convert_to_percent(i);
      assert( sim <= last_percentage );
      last_percentage = sim;
      //      cerr << "sim = " << sim << endl;

      Xapian::Document doc = i.get_document();
      string data = doc.get_data();

      total_transactions_read++; // used for global queries

      //      cerr << "Found " << data << endl;

      list<string> symbols;
      split( data, " \n", symbols ); // 20% of time spent here

      //#warning "IGNORING DATA FOR SPEED TEST"
      //#if 0

      // code below takes 80% of time

      // the commit number is also stored in data now, so we need to check for it below
      int commit_id = -1;


      for( list<string>::iterator s = symbols.begin(); s != symbols.end(); s++ ) {
	//	assert( s->length() >= 1 );
	if ( isdigit((*s)[0]) ) {
	  if( commit_id != -1 ) {
	    //	    cerr << "warning:  found " << (*s) << " in code symbol terms" << endl;
	    continue;
	  }

	  commit_id = atoi( s->c_str() );

	  //      cerr << "** commit " << commit_id << endl;
	  
	  if ( !commit_of_interest( commit_id, in_opt_list, commit_package, package_last_commit ) ) {
	    other_transactions_read++;
	    if ( other_transactions_read > LOCAL_QUERY_MAX_OTHER_TRANSACTIONS ) {
	      //cerr << "skipping other transaction " << commit_id << endl;
	      goto skip;
	    } else {
	      //	      cerr << "ACCEPTED OTHER TRANSACTION " << other_transactions_read << endl;
	    }
	  }


	  set<string> empty;
	  transaction_comment_words[commit_id] = empty;
	  transaction_code_words[commit_id] = empty;

	  continue;
	}

	assert( commit_id != -1 );


	if ( (*s)[0] == '+' ) {
	  const string& s2 = s->substr(1);
	  if ( isdigit(s2[0]) ) {
	    //	    cerr << "warning:  found " << (*s) << " in code comment terms" << endl;
	    continue;
	  }

	  //	  if ( s2.length() == 1 ) {
	  //	    continue;
	  //	  }

	  transaction_comment_word_count[ make_pair( commit_id, s2 ) ] ++;
	  transaction_comment_words[commit_id].insert( s2 );
	  //	  cerr << "... comment word -" << s2 << "-" << endl;
	} else {

	  //	  if ( s->length() == 1 ) {
	  //	    continue;
	  //	  }

	  transaction_code_word_count[ make_pair( commit_id, *s ) ] ++;
	  transaction_code_words[commit_id].insert(*s);
	  //	  cerr << "... code word -" << (*s) << "-" << endl;
	}
      }

      // update item count
      for( set<string>::iterator i = transaction_code_words[commit_id].begin();
	   i != transaction_code_words[commit_id].end(); i++ ) {
	item_count[*i] ++;
      }


    skip: ;
      assert( commit_id != -1 );
      //#endif
    }

    cerr << "done" << endl;

    assert( total_commit_transactions > 0 );

    cerr << "# code transactions = " << transaction_code_words.size() << endl;
    cerr << "# comment transactions = " << transaction_comment_words.size() << endl;

#warning "data mining done with respect to code"      



    // the consequent of our rules contains only one item
    // so we need only count single items

    //////////////////////////////////// count single items

    map< string, int > relative_item_count; // the count is relative to the transactions returned


    cerr << "counting single items" << endl;
    count_single_items( transaction_code_words, relative_item_count );

    // at this point we can generate rules
    map< double, set<string> > code_term_ranking;

    cerr << "generating rules" << endl;
    generate_rules( item_count, query_symbols, relative_item_count,
		    transaction_code_words.size(),
		    total_commit_transactions, code_term_ranking );

    //    cerr << "outputing items" << endl;
    //    output_items( code_term_ranking, max_results );








    // construct query vector

    map< string, double > expanded_query_vector;

    set< string > query_expansion;

    //      int rank = 0;

    cerr << "creating expanded vector" << endl;
    for( map< double, set<string> >::const_iterator i = code_term_ranking.begin(); i != code_term_ranking.end(); i++ ) {
      set<string> W = i->second;
      for( set<string>::const_iterator w = W.begin(); w != W.end(); w++ ) {

#warning "idf here computed using code counts only"
	double idf = compute_idf( db_code, *w, total_commit_transactions ); 


	//	  rank++;
	expanded_query_vector[ *w ] = idf; //*(-(i->first)); //pow(2.0,pow(2.0, -(i->first)));

	if ( query_term_set.find(*w) != query_term_set.end() ) {
	  // this has infinite conviction
#warning "2.0 factor for query words"
	  expanded_query_vector[*w] *= 2.0; 
	}

	cerr << "word " << *w << " has score " << expanded_query_vector[*w] << endl;
	query_expansion.insert(*w);
	if ( expanded_query_vector.size() == MAX_QUERY_VECTOR_TERMS ) {
	  goto done;
	}
      }
    }
  done: ;

    cerr << endl;

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


      if ( ! commit_of_interest( commit_id, in_opt_list, commit_package, package_last_commit ) ) {
	//	  cerr << "... skipping commit " << commit_id << " not in app range" << endl;
	continue;
      }

	
      // build code vector

      map< string, double > code_vector;
      //	cerr << ".. building code vector for commit " << commit_id << endl;

      int total_commit_code_words = 0;
      for( set<string>::iterator j = T.begin(); j != T.end(); j++ ) {
	int c = transaction_code_word_count[ make_pair(commit_id, *j) ];
#warning "idf here computed using code counts"
	code_vector[ *j ] = (double)c*compute_idf( db_code, *j, total_commit_transactions );;
	total_commit_code_words += c;
      }

      // build comment vector
      //	cerr << ".. building comment vector for commit " << commit_id << endl;
      map< string, double > comment_vector;
      set<string> C = transaction_comment_words[commit_id];
      for( set<string>::iterator c = C.begin(); c != C.end(); c++ ) {
#warning "idf for comment uses comment counts"
	comment_vector[ *c ] = (double)transaction_comment_word_count[ make_pair(commit_id, *c) ] * compute_idf( db_cmt, *c, total_commit_transactions );
      }


      //	cerr << "Transaction " << (t->first-FIRST_COMMIT) << " has # code terms = " << total_commit_code_words << endl;
      //	cerr << "...query expansion size " << query_expansion.size() << endl;

      // cosine similarity 1:  query words with comment vector

      double cosine1 = compute_cosine_similarity( query_vector, comment_vector );	

      //      cerr << "... comment cosine similarity = " << cosine1 << endl;

      // cosine similarity 2:  query expansion words with code vector

      double cosine2 = compute_cosine_similarity( expanded_query_vector, code_vector );


      //      cerr << ".... code cosine similarity = " << cosine2 << endl;

      // a low value (e.g., 0.01) makes each factor really important
      double final_score = (0.01+cosine1)*(0.01+cosine2)*log((double)total_commit_code_words);
      //      cerr << "cosine1 = " << cosine1 << endl;
      //      cerr << "cosine2 = " << cosine2 << endl;
      //      cerr << total_commit_code_words << " => " << final_score << endl;


      /***
	  cerr << "... multiplying by " << log((double)total_commit_code_words) << endl;
	  final_score = final_score * log((double)total_commit_code_words);

	  cerr << "... dividing by " <<  log(1.0+transaction_comment_word_count[ commit_id ]) << endl;
	  final_score = final_score / log(1.0+transaction_comment_word_count[ commit_id ]);
      ***/


      final_ranking[-final_score].insert( t->first );
    }


    //    cerr << "query expansion size = " << query_expansion.size() << endl;

    int count = 0;
    for( map< double, set<int> >::iterator i = final_ranking.begin(); i != final_ranking.end(); i++ ) {
      double score = i->first;
      set<int> S = i->second;
      for ( set<int>::iterator j = S.begin(); j != S.end(); j++ ) {
	cerr << "commit " << ((*j)-package_first_commit[commit_package[*j]]) << " in package " << commit_package[*j] << " of code size " << transaction_code_words[*j].size() << " has score " << -score << endl;

	cout << (*j) << endl;

	/**
	// show comment profile also
	cerr << "...-";
	for( set<string>::iterator k = transaction_comment_words[*j].begin();
	     k != transaction_comment_words[*j].end(); k++ ) {
	  cerr << (*k) << "@" << transaction_comment_word_count[ make_pair(*j, *k) ] << " ";
	}
	cerr << "-" << endl;
	**/

	count++;
	if ( count == max_results ) {
	  goto completed;
	}
      }
    }
  completed: ;
    cerr << endl << endl;

    db_code.close(0);
    db_cmt.close(0);


  }
  catch(const Xapian::Error & error) {
    cerr << "Exception: " << error.get_msg() << endl;
  }  catch( DbException& e ) {
    cerr << "Exception:  " << e.what() << endl;
  }
}
