/********************************************************************************
 * cvscommitindex.C
 *
 * (c) 2001 Amir Michail (amir@users.sourceforge.net)
 * modified by Andrew Yao (andrewy@users.sourceforge.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Usage:     cvscommitindex -r root0 -f package_list_file 
 *
 *           => builds a directory
 *                        $CVSDATA/root0/db/commit.om with quartz database
 *                         inside
 *           => builds a directory
 *                        $CVSDATA/root0/db/commit.db with berkerley db
 *           => builds a file
 *                        $CVSDATA/root0/db/commit.count with # of commits
 *
 ********************************************************************************/

#warning "SHOULD PROBABLY USE LIST FOR INDEX ENTRIES -- NOT SET !!"

#warning "*** USING COMMENT PROFILES"

#warning "*** COMMIT OFFSET FILE SHARED BETWEEN MINE & COMMIT RIGHT NOW!"


#warning "perhaps we should not stem words in symbols at all"\
 "but of course we should continue stemming in comment words"

// latent semantic indexing is probably a better way to do this
// can also use idf instead of stop list

#include "stoplist.h"

// ??????????? is this info still correct below?
//     Generates Xapian databases each page.
//
//     If library directories given, also generates a "commit" Xapian database
//     for library usage
//

// ------------------------------------------------------------
// General approach:
//
// For each symbol, we identify all comments that were associated
//  with lines containing that symbol.
//
// We do not put duplicate comments in the symbol's profile, so
// the profiles are reasonably small.
//
//
// Query => classes/functions related to the task at hand, both
// at a local & global level.
//
//
// Reasoning for looking at symbols in the most recent copy and
// not on a commit by commit basis in previous versions:
//
//    code symbols change over time but not the domain concepts
//    we describe them by in comments
// ------------------------------------------------------------

#include <xapian.h>
#include <unistd.h>
#include <db_cxx.h>
#include <fstream>
#include <stdio.h>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <math.h>
#include <algorithm>

#include "cvs_db_file.h"
#include "util.h"

// ----------------------------------------
// function declarations.
// ----------------------------------------
static void usage(char * prog_name);
static void write_OM_database( Xapian::WritableDatabase &database,
                               const map<string, list<string> > & commit_code_words,
                               const map<string, list<string> > & commit_comment_words
                               );

static void write_DB_database( const string & database_file,
                               const map<string, list<string> > & commit_code_words
                               );

static void
get_data(lines & lines,
         const string & package_path, cvs_db_file & db_file,
         map<string, list <string> > & commit_code_words,
         map<string, list<string> >  & commit_comment_words,
         map<string, list<string> >  & commit_all_words,
         set<unsigned int> & commit_id_set,        
         unsigned int offset);

int main(unsigned int argc, char *argv[]) {
  stoplist_init();

  string cvsdata = get_cvsdata();
  string root = "";
  set<string> packages;

  for (unsigned int i = 1; i < argc; ++i) {
    if (0) {
    } else if (!strcmp(argv[i],"-f") && i+1 < argc) {
      ifstream fin(argv[++i]);
      get_packages(fin, packages);
    } else if (!strcmp(argv[i],"-r") && i+1 < argc) {
      root = argv[++i];
    } else /*if (!strcmp(argv[i],"-h")) */ {
      usage(argv[0]);
    }
  }
  if (root.length() == 0) {
    usage(argv[0]);
  }


  // ----------------------------------------
  // This is the key:   It takes a commit id
  // (global) to all the symbols under that commit.
  // ----------------------------------------
  // commit_code_words[commit_id] -> code symbols.
  // commit_comment_words  [commit_id] -> stemmed words.

  set<unsigned int> commit_id_set; // set of all commit ids

#warning "file name (should include repository prefix?)"



  unsigned int offset = 0;

  string commit_path = cvsdata + "/" + root + "/file.offset";
  ofstream fout(commit_path.c_str());

  try {

    // ----------------------------------------
    // data commit location.
    // ----------------------------------------
    string commit_path = cvsdata + "/" + root + "/db/file";

    string database_dir = commit_path + ".om";
    Xapian::WritableDatabase database(Xapian::Quartz::open(database_dir, Xapian::DB_CREATE_OR_OVERWRITE));

    // ----------------------------------------
    // go through each package
    // ----------------------------------------
    set<string>::const_iterator i;
    for ( i = packages.begin(); i != packages.end(); i++ ) {
      string package_path = *i;                              // e.g. kdebase/konqueror
      string package_name = convert(package_path, '/', '_'); // e.g. kdebase_konqueror
      string package_db_path  = cvsdata + "/" + root + "/db/" + package_name; // e.g. ...cvsdata/root0/db/kdebase_konqueror
      string package_src_path = cvsdata + "/" + root + "/src/" + package_path;// e.g. ...cvsdata/root0/src/kdebase/konqueror

      // ------------------------------------------------------------
      // need first input to be
      // "...cvsdata/root0/db/pkg" + ".db/" + pkg.db"
      // ------------------------------------------------------------
      cvs_db_file db_file(package_db_path + ".db/" + package_name + ".db", true);
      lines_db  lines (root, package_path, " file", db_file);

      // cerr << "getdata " << endl;

      map<string, list <string> > commit_code_words;
      map<string, list<string> > commit_comment_words;
      map<string, list<string> > commit_all_words;

      get_data(lines,
	       package_path,
	       db_file,
	       commit_code_words,
	       commit_comment_words,
	       commit_all_words,
	       commit_id_set,
	       offset);

      
      cerr << "... WRITING TO OM DB" << endl;


      // write to OM file after every file
      write_OM_database( database,  commit_code_words, commit_comment_words);


      // cerr << "getdata1" << endl;

      // ----------------------------------------
      // writing to the offset of each package
      // commits here. (pkg, commit_offset)
      // so later we can do
      // global commit id-> (pkg, local commit id)
      // ----------------------------------------
      unsigned int count;
      if (db_file.get_commit_count(count) == 0) {
	fout << package_path << " " << offset << endl;
	offset += count;
	cerr << "... offset now " << offset << endl;
      }
    } // for packages
    fout.close();

    // ----------------------------------------
    // write results
    // ----------------------------------------



    // ----------------------------------------
    // printing # of commits to a file
    // commit.count
    // transactions are in commit_code_words
    // ----------------------------------------
    ofstream out((commit_path + ".count").c_str());
    //    out << commit_comment_words.size() << endl; // # files
    out.close();

    // ----------------------------------------
    // write out the berkeley database
    // mainly the frequency of each symbol and
    // # of times it appeared.
    // ----------------------------------------
    //    write_DB_database( commit_path + "_cmt.db",  commit_comment_words);
    //    write_DB_database( commit_path + "_code.db",  commit_code_words);


    // ----------------------------------------
    // write out the Xapian database
    // index by comment terms, the info field
    // should contain all the symbols
    // ----------------------------------------

    // write_OM_database( commit_path + ".om",  commit_code_words, commit_comment_words);

  } catch(const Xapian::Error & error) {
    cerr << "Xapian Exception: " << error.get_msg() << endl;
  } catch( DbException& e ) {
    cerr << "Sleepy Cat Exception:  " << e.what() << endl;
  }
}

void
usage(char * prog_name)
{
  cerr << "Usage: " << prog_name << " [Options]" << endl
       << endl
       << "Options:" << endl
       << "  -f pkg_list_file       a file containing the list of packages to mine" << endl
       << "  -r root                the root? directory under $CVSDATA where cvssearch information is stored" << endl
       << "  -h                     prints out this message" << endl
    ;

  exit(0);
}

void write_DB_database( const string & database_file,
                        const map<string, list<string> > & commit_all_words)
{
  // ----------------------------------------
  // we also want to generate a sleepy cat
  // database with the following
  // information
  //
  // a=>b -> confidence (irrespective of query)
  // b -> confidence (irrespective of query)
  // this can all be put in one database
  //
  // item_count maps each code symbol to # of
  // times it appeared in the entire
  // package.
  // ----------------------------------------
  map< string, unsigned int > item_count;
  map<string, list<string> >::const_iterator t;
  for (t = commit_all_words.begin(); t!= commit_all_words.end(); ++t) {
    const list<string> & symbols = t->second;
    //cerr << "... transaction of size " << symbols.size() << endl;

    set<string> seen_already;
    list<string>::const_iterator s;
    for (s = symbols.begin(); s != symbols.end(); ++s) {
      if ( seen_already.find(*s) == seen_already.end() ) {
	//cerr << "........... symbol " << (*s) << endl;
	++(item_count[*s]); /////////////////////////////////// counted by ignoring repeats!!!!
	seen_already.insert(*s);
      }
    }
  }

  // ----------------------------------------
  // write to a berkeley db, each symbol
  // and # of times it has appeared.
  // ----------------------------------------
  cerr << "... removing previous version (if any) of " << database_file << endl;
  system( ("rm -rf " + database_file ).c_str() );
  cerr << "... writing out item counts" << endl;

  Db db(0,0);
#if DB_VERSION_MAJOR > 4 || (DB_VERSION_MAJOR == 4 && DB_VERSION_MINOR >= 1)
  db.open(NULL, database_file.c_str(), 0, DB_HASH, DB_CREATE, 0);
#else
  db.open(database_file.c_str(), 0, DB_HASH, DB_CREATE, 0);
#endif

  //cerr << "... writing counts to db file -" << database_file << "-" << endl;

  map<string, unsigned int>::const_iterator i;
  for(i = item_count.begin(); i != item_count.end(); ++i) {
    string item = i->first;
    string count = uint_to_string(i->second);

    //cerr << "... writing item " << item << " with count " << count << endl;

    // ----------------------------------------
    // write to database
    // ----------------------------------------
    Dbt key ( (void*) item.c_str(), item.length()+1);
    Dbt data( (void*) count.c_str(), count.length()+1);
    db.put( 0, &key, &data, DB_NOOVERWRITE );
  }
  db.close(0);
}



// ----------------------------------------
// reason for const in set<>, list<> are because
// the values in the map should not be changed.
// reason for const in map<> is because
// the parameter shouldn't be changed
// ----------------------------------------

void write_OM_database( Xapian::WritableDatabase &database,
                        const map<string, list<string> > & commit_code_words,
                        const map<string, list<string> > & commit_comment_words
			)
{
  /***
  Xapian::WritableDatabase database(Xapian::Quartz::open(database_dir, Xapian::DB_CREATE_OR_OVERWRITE)); // open database
  ***/

  int transactions_written = 0;

  map<string, list<string> >::const_iterator i;

  // iterate over commits
  for (i = commit_code_words.begin(); i != commit_code_words.end(); ++i)
    {
      string symbol_string = i->first + " "; // commit id comes first


      // find comment words associated with that commit
      map<string, list<string> >::const_iterator f = commit_comment_words.find(i->first);


#warning " *** CHANGE TO SET WHEN YOU DO QUERY EXPANSION"
      list<string> words; // words to index by

      assert( f != commit_comment_words.end() );

      // index by stemmed comment words
      const list<string>& comment_words = f->second;
      for( list<string>::const_iterator j = comment_words.begin(); j != comment_words.end(); j++ ) {
	symbol_string += "+"+(*j) + " "; // comment words have '+' prefix
	words.push_back(*j); // index by comment words (stemmed)
      }

      // also index by code words
      const list<string> & symbols = i->second;
      list<string>::const_iterator j;
      for (j = symbols.begin(); j != symbols.end(); ++j) {
	symbol_string += (*j) + " ";
	words.push_back(*j); // also index by code words (stemmed)
      }

      // cerr << "DATA = " << symbol_string << endl;


      Xapian::Document newdocument;

      // ----------------------------------------
      // add terms for indexing
      // ----------------------------------------

      if ( words.empty() ) {
	words.push_back("EMPTY");
      }

      list<string>::const_iterator w;
      for (w = words.begin(); w != words.end(); ++w) {
	newdocument.add_term_nopos(*w);
	   //cerr << "... index term " << (*w) << endl;
      }

      /***************
       // ----------------------------------------
       // add symbols for indexing (symbols get a
       // : prefix to distinguish them from terms)
       // ----------------------------------------
        for(j = symbols.begin(); j != symbols.end(); ++j) {
            newdocument.add_term_nopos(":"+(*j));
            //cerr << "... symbol " << (":"+(*j)) << endl;
        }
      ***************/

      // ----------------------------------------
      // put transaction contents in data
      // ----------------------------------------
      newdocument.set_data(  symbol_string );

      database.add_document(newdocument);
      //      cerr << "Just wrote " << symbol_string << endl;      
      transactions_written++;
    }
#warning "why is this number lower than largest offset?"
#warning "possibly because some lines where deleted and it doesn't reach the most current version?"
#warning "since we build up transactions on a line by line basis from the most current version"
  cerr << "transactions written = " << transactions_written << endl;
}

void
get_data(lines & lines,
         const string & package_path,
         cvs_db_file & db_file,
         map<string, list<string> > & commit_code_words,
         map<string, list<string> >  & commit_comment_words,
         map<string, list<string> >  & commit_all_words,	 
         set<unsigned int> & commit_id_set,
         unsigned int offset)
{
  unsigned int commitid = 0;
  unsigned int fileid = 0;
  string full_filename = "";
  string package_filename = "";

  map< string, set< unsigned int > > file_commits;

  while ( lines.readNextLine() ) {
    string data = lines.getData();

    /***
        set<string> symbols = lines.getCodeSymbols();

        cerr << "Just read line " << data << endl;
        for( set<string>::iterator i = symbols.begin(); i != symbols.end(); i++ ) {
	cerr << "... symbol " << (*i) << endl;
        }
    ***/

    list<string> symbol_terms = lines.getCodeSymbolTerms();

    /***
        for( set<string>::iterator i = symbol_terms.begin(); i != symbol_terms.end(); i++ ) {
	cerr << "... term " << (*i) << endl;
        }
    **/

    if (strcmp(package_filename.c_str(), lines.getCurrentFile().c_str())) {
      full_filename = lines.getCurrentFile();
      package_filename = full_filename.substr(package_path.length() + 1, full_filename.length() - package_path.length() - 1);
      // ----------------------------------------
      // need to obtain file id
      // only when the file has changed.
      // ----------------------------------------
      if (db_file.get_fileid(fileid, package_filename) != 0) {
	fileid = 0;
      }
    }
#warning "when is fileid zero?"
    // ----------------------------------------
    // fileid is zero only when the given filename
    // is not a file we are interested in.
    // (ie not in the berkeley database)
    // ----------------------------------------
    if (fileid == 0) {
      continue;
    }
    // ----------------------------------------
    // here we have a mapping between revision
    // for this line and the associated CVS comment.
    // ----------------------------------------
    const map<string, string > & revisions = lines.getRevisionCommentString();
    map<string, string >::const_iterator i;

    // cycle through every revision that this line is part of
    for(i = revisions.begin(); i != revisions.end(); ++i ) {
      // cerr << "get commit with input " << fileid << " " << i->first << endl;
      if (db_file.get_commit(fileid, i->first, commitid) == 0)
	{
	  // cerr << "got commit " << commitid << " with input " << fileid << " " << i->first << endl;
	  commit_id_set.insert( commitid+offset ); // set of all commit ids


	  if ( commit_comment_words.find( full_filename ) == commit_comment_words.end() ) {
	    list<string> empty;
	    commit_comment_words[ full_filename ] = empty;
	  }
	  if ( commit_code_words.find( full_filename ) == commit_code_words.end() ) {
	    list<string> empty;
	    commit_code_words[ full_filename ] = empty;
	  }
	  if ( commit_all_words.find( full_filename ) == commit_all_words.end() ) {
	    list<string> empty;
	    commit_all_words[ full_filename ] = empty;
	  }

	  // ----------------------------------------
	  // have we entered info for this commit ?
	  // ----------------------------------------

          // enter commit comment only once per file
	  if ( file_commits[full_filename].find(commitid+offset) == file_commits[full_filename].end() ) 
	    {

	      file_commits[full_filename].insert(commitid+offset);

	      // ----------------------------------------
	      // nope, we have not. so need to set
	      // commit_comment_words[commitid]
	      // ----------------------------------------
	      const map<string, list<string> > & terms = lines.getRevisionCommentWords();
	      map<string, list<string> >::const_iterator itr = terms.find(i->first);
	      if (itr != terms.end()) {
		const list<string>& words = itr->second;
		for( list<string>::const_iterator i = words.begin(); i != words.end(); i++ ) {
		  if ( stopSet.find(*i) == stopSet.end() ) {
		    commit_comment_words[full_filename].push_back(*i);
		    commit_all_words[full_filename].push_back(*i);		  
		  }
		}
	      }

	    } else {
		//cerr << "already have commit " << (commitid+offset) << " for " << full_filename << endl;
          }

	  // ----------------------------------------
	  // now go through each symbol,
	  // and add it to the commit_code_words mapping
	  // ----------------------------------------
	  // this is to be done for every line associated with the commit
	  for( list<string>::iterator s = symbol_terms.begin(); s != symbol_terms.end(); ++s ) {
	    if ( stopSet.find(*s) == stopSet.end() ) {
	      commit_code_words[full_filename].push_back(*s);
	      commit_all_words[full_filename].push_back(*s);
	    }
	  }
	}
    }
  }
}

