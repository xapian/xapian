// cvsfileindex.C
//
// (c) 2001 Amir Michail (amir@users.sourceforge.net)

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

//
// Usage:  cvsusageindex < PACKAGE_LIST lib1_dir lib2_dir ...
//
//     Generates omsee databases each page.
//
//     If library directories given, also generates a "mining" omsee database
//     for library usage
//

#warning "should generate an om database for each app"
#warning "should be able to search multiple databases easily using om"

// ctags options
//  want classes
//  want public/protected member *functions*
//  ignore inheritance for now...

// -R (recursive)
// --c-types=cfsuAC
// --kind-long=yes (verbose tag descriptions)
// from this, ignore all entries with access:private

// /tmp is small, so we use /tmp

#define TEMP "/tmp"


//
// Reasoning for looking at symbols in the most recent copy and
// not on a commit by commit basis in previous versions:
//
//    code symbols change over time but not the domain concepts
//    we describe them by in comments
//

#include <db_cxx.h>
#include <fstream.h>
#include <strstream>
#include <stdio.h>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <math.h>
#include <algorithm>
#include <om/om.h>

#include "util.h"


void usage(char * prog_name);
const string database = "db";

void writeOMDatabase( const string& database_dir,
		      map< string, set<string> >& comment_symbols, 
		      map< string, list<string> >& comment_words ) {
  
  system( ("rm -rf " + database_dir).c_str() );
  system(("mkdir " + database_dir).c_str());

  OmSettings db_parameters;
  db_parameters.set("backend", "quartz");
  db_parameters.set("quartz_dir", database_dir);
  db_parameters.set("database_create", true);
  OmWritableDatabase database(db_parameters); // open database 
    
  for( map< string, set<string > >::iterator i = comment_symbols.begin(); i != comment_symbols.end(); i++ ) {
    string cmt = i->first;

    set<string> symbols = i->second;
    string symbol_string;
    for( set<string>::iterator j = symbols.begin(); j != symbols.end(); j++ ) {
      symbol_string = symbol_string + (*j) + " ";
    }

    //      cerr << "Looking at comment " << cmt << endl;
    list<string> W = comment_words[cmt];
      
    OmDocument newdocument;
    int pos = 1;

    // add terms for indexing
    set<string> added;
    for( list<string>::iterator w = W.begin(); w != W.end(); w++ ) {
      if ( added.find(*w) != added.end() ) {
	continue; // added already, save some space by skipping
      }
      newdocument.add_posting(*w, pos++); 
      added.insert(*w);
    }

    //      cerr << "Symbol string is:  " << symbol_string << endl;

    // put transaction contents in data
    newdocument.set_data(  symbol_string );

    database.add_document(newdocument);

  }
    
    
}

int main(int argc, char *argv[]) {


  // for each package, we process it seperately

  string cvsdata = get_cvsdata();

  // get list of packages to process from file

  set<string> packages;

  int qpos;
  int npos;

  // get apps from stdin
  string p;
  while (!cin.eof()) {
    cin >> p;
    if ( cin.eof() && p == "" ) {
      break;
    }
    cerr << "... will process " << p << endl;
    packages.insert(p);
  }
  npos = 1;
  qpos = 2;
  



  // might be easier to just maintain something like:  file:revision
  // that we way do not duplicate comments
  //
  // in fact, we can use:
  //
  // cvsquery -c file_id revision to get the comment
  //


  ///////// This is the key map:  It takes a commit comment to all the symbols under that comment
  map< string, set<string> > comment_files;
  map< string, set<string> > comment_symbols;
  map< string, list< string > > comment_words;



  try {

      

    for ( set<string>::iterator i = packages.begin(); i != packages.end(); i++ ) {

      string package = *i + ".cmt";

      string package_name;
      string package_path;

      unsigned int p = package.find(".cmt");
      unsigned int q = package.find_last_of('/');

      if ( p == string::npos ) {
	cerr << "Must include .cmt extension in package(s)." << endl;
	exit(1);
      }
      // ----------------------------------------
      // no '/', so use current directory
      // ----------------------------------------
      if ( q == string::npos) {
	q = 0;
      }
      if ( q >= p )
	{
	  cerr << "Cannot parse package.cmt. found a \"/\" after \".cmt\" in the filename." << endl;
	  exit(1);
	}
      package_name = string(package, q, p-q);
      package_path = string(package, 0, p);
      
      // change / to _ in package
      for( unsigned int i = 0; i < package_path.length(); i++ ) {
          if ( package_path[i] == '/' ) {
              package_path[i] = '_';
          }
      }
      
      package_path = cvsdata +"/root0/db/"+package_path;

      //    string file_cmt = cvsdata+"/database/"+package + ".cmt";
      //    string file_offset = cvsdata +"/database/"+package +".offset";






      // int lines_read = 0;


      package = string(package, q, p);
      cerr << "package -" << package_name << "-" << endl;

      assert( package != "." ); // safety checks
      assert( package != ".." );


      string file_cmt    = package_path + ".cmt";
      string file_offset = package_path + ".offset";

      // file may not exist (if it was deleted in repostory at some point)
      {
	ifstream in( file_cmt.c_str() );
	if ( !in ) {
	  cerr << "Could not find " << file_cmt << endl;
	  continue;
	}
      }

      cerr << "... reading " << file_cmt << endl;

      map< string, set<list<string> > > app_symbol_terms; // accumulated from all its points of usage
      map<string, int> app_symbol_count;
      set<string> found_symbol_before;

      lines_cmt lines( cvsdata + "/root0/src/", "", package, file_cmt, file_offset, " mining" ); 

      while ( lines.readNextLine() ) {

          string data = lines.getData();
          map<string, list<string> > terms = lines.getRevisionCommentWords();
          set<string> symbols = lines.getCodeSymbols();


	//	cerr << "Line " << data << endl;

	//// basically, we need to cycle over all the comments here
	map<string, string > revisions = lines.getRevisionCommentString();
	for( map<string, string >::iterator i = revisions.begin(); i != revisions.end(); i++ ) {
	  
	  //	  cerr << "Revision " << i->first << " has comment " << i->second << endl;

	  comment_words[i->second] = terms[i->first];
	  comment_files[i->second].insert( lines.getCurrentFile() );

	  for( set<string>::iterator s = symbols.begin(); s != symbols.end(); s++ ) {
	      comment_symbols[ i->second].insert(*s);
	  }

	}

      } // while


    } // for packages


    // write results
    // 
    // index by comment terms, the info field should contain all the symbols


    // we also want to generate sleepy cat databases with the following
    // information
    //
    // a=>b -> confidence (irrespective of query)
    // b -> confidence (irrespective of query)
    // this can all be put in one database








    /////// data mining step
    /////// transactions are in comment_symbols
    
    int transaction_count = comment_symbols.size();

    cerr << "# transactions = " << transaction_count << endl;

    map< string, int > item_count;

    // count items
    cerr << "... counting items" << endl;
    for( map< string, set<string> >::iterator t = comment_files.begin(); t != comment_files.end(); t++ ) {
      set<string> S = t->second;
      for( set<string>::iterator s = S.begin(); s != S.end(); s++ ) {
	item_count[*s]++;
      }
    }


    cerr << "... writing out item counts" << endl;

    system( ("rm -rf " + cvsdata +"/root0/db/files.count" ).c_str() );
    ofstream out( (cvsdata+"/root0/db/files.count").c_str() );
    out << transaction_count << endl;
    out.close();

    system( ("rm -rf " + cvsdata +"/root0/db/files.db" ).c_str() );
    
    Db db(0,0);
    db.open( (cvsdata +"/root0/db/files.db").c_str(),  0 , DB_HASH, DB_CREATE, 0 );


    for( map<string, int>::iterator i = item_count.begin(); i != item_count.end(); i++ ) {
      int count = i->second;
      string item = i->first;
      

      ostrstream ost;
      ost << count << ends;
      string s = ost.str();
      
      // write to database
      Dbt key( (void*) item.c_str(), item.length()+1);
      Dbt data( (void*) s.c_str(), s.length()+1);
      db.put( 0, &key, &data, DB_NOOVERWRITE );
      ost.freeze(0);
    }

    db.close(0);








    /////// write out OM database
    writeOMDatabase( cvsdata + "/root0/db/files.om", 
		     comment_files,
		     comment_words );

  } catch(OmError & error) {
    cerr << "OMSEE Exception: " << error.get_msg() << endl;
  } catch( DbException& e ) {
    cerr << "Exception:  " << e.what() << endl;
  }


}
