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
// Example:  cvssearch kdeutils_kfind 10 ftp nfs
//
//     Returns the top 10 lines with both ftp and nfs.
//
// (package is the directory with the quartz database inside)
//

#warning "sometimes have '(null)' in results"

#include <om/om.h>
#include <db_cxx.h>
#include <stdio.h>
#include <math.h>

#include "util.h"



int main(int argc, char *argv[]) {
     if(argc < 3) {
        cout << "Usage: " << argv[0] <<
                " <path to database> <search terms>" << endl;
        exit(1);
    }

     string cvsdata;
     char *s = getenv("CVSDATA");
     if ( s==0 ) {
       cerr <<" Warning:  $CVSDATA not set, using current directory." << endl;
       cvsdata = ".";
     } else {
       cvsdata = s;
       // strip trailing / if any
       if ( cvsdata[cvsdata.length()-1] == '/' ) {
	 cvsdata = cvsdata.substr( 0, cvsdata.length()-1 );
       }
       //       cerr << "$CVSDATA = " << cvsdata << endl;
     }


     set<string> packages;

     int qpos;
     int npos;

     // get packages from cmd line or from file
     if ( isdigit(argv[1][0] )) {
       // get packages from file
       string p;
       while (!cin.eof()) {
	 cin >> p;
	 packages.insert(p);
       }
       npos = 1;
       qpos = 2;
     } else {
       // get package from cmd line
       packages.insert( argv[1] );
       npos = 2;
       qpos = 3;
     }

     try {
       OmStem stemmer("english");

       assert( qpos == argc-1 );

       om_termname term = argv[qpos];
       lowercase_term(term);
       term = stemmer.stem_word(term);
       string queryterm = term;
       cout << term << endl;

       int num_results = atoi( argv[npos] );

       bool doFunctions = ( string(argv[0]).find("functions") != -1 );

       map< double, set<string> > results;
       
       // cycle over all packages
       for( set<string>::iterator p = packages.begin(); p != packages.end(); p++ ) {

	 string package = *p;

	 // change / to _ in package
	 for( int i = 0; i < package.length(); i++ ) {
	   if ( package[i] == '/' ) {
	     package[i] = '_';
	   }
	 }


	 //	 cerr << "...processing " << package << endl;
	 Db db(0,0);
	 if ( doFunctions ) {
	   db.open( (cvsdata+"/"+package+".functions").c_str(), 0, DB_HASH, DB_RDONLY, 0);
	 } else {
	   db.open( (cvsdata +"/"+package+".classes").c_str(), 0, DB_HASH, DB_RDONLY, 0);
	 }

	 Dbt key((void*) queryterm.c_str(), queryterm.length()+1);
	 Dbt data;
	 db.get(0, &key, &data, 0);
	 //	 cout << (char*)data.get_data();

	 string data_str = string((char*)data.get_data());
	 list<string> lines;
	 split( data_str, "\n", lines );
	 for( list<string>::iterator line = lines.begin(); line != lines.end(); line++ ) {
	   string score_str = line->substr(0, line->find(" "));
	   double score = atof(score_str.c_str());
	   // cerr << "*** score = " << score << " " << (*line) << endl;
	   results[-score].insert(*line);
	 }
	 db.close(0);

	 int c = 0;
	 for( map<double, set<string> >::iterator i = results.begin(); i != results.end(); i++ ) {
	   double score = i->first;
	   set<string> S = i->second;
	   for( set<string>::iterator s = S.begin(); s != S.end(); s++ ) {
	     cout << (*s) << endl;
	     c++;
	     if ( c == num_results ) {
	       goto done;
	     }
	   }
	 }
       done: ;
       }
       
    } catch( DbException& e ) {
      cerr << "Exception:  " << e.what() << endl;     
    } 

     
}
