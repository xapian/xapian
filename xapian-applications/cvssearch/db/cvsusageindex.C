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

//
// General approach:
//
//   For each symbol, we identify all comments that were associated
//   with lines containing that symbol.
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

#define TOTAL_WORDS 9999999
#define MAX_WORDS 9999999
#define MIN_WORDS 1

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
#define CTAGS_FLAGS "-R --fields=aiK --extra=q --c-types=cfsu --java-types=cim --kind-long=yes -f" TEMP "/tags"


//
// Reasoning for looking at symbols in the most recent copy and
// not on a commit by commit basis in previous versions:
//
//    code symbols change over time but not the domain concepts
//    we describe them by in comments
//

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


void usage(char * prog_name);
const string database = "db";

void writeDatabase( const string& database_dir, map<string, int>& app_symbol_count, map<string, set<list<string> > >& app_symbol_terms ) {

  cerr << "... removing directory " << database_dir << " (if it already exists)" << endl;
  assert( database_dir != "." );
  assert( database_dir != "..");
  //  system( ("rm -rf " + database_dir).c_str() );
  system( ("rm -rf " + database_dir+"_c").c_str() );
  system( ("rm -rf " + database_dir+"_f").c_str() );


  // ----------------------------------------
  // create database directory
  // ----------------------------------------
  system(("mkdir " + database_dir+"_c" ).c_str());
  system(("mkdir " + database_dir+"_f" ).c_str());

  // code which accesses Omsee

  OmSettings db_parameters_classes;
  db_parameters_classes.set("backend", "quartz");
  db_parameters_classes.set("quartz_dir", database_dir+"_c");
  db_parameters_classes.set("database_create", true);
  OmWritableDatabase database_classes(db_parameters_classes); // open database

  OmSettings db_parameters_functions;
  db_parameters_functions.set("backend", "quartz");
  db_parameters_functions.set("quartz_dir", database_dir+"_f");
  db_parameters_functions.set("database_create", true);
  OmWritableDatabase database_functions(db_parameters_functions); // open database



  for( map<string, int>::iterator c = app_symbol_count.begin(); c != app_symbol_count.end(); c++ ) {
    string symbol = c->first;
    int count = c->second;
    bool isFunction = ( symbol.find("()") != -1 );

    cerr <<"*** Symbol " << symbol << " has count " << count << ":" << endl;

    // merge all lists together
    list<string> words;
    for( set< list<string> >::const_iterator s = app_symbol_terms[symbol].begin(); s != app_symbol_terms[symbol].end(); s++ ) {
      list<string> L = *s;
      for (list<string>::const_iterator l = L.begin(); l != L.end(); l++ ) {
	words.push_back(*l);
	cerr << (*l) << " ";
      }
      cerr << endl;
    }

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
      database_classes.add_document(newdocument);
    }
  }


  cerr << "Done!" << endl;
}


int main(int argc, char *argv[]) {


  // for each package, we process it seperately

  string cvsdata = get_cvsdata();

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
  readTags( TEMP "/tags", lib_symbols );

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
      map<string, int> lib_symbol_app_count;
      readTags( TEMP "/tags", app_symbols );




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
      map< string, int > contributed_terms;

      Lines lines( cvsdata + "/root0/src/", "", package, file_cmt, file_offset, " accumulating" ); 

      while ( lines.ReadNextLine() ) {

	string data = lines.getData();
	map<string, list<string> > terms = lines.getRevisionCommentWords();
	set<string> symbols = lines.getCodeSymbols();
          
	for( set<string>::iterator i = symbols.begin(); i != symbols.end(); i++ ) {
              
	  if ( SKIP_FUNCTIONS && i->find("()") != -1 ) {
	    continue;
	  }

	  if ( app_symbols.find(*i) != app_symbols.end() ) {
	    app_symbol_count[*i]++; // count number of lines that contain symbol

	    //	    cerr << "*** APP SYMBOL " << (*i) << endl;

	    // for each revision comment associated with this line, we insert it
	    for( map<string, list<string> >::iterator r = terms.begin(); r != terms.end(); r++ ) {
	      list<string> word_list = r->second;

	      /*
	      for( list<string>::iterator t = word_list.begin(); t != word_list.end(); t++ ) {
		cerr << (*t) << " ";
	      }
	      cerr << endl;
	      */

	      app_symbol_terms[*i].insert(word_list); // avoid repetition
	    }



	  }
              
	  if ( terms.size() < MIN_WORDS ) {
	    continue; 
	  }

	  if ( terms.size() > MAX_WORDS ) {
	    continue; 
	  }

	  if ( contributed_terms[*i] + terms.size() >= TOTAL_WORDS ) {
	    continue;
	  }

	  if ( lib_symbols.find(*i) != lib_symbols.end() ) {

	    if ( lib_symbol_app_count[*i] < MAX_FROM_APP ) {
	      
	      lib_symbol_count[*i]++; // count number of lines that contain symbol
	      
	      for( map<string, list<string> >::iterator r = terms.begin(); r != terms.end(); r++ ) {
		list<string> word_list = r->second;

		for( list<string>::iterator t = word_list.begin(); t != word_list.end(); t++ ) {
		  //lib_symbol_word_list[*i].push_back(*t);
		  contributed_terms[*i]++;
		}


		lib_symbol_terms[*i].insert(word_list); // avoid repetition
	      }

	      //	      cerr << "** NOT ignoring " << (*i) << endl;
	      lib_symbol_app_count[*i]++;
	    } else {
	      //	      cerr << "** ignoring " << (*i) << endl;
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
