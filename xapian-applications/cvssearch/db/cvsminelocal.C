// cvsmine.C
//
// (c) 2001 Amir Michail (amir@users.sourceforge.net)

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

//
// Usage:  cvsminelocal < PACKAGE_LIST
//
//     Generates Berkeley database files with results.
//           

/***********

KWord seems easiest to test.

Some queries to try:

    *  Normal editing and formatting (fonts, colors, attributes, super/sub script, etc.)
    * Paragraph styles (alignment, spacings, indentations, default font, etc.)
    * Stylist to edit, add, remove and update paragraph styles. There are predefined ones like Standard Text, Header 1 - 3, Bullet List, Ordered List, etc.
    * Frame orientation (for DTP you can insert multiple frames and connect/disconnect some of them, which means defining a text flow)
    * Multiple columns
    * Inserting Tables
    * Inserting Pictures
    * Embedding other KOffice parts
    * In-Place formula editor
    * Letting text of one (text)frame run around another frame(s)
    * Headers/Footers
    * Endnotes
    * Numbering of chapters
    * Automatic generation of Table of Contents
    * Inserting Predefined variables (Date, Time, Page Number, etc.)
    * Inserting and defining custom variables
    * Generating serial letters
    * Autocorrection
    * Spell checking (uses ISpell)
    * Document structure viewer
    * Templates
    * Creating own Templates


************/


//
//
// Local mining:
//
//  symbols - from app itself
//  
//
// Global mining:
// 
//    give a list of apps as usual, but specify
//    global option, e.g., -g file.list
//    which are the files to use symbols from
//   
//   The counting is also different in global minine.
//   We only count apps, not individual lines.
//   We also generate one set of database files, not
//   one per app.
//





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

  set<string> packages;

  int qpos;
  int npos;

  // get packages from cmd line or from file
  if ( argc == 1 ) {
    // get packages from file
    string p;
    while (!cin.eof()) {
      cin >> p;
      cerr << "... will process " << p << endl;
      packages.insert(p);
    }
    npos = 1;
    qpos = 2;
  } else {
    // get package from cmd line
    cerr << "... will process " << argv[1] << endl;
    packages.insert( argv[1] );
    npos = 2;
    qpos = 3;
  }

     


  system("rm -f /home/amichail/temp/tags"); 

  for ( set<string>::iterator i = packages.begin(); i != packages.end(); i++ ) {
    
    string package = *i + ".cmt";
    
    int p = package.find(".cmt");
    if ( p == -1 ) {
      cerr << "Must include .cmt extension in package(s)." << endl;
      exit(1);
    }

    package = string(package, 0, p);

    cerr << "package -" << package << "-" << endl;

    cerr << "Running ctags on " << package << endl;
    string fullpath = cvsdata +"/" + package;
    string cmd = string("ctags ") + string(CTAGS_FLAGS) + " " + fullpath;
    cerr << "Invoking " << cmd << endl;
    system(cmd.c_str());
    cerr << "Done" << endl;






    set<string> defined_symbols;
    readTags( "/home/amichail/temp/tags", defined_symbols );




    // change / to _ in package
    for( int i = 0; i < package.length(); i++ ) {
      if ( package[i] == '/' ) {
	package[i] = '_';
      }
    }

    string file_cmt = package + ".cmt";
    string file_offset = package +".offset";

    map<string, int> symbol_count;
    map<string, int> term_count;
    //    set<string> apps;

    int lines_read = 0;

    try {

      { // pass 1
	cerr << "PASS 1" << endl;
	Lines lines( cvsdata, package, file_cmt, file_offset, GRANULARITY, USE_STOP_LIST ); // file level granularity
	lines_read = 0;
	string prev_file = "";
	while ( lines.ReadNextLine() ) {

	  if ( lines.currentFile() != prev_file ) {
	    // run ctags on file
	    /****
	    string cmd = string("ctags ") + string(CTAGS_FLAGS) + " " + cvsdata +"/"+lines.currentFile();
	    system(cmd.c_str());
	    ***/
	    prev_file = lines.currentFile();
	  }

	  set<string> terms = lines.getCommentTerms();
	  set<string> symbols = lines.getCodeSymbols();

	  //	  string app = extractApp( lines.currentFile() );
	  //	  apps.insert(app);

	  for( set<string>::iterator i = terms.begin(); i != terms.end(); i++ ) {
	    term_count[*i]++;
	  }

	  for( set<string>::iterator i = symbols.begin(); i != symbols.end(); i++ ) {
	    if ( defined_symbols.find(*i) != defined_symbols.end() ) {
	      symbol_count[*i]++;
	    }
	  }
	  lines_read++;
	} // while
      }


      map< pair<string, string>, set<string> > rule_support;
      
      { // pass 2
	cerr << "PASS 2" << endl;
	Lines lines( cvsdata, package, file_cmt, file_offset, GRANULARITY, USE_STOP_LIST );
	
	lines_read = 0;

	while ( lines.ReadNextLine() ) {

	  set<string> terms = lines.getCommentTerms();
	  set<string> symbols = lines.getCodeSymbols();
	  
	  //	  string app = extractApp( lines.currentFile() );


	  for( set<string>::iterator t = terms.begin(); t != terms.end(); t++ ) {
	    if ( term_count[*t] >= MIN_SUPP ) {
	      for ( set<string>::iterator s = symbols.begin(); s != symbols.end(); s++ ) {

		if ( defined_symbols.find(*s) == defined_symbols.end() ) {
		  continue;
		}

		if ( symbol_count[*s] >= MIN_SUPP ) {

		  rule_support[ make_pair( *t, *s ) ].insert( lines.getData() );

		}
	      }
	    }

	  }
	  lines_read++;	  
	} // while
      }

      cerr << "*** lines read " << lines_read << endl;


      // write results to two database files

      Db dbclasses(0,0), dbfunctions(0,0);
      
      dbclasses.open( (cvsdata+"/"+package+".classes").c_str() , 0 , DB_HASH, DB_CREATE, 0);
      dbfunctions.open( (cvsdata+"/"+package+".functions").c_str() , 0 , DB_HASH, DB_CREATE, 0);
      

      /////// we have term_count, symbol_count, rule_support (term=>symbol)

      string prev_term = "";
      string entryclasses;
      string entryfunctions;
      // print out rules
      for ( map< pair<string, string>, set<string> >::iterator r = rule_support.begin(); r != rule_support.end(); r++ ) {
	int supp = (r->second).size();
	string ant = (r->first).first;
	string con = (r->first).second;

	if ( ant != prev_term ) { 
	  // print out results in sorted order

	  // What we need to store in the database
	  //
	  // term => symbol
	  //
	  // we also need to know all lines in which (term, symbol) are found
	  // this includes information about the file id, etc.

	  // let's try to use the standard format:
	  //
	  //	  0.453 15:80 kdepim/korganizer:1.8 1.3 1.1
	  //              0.454 16:80 kdepim/korganizer:1.8 1.3
	  //              0.467 17:80 kdepim/korganizer:1.8 1.1
	  //              0.453 266:80 kdebase/konqueror:1.8 1.3 1.1
	  //              0.353 267:80 kdebase/konqueror:1.3 1.1
	  //              0.443 17:87 kdepim/korganizer:1.12 1.2
	  //              0.453 18:87 kdepim/korganizer:1.13 1.4 1.1
	  //              0.453 19:87 kdepim/korganizer:1.14 1.6 1.1

	  // we should probably have other commands to show you these
	  //
	  // What happens if you search multiple applications?  What rules
	  // should we show?
	  //
	  // cvslocalpatterns < APPS.txt 10 10 term 
	  //
	  // cvspatterns --local --classes < APPS.txt 10 term
	  // cvspatterns --global --classes 10 term
	  //
	  // (max # classes, max # functions)
	  //
	  // What should it output?  Should we put it in several files?
	  //
	  // class KMainWindow
	  //	  0.453 15:80 kdepim/korganizer:1.8 1.3 1.1
	  //              0.454 16:80 kdepim/korganizer:1.8 1.3
	  //              0.467 17:80 kdepim/korganizer:1.8 1.1
	  //              0.453 266:80 kdebase/konqueror:1.8 1.3 1.1
	  //              0.353 267:80 kdebase/konqueror:1.3 1.1
	  //              0.443 17:87 kdepim/korganizer:1.12 1.2
	  //              0.453 18:87 kdepim/korganizer:1.13 1.4 1.1
	  // 
	  // function exec()
	  //	  0.453 15:80 kdepim/korganizer:1.8 1.3 1.1
	  //              0.454 16:80 kdepim/korganizer:1.8 1.3
	  //              0.467 17:80 kdepim/korganizer:1.8 1.1
	  //              0.453 266:80 kdebase/konqueror:1.8 1.3 1.1
	  //              0.353 267:80 kdebase/konqueror:1.3 1.1
	  //              0.443 17:87 kdepim/korganizer:1.12 1.2
	  //              0.453 18:87 kdepim/korganizer:1.13 1.4 1.1
	  //
	  // What database files do we need to generate all this?
	  //
	  // For app X, say we have:
	  //   term -> 
	  //       KMainWindow\n15:80 kdepim/korganizer:1.8 1.3 1.1\n0.454 16:80 kdepim/korganizer:1.8 1.3
	  //  For each app, you store files:
	  //
	  //   
	  // Basically, we just need two databases.  X.classes and X.functions
	  //
	  // 


	  cerr << "*** ENTRY FOR " << prev_term << endl << entryclasses << entryfunctions << endl;

	  if ( entryclasses != "" ) {
	    Dbt key( (void*) prev_term.c_str(), prev_term.length()+1);
	    Dbt data( (void*) entryclasses.c_str(), entryclasses.length()+1);
	    dbclasses.put( 0, &key, &data, DB_NOOVERWRITE );	  
	  }
	  if ( entryfunctions != "" ) {
	    Dbt key( (void*) prev_term.c_str(), prev_term.length()+1);
	    Dbt data( (void*) entryfunctions.c_str(), entryfunctions.length()+1);
	    dbfunctions.put( 0, &key, &data, DB_NOOVERWRITE );	  
	  }
	    
	  prev_term = ant;
	  entryclasses = "";
	  entryfunctions = "";
	}


	
	if ( supp >= MIN_SUPP ) {

	  double con_conf = 100.0*(double)symbol_count[con] / (double)lines_read;

	  double conf = 100.0*(double)supp / (double)term_count[ant];

	  double surprise = (conf / con_conf ) * (double)supp; // log(1.1+(double)supp);

	  bool isFunction = ( con.find("()") != -1 );

	  static char str[256];
	  sprintf(str, "%f", surprise);
	  
	  if ( isFunction ) {
	    entryfunctions += string(str) + " " + con;
	  } else {
	    entryclasses += string(str) + " " + con;
	  }

	  if ( surprise >= MIN_SURPRISE ) {
	    //	    cerr << ant << " => " << con << " has conf " << conf << " and support " << supp << " with con conf " << con_conf << endl;
	    //	    results[-surprise].insert(ant + " => " + con);

	    // we can dump everything to the database at this point
	    // database depends on whether we have a class or a function


	    set<string> L = r->second;
	    for( set<string>::iterator l = L.begin(); l != L.end(); l++ ) {
	      //	      cerr << "..." << surprise << " " << (*l) << endl;
	      if ( isFunction ) {
		entryfunctions += " " + (*l);
	      } else {
		entryclasses += " " + (*l);
	      }
	    }
	  }
	  if ( isFunction ) {
	    entryfunctions += "\n";
	  } else {
	    entryclasses += "\n";
	  }
	}
      }

      cerr << "*** ENTRY FOR " << prev_term << endl << entryclasses << entryfunctions << endl;
      if ( entryclasses != "" ) {
	Dbt key( (void*) prev_term.c_str(), prev_term.length()+1);
	Dbt data( (void*) entryclasses.c_str(), entryclasses.length()+1);
	dbclasses.put( 0, &key, &data, DB_NOOVERWRITE );	  
      }
      if ( entryfunctions != "" ) {
	Dbt key( (void*) prev_term.c_str(), prev_term.length()+1);
	Dbt data( (void*) entryfunctions.c_str(), entryfunctions.length()+1);
	dbfunctions.put( 0, &key, &data, DB_NOOVERWRITE );	  
      }

      dbclasses.close(0);
      dbfunctions.close(0);


    } catch( DbException& e ) {
      cerr << "Exception:  " << e.what() << endl;     
    } 

  } // for packages
  
}
