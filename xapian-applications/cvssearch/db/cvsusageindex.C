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
//     Generates Xapian databases each page.
//
//     If library directories given, also generates a "global" Xapian database
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
#include <fstream>
#include <stdio.h>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <math.h>
#include <algorithm>
#include <xapian.h>

#include "util.h"


void usage(char * prog_name);
const string database = "db";

void writeDatabase( const string& database_dir, map<string, int>& app_symbol_count, map<string, set<list<string> > >& app_symbol_terms ) {
  cerr << "... creating databases in directory " << database_dir << endl;
  assert( database_dir != "." );
  assert( database_dir != "..");

  // code which accesses Xapian

  // open databases
  Xapian::WritableDatabase database_classes(Xapian::Quartz::open(database_dir+"_c", Xapian::DB_CREATE_OR_OVERWRITE));
  Xapian::WritableDatabase database_functions(Xapian::Quartz::open(database_dir+"_f", Xapian::DB_CREATE_OR_OVERWRITE));

  for( map<string, int>::iterator c = app_symbol_count.begin(); c != app_symbol_count.end(); c++ ) {
    string symbol = c->first;

    int count = c->second; //app_symbol_terms[symbol].size();  //= c->second;

    bool isFunction = ( symbol.find("()") != -1 );

    cerr <<"*** Symbol " << symbol << " has count " << count << ":" << endl;

    // merge all lists together, the number of lists is approx. the # of commits
    // with that symbol
    list<string> words;
    for( set< list<string> >::const_iterator s = app_symbol_terms[symbol].begin(); s != app_symbol_terms[symbol].end(); s++ ) {
      list<string> L = *s;
      for (list<string>::const_iterator l = L.begin(); l != L.end(); l++ ) {
	words.push_back(*l);
	cerr << (*l) << " ";
      }
      cerr << endl;
    }

    Xapian::Document newdocument;
    for( list<string>::iterator i = words.begin(); i != words.end(); i++ ) {
	  
      string word = *i;
      //	  cerr << "... " << word << endl;
      newdocument.add_term_nopos(word);
    }

    assert( symbol.size() <= 1000 );
    static char str[4096];
    sprintf(str, "%d %s", count, symbol.c_str());
    assert( strlen(str) <= 1000 );
    newdocument.set_data( string(str) );

    if ( isFunction ) {
      //      cerr << "Data -" << newdocument.data << endl;
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
      readTags( TEMP "/tags", app_symbols, app_symbol_tag );

      for( map<string, string>::iterator i = app_symbol_tag.begin(); i != app_symbol_tag.end(); i++ ) {
	cerr << "Line " << i->first << " has " << i->second << endl;
      }


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

      Lines lines( cvsdata + "/root0/src/", "", package, file_cmt, file_offset, " accumulating" ); 

      string current_function = "";

      while ( lines.ReadNextLine() ) {

	string data = lines.getData();
	map<string, list<string> > terms = lines.getRevisionCommentWords();
	set<string> symbols = lines.getCodeSymbols();
          
	string file = cvsdata+"/root0/src/"+lines.currentFile();
	unsigned int line_no = lines.getLineNumber();
	//	cerr << "Looking at line " << file << "\t" << line_no << endl;
	string st = file + '\t' + uint_to_string(line_no);
	if ( app_symbol_tag.find(st) != app_symbol_tag.end() ) {
	  string f = app_symbol_tag[st];
#warning "need to be careful with one line functions since they only have one tag"
	  cerr << "FOUND FUNCTION " << f << " at line " << line_no << endl;

	  if ( f == current_function ) {
	    current_function = ""; // end tag
	  } else {
	    current_function = f;
	  }
	}

	//	  if ( app_symbols.find(*i) != app_symbols.end() ) {
	//	    app_symbol_count[*i]++; // count number of lines that contain symbol

	//	    cerr << "*** APP SYMBOL " << (*i) << endl;

	// for each revision comment associated with this line, we insert it

	if ( current_function != "" ) {

	  app_symbol_count[current_function] = 1;

	  for( map<string, list<string> >::iterator r = terms.begin(); r != terms.end(); r++ ) {
	    list<string> word_list = r->second;

	      app_symbol_terms[ current_function ].insert(word_list); // avoid repetition

	  }

	}
	//	  }

	//////////////////// see if this is a one line function, if so it won't have another tag!
	//////////////////// hack...
#warning "CHECK FOR ONE LINE FUNCTIONS"
	

	  
	// look at symbols in line
	for( set<string>::iterator i = symbols.begin(); i != symbols.end(); i++ ) {
              
	  if ( SKIP_FUNCTIONS && i->find("()") != -1 ) {
	    continue;
	  }

	  //	  if ( app_symbols.find(*i) != app_symbols.end() ) {              
	  if ( lib_symbols.find(*i) != lib_symbols.end() ) {

	    if ( lib_symbol_app_count[*i] < MAX_FROM_APP ) {
	      
	      if ( found_symbol_before.find(*i) == found_symbol_before.end() ) {
		lib_symbol_count[*i]++; // count number of lines that contain symbol
		found_symbol_before.insert(*i);
	      } 
	      
	      for( map<string, list<string> >::iterator r = terms.begin(); r != terms.end(); r++ ) {
		list<string> word_list = r->second;

		if ( word_list.size() <= MAX_COMMENT_WORDS ) {
		  lib_symbol_terms[*i].insert(word_list); // avoid repetition
		}
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
      /// write out results to Xapian
      writeDatabase( database_dir, app_symbol_count, app_symbol_terms );
	
	

    } // for packages

    /// write out results to Xapian
    cerr << "Writing global" << endl;
    writeDatabase( "global.om", lib_symbol_count, lib_symbol_terms );

  } catch(const Xapian::Error & error) {
    cerr << "Xapian Exception: " << error.get_msg() << endl;
  } catch( DbException& e ) {
    cerr << "Exception:  " << e.what() << endl;
  }


}
