/********************************************************************************
 * cvsrelatedindex.C
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



// for some reason, Xapian ranking is better with set than list for data mining purposes??
#warning "USING SET INSTEAD OF LIST FOR INDEX ENTRIES"

#warning "*** USING COMMENT PROFILES"

#warning "*** COMMIT OFFSET FILE SHARED BETWEEN MINE & COMMIT RIGHT NOW!"


#warning "perhaps we should not stem words in symbols at all"
#warning "but of course we should continue stemming in comment words"


// ??????????? is this info still correct below?
//     Generates Xapian databases each page.

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
static void write_OM_database( const string & database_dir,
                               const map<unsigned int, list<string> > & commit_comment_words
                               );


static void
get_data(lines & lines,
         const string & package_path, cvs_db_file & db_file,
         map<unsigned int, list<string> >  & commit_comment_words,
         set<unsigned int> & commit_id_set,        
         unsigned int offset);


int main(unsigned int argc, char *argv[]) {

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
  //  map<unsigned int, list <string> > commit_code_words;
  map<unsigned int, list<string> > commit_comment_words;
  //  map<unsigned int, list<string> > commit_all_words;


  unsigned int offset = 0;

#warning "writes to related.offset but uses commit.offset!!!"
  string commit_path = cvsdata + "/" + root + "/related.offset";
  ofstream fout(commit_path.c_str());

  try {
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
      lines_db  lines (root, package_path, " related", db_file);

      // cerr << "getdata " << endl;
      get_data(lines,
	       package_path,
	       db_file,
	       commit_comment_words,
	       commit_id_set,
	       offset);

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
    // data commit location.
    // ----------------------------------------
    string commit_path = cvsdata + "/" + root + "/db/related";

    // ----------------------------------------
    // printing # of commits to a file
    // commit.count
    // transactions are in commit_code_words
    // ----------------------------------------
    assert( commit_id_set.size() == commit_comment_words.size() );
    ofstream out((commit_path + ".count").c_str());
    out << commit_id_set.size() << endl;
    out.close();

    // ----------------------------------------
    // write out the berkeley database
    // mainly the frequency of each symbol and
    // # of times it appeared.
    // ----------------------------------------
    //    write_DB_database( commit_path + ".db",  commit_comment_words);

    // ----------------------------------------
    // write out the Xapian database
    // index by comment terms, the info field
    // should contain all the symbols
    // ----------------------------------------
    write_OM_database( commit_path + ".om",  commit_comment_words);
    //    write_OM_database( commit_path + ".om1", commit_code_words1, commit_comment_words1);
  } catch(const Xapian::Error & error) {
    cerr << "Xapian Exception: " << error.get_msg() << endl;
  } catch( DbException& e ) {
    cerr << "Sleepy Cat Exception:  " << e.what() << endl;
  }
}

void
usage(char * prog_name)
{
  cerr << "Usage: " << prog_name << "[Options]" << endl
       << endl
       << "Options:" << endl
       << "  -f pkg_list_file       a file containing the list of packages to mine" << endl
       << "  -r root                the root? directory under $CVSDATA where cvssearch information is stored" << endl
       << "  -h                     prints out this message" << endl
    ;

  exit(0);
}



// ----------------------------------------
// reason for const in set<>, list<> are because
// the values in the map should not be changed.
// reason for const in map<> is because
// the parameter shouldn't be changed
// ----------------------------------------

void write_OM_database( const string & database_dir,
                        const map<unsigned int, list<string> > & commit_comment_words
			)
{
  Xapian::WritableDatabase database(Xapian::Quartz::open(database_dir, Xapian::DB_CREATE_OR_OVERWRITE));

  int transactions_written = 0;

  map<unsigned int, list<string> >::const_iterator i;

  // iterate over commits
  for (i = commit_comment_words.begin(); i != commit_comment_words.end(); ++i)
    {
      string symbol_string = uint_to_string(i->first) + " "; // commit id comes first


      // find comment words associated with that commit
      map<unsigned int, list<string> >::const_iterator f = i; //commit_comment_words.find(i->first);


      //list<string> words; // words to index by
      set<string> words; // words to index by

      // index by stemmed comment words
      const list<string>& comment_words = f->second;
      for( list<string>::const_iterator j = comment_words.begin(); j != comment_words.end(); j++ ) {
	symbol_string += (*j) + " "; 
	//words.push_back(*j); // index by comment words (stemmed)
	words.insert(*j); // index by comment words (stemmed)
      }

      //      cerr << "DATA = " << symbol_string << endl;


      Xapian::Document newdocument;

      // ----------------------------------------
      // add terms for indexing
      // ----------------------------------------

      if ( words.empty() ) {
	//words.push_back("EMPTY");
	words.insert("EMPTY");
      }

      //list<string>::const_iterator w;
      set<string>::const_iterator w;
      for (w = words.begin(); w != words.end(); ++w) {
	newdocument.add_term_nopos(*w);
	   //cerr << "... index term " << (*w) << endl;
      }

      // ----------------------------------------
      // put transaction contents in data
      // ----------------------------------------
      newdocument.set_data(  symbol_string );

      database.add_document(newdocument);
      transactions_written++;
    }
#warning "why is this number lower than largest offset?"\
 "possibly because some lines where deleted and it doesn't reach the most current version?"\
 "since we build up transactions on a line by line basis from the most current version"
  cerr << "transactions written = " << transactions_written << endl;
}

void
get_data(lines & lines,
         const string & package_path,
         cvs_db_file & db_file,
         map<unsigned int, list<string> >  & commit_comment_words,
         set<unsigned int> & commit_id_set,
         unsigned int offset)
{
  unsigned int commitid = 0;
  unsigned int fileid = 0;
  string filename = "";

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

    if (strcmp(filename.c_str(), lines.getCurrentFile().c_str())) {
      filename = lines.getCurrentFile();
      filename = filename.substr(package_path.length() + 1, filename.length() - package_path.length() - 1);
      // ----------------------------------------
      // need to obtain file id
      // only when the file has changed.
      // ----------------------------------------
      if (db_file.get_fileid(fileid, filename) != 0) {
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


	  if ( commit_comment_words.find( commitid+offset ) == commit_comment_words.end() ) {
	    list<string> empty;
	    commit_comment_words[ commitid+offset ] = empty;
	  }



	  // ----------------------------------------
	  // have we entered info for this commit ?
	  // ----------------------------------------
	  if (commit_comment_words[commitid+offset].empty())
	    {
	      // ----------------------------------------
	      // nope, we have not. so need to set
	      // commit_comment_words[commitid]
	      // ----------------------------------------
	      const map<string, list<string> > & terms = lines.getRevisionCommentWords();
	      map<string, list<string> >::const_iterator itr = terms.find(i->first);
	      if (itr != terms.end()) {
		const list<string>& words = itr->second;
		for( list<string>::const_iterator i = words.begin(); i != words.end(); i++ ) {
		    commit_comment_words[commitid+offset].push_back(*i);
		}
	      }
	    }
	}
    }
  }
}

void
compare(map<unsigned int, set <string> > & commit_code_words,
        map<unsigned int, set<string> >  & commit_comment_words,
        map<unsigned int, set <string> > & commit_code_words1,
        map<unsigned int, set<string> >  & commit_comment_words1
        )
{
  assert(commit_comment_words1.size()   == commit_comment_words.size());
  assert(commit_code_words1.size() == commit_code_words.size());
  if (commit_comment_words1.size()   != commit_comment_words.size()) {
    cerr << "commit_comment_words1   size" << commit_comment_words1.size() << endl;
    cerr << "commit_comment_words    size" << commit_comment_words.size() << endl;
  }
  if (commit_code_words1.size() != commit_code_words.size()) {
    cerr << "commit_code_words1 size" << commit_code_words1.size() << endl;
    cerr << "commit_code_words  size" << commit_code_words.size() << endl;
  }
  map<unsigned int, set<string> >::const_iterator itr;
  map<unsigned int, set<string> >::const_iterator itr1;
  for (itr = commit_code_words.begin(), itr1 = commit_code_words1.begin();
       itr != commit_code_words.end() && itr1 != commit_code_words1.end();
       ++itr, ++itr1)
    {
      if ((itr->second).size() != (itr1->second).size()) {
	cerr << "symbols  size " << (itr->second).size() << endl;
	cerr << "symbols1 size " << (itr1->second).size() << endl;
      }
      assert((itr->second).size() == (itr1->second).size());
      set<string>::const_iterator sitr;
      set<string>::const_iterator sitr1;
      for(sitr = (itr->second).begin(), sitr1 = (itr1->second).begin();
	  sitr != (itr->second).end() && sitr1 != (itr1->second).end();
	  ++sitr, ++sitr1)
        {
	  if ((strcmp ((*sitr).c_str(), (*sitr1).c_str()))) {
	    cerr << "from symbols  " << *sitr << endl;
	    cerr << "from symbols1 " << *sitr1<< endl;
	  }
	  assert(!strcmp ((*sitr).c_str(), (*sitr1).c_str()));
        }
    }

  for (itr = commit_comment_words.begin(), itr1 = commit_comment_words1.begin();
       itr != commit_comment_words.end() && itr1 != commit_comment_words1.end();
       ++itr, ++itr1)
    {
      cerr << "commit id  " << (itr->first) << endl;
      cerr << "commit id1 " << (itr1->first) << endl;
      if ((itr->second).size() != (itr1->second).size()) {
	cerr << "words    size " << (itr->second).size() << endl;
	cerr << "words1   size " << (itr1->second).size() << endl;
      }
      assert((itr->second).size() == (itr1->second).size());
      set<string>::const_iterator sitr;
      set<string>::const_iterator sitr1;
      for(sitr = (itr->second).begin(), sitr1 = (itr1->second).begin();
	  sitr != (itr->second).end() && sitr1 != (itr1->second).end();
	  ++sitr, ++sitr1)
        {
	  if (strcmp ((*sitr).c_str(), (*sitr1).c_str())) {
	    cerr << "from words  " << *sitr << endl;
	    cerr << "from words1 " << *sitr1<< endl;
	  }
	  assert(!strcmp ((*sitr).c_str(), (*sitr1).c_str()));
        }
    }

}
