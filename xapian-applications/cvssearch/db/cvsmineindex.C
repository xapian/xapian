// cvsmine.C
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
//     If library directories given, also generates a "global" omsee database
//     for library usage
//

//
// General approach:
//
//   For each symbol, we identify all comments that were associated
//   with lines containing that symbol.

//
// We do not put duplicate comments in the symbol's profile, so the
// profiles are reasonably small.  
//

//
//   Query => classes/functions related to the task at hand, both at a
//                    local & global level.
//
//     






// should have another command for classes/functions that look
// at their contents 

#define SKIP_FUNCTIONS 0

#define MAX_FROM_APP 9999999

// makes things to limit things in this way because symbols
// could occur in many places.

#define MAX_COMMENT_WORDS 40


#warning "requires ctags from http://ctags.sourceforge.net/"
#warning "should generate unique file for tags"
#warning "ctags contains inheritance information; this can help if (t,S) does not occur in class declaration say or where member variable is declared"
#warning "requires omsee 0.4.1"

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

// support C/C++/Java for now

// ctags 5.0 flags (see http://ctags.sourceforge.net/ctags.html)
//
#define CTAGS_FLAGS "-R -n --file-scope=no --fields=aiKs --c-types=cfsu --java-types=cim -f" TEMP "/tags"


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



int main(int argc, char *argv[]) {


  // for each package, we process it seperately

  string cvsdata = get_cvsdata();

  // get list of packages to process from file

  set<string> lib_symbols;
  map<string, string > lib_symbol_tag;

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
  


  // get libraries if any from cmd line
  system("rm -f " TEMP "/tags"); 

  for( int i = 1; i < argc; i++ ) {
    string dir = argv[i];
    cerr << "...running ctags on library " << dir << endl;
    string cmd = string("ctags -a ") + string(CTAGS_FLAGS) + " " + dir; // append mode
    cerr << "...invoking " << cmd << endl;
    system(cmd.c_str());
  }
  
  cerr << "...reading library tags" << endl;
#warning "not reading library tags"
  //  readTags( TEMP "/tags", lib_symbols, lib_symbol_tag );

  // might be easier to just maintain something like:  file:revision
  // that we way do not duplicate comments
  //
  // in fact, we can use:
  //
  // cvsquery -c file_id revision to get the comment
  //

  // we consider each comment only once
  map< string, set<list<string> > > lib_symbol_terms; // accumulated from all its points of usage

  map<string, int> lib_symbol_count;



  ///////// This is the key map:  It takes a commit comment to all the symbols under that comment
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
      
      cerr << "package -" << package_name << "-" << endl;
      system("rm -f " TEMP "/tags" );

      cerr << "Running ctags on " << package_path << endl;
      string fullpath = cvsdata +"/root0/src/" + package_path;
      string cmd = string("ctags ") + string(CTAGS_FLAGS) + " " + fullpath;
      cerr << "Invoking " << cmd << endl;
      system(cmd.c_str());
      cerr << "Done" << endl;


      set<string> app_symbols;
      map< string, string > app_symbol_tag;

      map<string, int> lib_symbol_app_count;
      //      readTags( TEMP "/tags", app_symbols, app_symbol_tag );

      // change / to _ in package
      for( int i = 0; i < package_path.length(); i++ ) {
	if ( package_path[i] == '/' ) {
	  package_path[i] = '_';
	}
      }
      
      package_path = cvsdata +"/root0/db/"+package_path;

      //    string file_cmt = cvsdata+"/database/"+package + ".cmt";
      //    string file_offset = cvsdata +"/database/"+package +".offset";






      int lines_read = 0;


      package = string(package, q, p);
      cerr << "package -" << package_name << "-" << endl;

      assert( package != "." ); // safety checks
      assert( package != ".." );


      string file_cmt    = package_path + ".cmt";
      string file_offset = package_path + ".offset";
      string database_dir= package_path + ".om";

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

      Lines lines( cvsdata + "/root0/src/", "", package, file_cmt, file_offset, " mining" ); 

      string current_function = "";

      while ( lines.ReadNextLine() ) {

	string data = lines.getData();
	map<string, list<string> > terms = lines.getRevisionCommentWords();
	set<string> symbols = lines.getCodeSymbols();


	//	cerr << "Line " << data << endl;

	//// basically, we need to cycle over all the comments here
	map<string, string > revisions = lines.getRevisionCommentString();
	for( map<string, string >::iterator i = revisions.begin(); i != revisions.end(); i++ ) {
	  
	  //	  cerr << "Revision " << i->first << " has comment " << i->second << endl;

	  comment_words[i->second] = terms[i->first];

	  for( set<string>::iterator s = symbols.begin(); s != symbols.end(); s++ ) {
	    comment_symbols[ i->second].insert(*s);
	    //	    cerr << "..." << (*s) << endl;
	  }

	}

      } // while


    } // for packages


    // write results
    // 
    // index by comment terms, the info field should contain all the symbols

    string package_path = cvsdata +"/root0/db/global";

    string database_dir2= package_path + ".om2";
    system( ("rm -rf " + database_dir2).c_str() );
    system(("mkdir " + database_dir2 ).c_str());

    OmSettings db_parameters2;
    db_parameters2.set("backend", "quartz");
    db_parameters2.set("quartz_dir", database_dir2);
    db_parameters2.set("database_create", true);
    OmWritableDatabase database2(db_parameters2); // open database 
    
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

      for( list<string>::iterator w = W.begin(); w != W.end(); w++ ) {
	//	cerr << "..." << (*w) << endl;
	
	newdocument.add_posting(*w, pos++); 
      }

      //      cerr << "Symbol string is:  " << symbol_string << endl;

      // put transaction contents in data
      newdocument.set_data(  symbol_string );

      database2.add_document(newdocument);

    }
    
    
    
  } catch(OmError & error) {
    cerr << "OMSEE Exception: " << error.get_msg() << endl;
  } catch( DbException& e ) {
    cerr << "Exception:  " << e.what() << endl;
  }


}
