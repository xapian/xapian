#warning "DOES NOT HANDLE STOP WORDS IN QUERY"
// cvsminesearch.C
//
// (c) 2001 Amir Michail (amir@users.sourceforge.net)

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
 

#include <math.h>
#include <algorithm>

#warning "IN: DOESN'T WORK WELL DUE TO LIMIT ON # OF SEARCH RESULTS"






// should probably put a limit on # of terms we look at in a commit
// when doing query expansion; now it may too long for some queries

//
// Major bug:
//
//./cvscommitsearch root0/db/commit.om 10 lyx  
//
// yields:
//
//  commit 0 in package kmusic/brahms of code size 5534 has score 1.7303  
// 
// which appears to be a bit entry

#define OFFSET_FILE "/root0/related.offset"



///////// more on data mining

// we should do data mining at two levels

// at the global level

// and also at the application level





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





//
// Usage:  cvsrelatedsearch package (# results) query_word1 query_word2 ...
//
// also:  cvsrelatedsearch package (# results) package:commit_id
//

#include <xapian.h>
#include <db_cxx.h>
#include <stdio.h>
#include <math.h>

#include "util.h"

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


    // load commit offset info
    map< string, int > package_first_commit;
    map< string, int > package_last_commit;
    map< int, string > commit_package;

    ifstream in2 (  (cvsdata+OFFSET_FILE).c_str() );
    string line;
    string last_package = "";
    int last_offset = -1;
    for(;;) {
      string package; int offset;
      in2 >> package; 
      if ( in2.eof() ) {
	break;
      }
      in2 >> offset;
      //      cerr << "read -" << package <<"- at offset " << offset << endl;     

      package_first_commit[package] = offset;
      if ( last_package != "" ) {
	package_last_commit[last_package] = offset-1;
	for( int i = last_offset; i <= offset-1; i++ ) {
	  commit_package[i] = last_package;
	}
      }

      last_package = package;
      last_offset = offset;
    }
    in2.close();
    






    // ----------------------------------------
    // code which accesses Xapian
    // ----------------------------------------
    Xapian::Database database;

    // can search multiple databases at once
    for( set<string>::iterator i = packages.begin(); i != packages.end(); i++ ) {
      database.add_database(Xapian::Quartz::open(cvsdata+"/"+(*i)));
    }

    // start an enquire session
    Xapian::Enquire enquire(database);

    vector<string> queryterms;

    string in_opt = "";
    list<string> in_opt_list;

    Xapian::Stem stemmer("english");

    string query_package = "";
    string query_commit = "";

    for (unsigned int optpos = qpos; optpos < argc; optpos++) {

      string s = argv[optpos];

      if ( s.find("@") != -1 ) {

	// we have:  package_name:commit_id
	string package = s.substr( 0, s.find("@"));
	cerr << "PACKAGE -" << package << "-" << endl;

	string query_commit = s.substr( s.find("@")+1, s.length()-s.find("@"));
	cerr << "QUERY COMMIT -" << query_commit << "-" << endl;
	

	
	string cmd = "./cvsquerydb root0 " + package + " -C " + query_commit + " > ./cache/cmt";
	system("rm -f ./cache/cmt");
	cerr << "CMD = -" << cmd << "-" << endl;
	system(cmd.c_str());

	ifstream in("./cache/cmt");
	while ( ! in.eof() ) {
	  string comment;
	  in >> comment;

	  list<string> terms;
	  split( comment, " .,:;#%_*+&'\"/!()[]{}<>?-\t\n\002\003", terms );

	  for( list<string>::iterator i = terms.begin(); i != terms.end(); i++ ) {
	    string term = *i;
	    lowercase_term(term);
	    term = stemmer.stem_word(term);
	    
	    if ( term != "" && isalpha(term[0]) ) {
	      queryterms.push_back(term);
	      cout << term << " ";
	      cerr << "QUERY TERM " << term << endl;
	    }

	  }
	  
	}
	in.close();

	
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
	if ( term != "" ) {
	  queryterms.push_back(term);
	  cout << term << " ";
	  cerr << "QUERY TERM " << term << endl;
	}
      }
    }
    cout << endl; // empty line if no query words


    Xapian::MSet matches;

    if ( ! queryterms.empty() ) {

      Xapian::Query query(Xapian::Query::OP_OR, queryterms.begin(), queryterms.end());
      enquire.set_query(query);

      if ( in_opt == "" ) {
	      matches = enquire.get_mset(0, max_results);
      } else { // in: used
	      matches = enquire.get_mset(0, 10000000);
      }
      cerr <<  matches.size() << " results found" << endl;
    } else {
      cerr << "... no query words specified" << endl;
      assert(0);
    }

    int last_percentage = 100;

    int count = 0;

    for (Xapian::MSetIterator i = matches.begin(); i != matches.end(); i++) {

      unsigned int sim = matches.convert_to_percent(i);
      assert( sim <= last_percentage );
      last_percentage = sim;
      //      cerr << "sim = " << sim << endl;

      Xapian::Document doc = i.get_document();
      string data = doc.get_data();

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

	  if ( commit_of_interest( commit_id, in_opt_list, commit_package, package_last_commit ) ) {
	    cout << commit_id << endl;
	    count ++;
            if ( count == max_results ) {
		goto done;
            }
	  }

	}
      }
    }

done: ;

    } catch(const Xapian::Error & error) {
      cerr << "Exception: " << error.get_msg() << endl;
    }  catch( DbException& e ) {
      cerr << "Exception:  " << e.what() << endl;
    }
  }
