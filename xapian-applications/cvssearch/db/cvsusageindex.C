// cvsusageindex.C
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

// should have another command for classes/functions that look
// at their contents 

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

#define GRANULARITY "line"
#define USE_STOP_LIST false

// /tmp is small, so we use /home/amichail/temp
#define CTAGS_FLAGS "-R --c-types=cfsuAC --kind-long=yes -f/home/amichail/temp/tags"


//
// Reasoning for looking at symbols in the most recent copy and
// not on a commit by commit basis in previous versions:
//
//  code symbols change over time but not the domain concepts
//  we describe them by in comments
//

// if in file mode, this is the minimum # files
// 15 is reasonable but takes a while


#define MIN_SUPP 1
#define MIN_SURPRISE 0.0

// if true, counts files.
// however, still requires line pairing of comment term, code symbol

// app count not exactly correct because files not handled
// in order of app exactly but also by order of extension
// (e.g., .h, .cc, .C, .cpp, etc.), so some apps may contribute
// 2 or 3 to the count


#include <db_cxx.h>
#include <fstream.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <math.h>
#include <algorithm>
#include <om/om.h>

#include "util.h"


// every 100 symbols
#define FLUSH_RATE 500 

void usage(char * prog_name);
const string database = "db";

#define SKIP_CLASSES 0 

void writeDatabase( const string& database_dir, map<string, int>& app_symbol_count, map<string, list<string> >& app_symbol_terms ) {

  cerr << "... removing directory " << database_dir << " (if it already exists)" << endl;
  assert( database_dir != "." );
  assert( database_dir != "..");
  system( ("rm -rf " + database_dir).c_str() );
  system( ("rm -rf " + database_dir+"_c").c_str() );
  system( ("rm -rf " + database_dir+"_f").c_str() );


  // ----------------------------------------
  // create database directory
  // ----------------------------------------
  system(("mkdir " + database_dir+"_c" ).c_str());
  system(("mkdir " + database_dir+"_f" ).c_str());

  // code which accesses Omsee

#if !SKIP_CLASSES
  OmSettings db_parameters_classes;
  db_parameters_classes.set("backend", "quartz");
  db_parameters_classes.set("quartz_dir", database_dir+"_c");
  db_parameters_classes.set("database_create", true);
  OmWritableDatabase database_classes(db_parameters_classes); // open database

#endif


  OmSettings db_parameters_functions;
  db_parameters_functions.set("backend", "quartz");
  db_parameters_functions.set("quartz_dir", database_dir+"_f");
  db_parameters_functions.set("database_create", true);
  OmWritableDatabase database_functions(db_parameters_functions); // open database



  int f_count = 0;
  int c_count = 0;
  for( map<string, int>::iterator c = app_symbol_count.begin(); c != app_symbol_count.end(); c++ ) {
    string symbol = c->first;
    int count = c->second;
    bool isFunction = ( symbol.find("()") != -1 );

    if ( isFunction ) {
      f_count++;

/***
      if ( f_count % FLUSH_RATE == 0 ) {
	cerr << "*** FLUSHING FUNCTIONS" << endl;
	database_functions.flush();
      }
***/
    } else {
      c_count++;
#if !SKIP_CLASSES
/***
      if ( c_count % FLUSH_RATE == 0 ) {
	cerr << "*** FLUSHING CLASSES" << endl;
	database_classes.flush();
      }
**/
#endif

    }



    //    cerr <<"*** Symbol " << symbol << " has count " << count << endl;
    list<string> words = app_symbol_terms[symbol];

    OmDocument newdocument;
    int pos = 1;
    for( list<string>::iterator i = words.begin(); i != words.end(); i++ ) {
	  
      string word = *i;
      //	  cerr << "..." << pos << " " << word << endl;
      newdocument.add_posting(word, pos++); // term, position of term
    }

    assert( symbol.size() <= 1000 );
    static char str[4096];
    sprintf(str, "%d %s", count, symbol.c_str());
    assert( strlen(str) <= 1000 );
    newdocument.set_data( string(str) );

    if ( isFunction ) {
      //      cerr << "Data -" << newdocument.data << "- with # words = " << pos << endl;
      database_functions.add_document(newdocument);
    } else {
#if !SKIP_CLASSES
      database_classes.add_document(newdocument);
#endif
    }
  }


  cerr << "Done!" << endl;
}


int main(int argc, char *argv[]) {


  // for each package, we process it seperately

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


  // get list of packages to process from file

  set<string> lib_symbols;
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
  


  // get libraries if any
  system("rm -f /home/amichail/temp/tags"); 
  for( int i = 1; i < argc; i++ ) {
    string dir = argv[i];
 

    cerr << "Running ctags on " << dir << endl;
    string cmd = string("ctags -a ") + string(CTAGS_FLAGS) + " " + dir; // append mode
    cerr << "Invoking " << cmd << endl;
    system(cmd.c_str());
    cerr << "Done" << endl;
    
  }

  readTags( "/home/amichail/temp/tags", lib_symbols );


  system("rm -f /home/amichail/temp/tags");





  map< string, list<string> > lib_symbol_terms; // accumulated from all its points of usage
  map<string, int> lib_symbol_count;

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

      cerr << "Running ctags on " << package_path << endl;
      string fullpath = cvsdata +"/" + package_path;
      string cmd = string("ctags ") + string(CTAGS_FLAGS) + " " + fullpath;
      cerr << "Invoking " << cmd << endl;
      system(cmd.c_str());
      cerr << "Done" << endl;






      set<string> app_symbols;
      readTags( "/home/amichail/temp/tags", app_symbols );




      // change / to _ in package
      for( int i = 0; i < package_path.length(); i++ ) {
	if ( package_path[i] == '/' ) {
	  package_path[i] = '_';
	}
      }

      package_path = "database/"+package_path;

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

      map< string, list<string> > app_symbol_terms; // accumulated from all its points of usage
      map<string, int> app_symbol_count;

      Lines lines( cvsdata, package, file_cmt, file_offset, GRANULARITY, USE_STOP_LIST ); // file level granularity

      while ( lines.ReadNextLine() ) {

	string data = lines.getData();
	list<string> terms = lines.getTermList();
	set<string> symbols = lines.getCodeSymbols();



	for( set<string>::iterator i = symbols.begin(); i != symbols.end(); i++ ) {

	  if ( app_symbols.find(*i) != app_symbols.end() ) {
	    app_symbol_count[*i]++; // count number of lines that contain symbol

	    for( list<string>::iterator t = terms.begin(); t != terms.end(); t++ ) {
	      app_symbol_terms[*i].push_back(*t);
	    }
	  }

	  if ( lib_symbols.find(*i) != lib_symbols.end() ) {
	    lib_symbol_count[*i]++; // count number of lines that contain symbol

	    for( list<string>::iterator t = terms.begin(); t != terms.end(); t++ ) {
	      lib_symbol_terms[*i].push_back(*t);
	    }
	  }


	}
	lines_read++;
      } // while


      cerr << "Writing " << database_dir << endl;
	/// write out results to omsee
      writeDatabase( database_dir, app_symbol_count, app_symbol_terms );
	
	

    } // for packages

    /// write out results to omsee
    cerr << "Writing global" << endl;
    writeDatabase( "global.om", lib_symbol_count, lib_symbol_terms );

  } catch(OmError & error) {
    cerr << "OMSEE Exception: " << error.get_msg() << endl;
  } catch( DbException& e ) {
    cerr << "Exception:  " << e.what() << endl;
  }


}
