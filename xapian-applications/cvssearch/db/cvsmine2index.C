/********************************************************************************
 * cvsmine2index.C
 * 
 * (c) 2001 Amir Michail (amir@users.sourceforge.net)
 * modified by Andrew Yao (andrewy@users.sourceforge.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Usage:     cvsmineindex -r root0 -f package_list_file lib1_dir lib2_dir
 * 
 *           => builds a directory 
 *                        $CVSDATA/root0/db/mining2.om with quartz database
 *                         inside
 *           => builds a directory
 *                        $CVSDATA/root0/db/mining2.db with berkerley db 
 *           => builds a file
 *                        $CVSDATA/root0/db/mining2.count with # of commits
 * 
 ********************************************************************************/


// ??????????? is this info still correct below?
//     Generates Xapian databases each page.
//
//     If library directories given, also generates a "mining" Xapian database
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

#warning "requires ctags 5.0 from http://ctags.sourceforge.net/"
#warning "should generate unique file for tags"
#warning "ctags contains inheritance information; this can help"
#warning "if (t,S) does not occur in class declaration say or where member variable is declared"


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
#include <xapian.h>

#include "cvs_db_file.h"
#include "util.h"

// ----------------------------------------
// ctags options
//  want classes
//  want public/protected member *functions*
//  ignore inheritance for now...

// -R (recursive)
// --c-types=cfsuAC
// --kind-long=yes (verbose tag descriptions)
// from this, ignore all entries with access:private

// /tmp is small, so we use /tmp
// ----------------------------------------



// ----------------------------------------
// function declarations.
// ----------------------------------------
static void usage(char * prog_name);
static void write_OM_database( const string & database_dir,
                               const map<unsigned int, set <string> > & commit_symbols, 
                               const map<unsigned int, list<string> > & commit_words );

static void write_DB_database( const string & database_file,
                               const map<unsigned int, set<string> > & commit_symbols);


int main(unsigned int argc, char *argv[]) {

    string cvsdata = get_cvsdata();
    string root = "";
    set<string> packages;

    for (unsigned int i = 1; i < argc; ++i)
    {
        if (0) {
        } else if (!strcmp(argv[i],"-f") && i+1 < argc) {
            ifstream fin(argv[++i]);
            get_packages(fin, packages);
        } else if (!strcmp(argv[i],"-r") && i+1 < argc) {
            root = argv[++i];
        } else if (!strcmp(argv[i],"-h")) {
            usage(argv[0]);
        } else {
		cerr << "No libraries please." << endl;
		assert(0);

        }
    }
    if (root.length() == 0) {
	cerr << "Must specify root!\n";
        usage(argv[0]);
    }
    
    
    string package_path;        // e.g. kdebase/konqueror
    string package_name;        // e.g. kdebase_konqueror
    string package_db_path;     // e.g. ...cvsdata/root0/db/kdebase_konqueror
    string package_src_path;    // e.g. ...cvsdata/root0/src/kdebase/konqueror
        
    // ----------------------------------------
    // This is the key:   It takes a commit id
    // to all the symbols under that commit.
    // ----------------------------------------
    // commit_symbols[commit_id] -> code symbols.
    // commit_words  [commit_id] -> stemmed words.
    map<unsigned int, set <string> > commit_symbols;
    map<unsigned int, list<string> > commit_words;
    
    try {
        // ----------------------------------------
        // go through each package
        // ----------------------------------------
        set<string>::const_iterator i;
        for ( i = packages.begin(); i != packages.end(); i++ ) {
            package_path = *i;
            package_name = convert(package_path, '/', '_');
            package_db_path  = cvsdata + "/" + root + "/db/" + package_name;
            package_src_path = cvsdata + "/" + root + "/src/" + package_path;

      
            // map< unsigned int, const set<list<string> > > app_symbol_terms; // accumulated from all its points of usage
            // map< unsigned int, int> app_symbol_count;
            // set<string> found_symbol_before;
            
            // ----------------------------------------
            // this line NEEDS to be changed..
            // ----------------------------------------
            lines_cmt lines(root, "", "", "", " mining" ); 
            
            // ------------------------------------------------------------
            // need first input to be 
            // "...cvsdata/root0/db/pkg" + ".db/" + pkg.db"
            // ------------------------------------------------------------
            cvs_db_file db_file(package_db_path + ".db/" + package_name + ".db", true);
            unsigned int commitid = 0;
            unsigned int fileid = 0;
            string filename = "";
            while ( lines.readNextLine() ) {
                string data = lines.getData();
                set<string> symbols = lines.getCodeSymbolTerms();
                
                if (strcmp(filename.c_str(), lines.getCurrentFile().c_str())) {
                    filename = lines.getCurrentFile();
                    // ----------------------------------------
                    // need to obtain file id
                    // only when the file has changed.
                    // ----------------------------------------
                    if (db_file.get_fileid(fileid, filename) != 0) {
                        fileid = 0;
                    }
                }
                if (fileid == 0) {
                    continue;
                }
                
                // ----------------------------------------
                // here we have a mapping between revision
                // for this line and the associated CVS comment.
                // ----------------------------------------
                const map<string, string > & revisions = lines.getRevisionCommentString();
                map<string, string >::const_iterator i;
                
                for(i = revisions.begin(); i != revisions.end(); ++i ) {
                    if (db_file.get_commit(fileid, i->first, commitid) == 0) {
                        // ----------------------------------------
                        // have we entered info for this commit ?
                        // ----------------------------------------
                        if (commit_words.find(commitid) == commit_words.end()) 
                        {
                            // ----------------------------------------
                            // nope, we have not. so need to set
                            // commit_words[commitid]
                            // ----------------------------------------
                            const map<string, list<string> > & terms = lines.getRevisionCommentWords();
                            map<string, list<string> >::const_iterator itr = terms.find(i->first);
                            if (itr != terms.end()) {
                                commit_words[commitid] = itr->second;
                            }
                        }
                        // ----------------------------------------
                        // now go through each symbol,
                        // and add it to the commit_symbols mapping
                        // ----------------------------------------
                        for( set<string>::iterator s = symbols.begin(); s != symbols.end(); ++s ) {
                            if ( 1 /*lib_symbols.find(*s) != lib_symbols.end()*/ ) {
                                commit_symbols[commitid].insert(*s);
                            } else {
				assert(0);
                                // ----------------------------------------
                                // this symbol is not in the library, so 
                                // let's see if its parents are;
                                // if so, we add every such parent
                                // ----------------------------------------
/***
                                set<string> parents = app_symbol_parents[*s];
                                for( set<string>::iterator p = parents.begin(); p != parents.end(); ++p ) {
                                    if ( lib_symbols.find(*p) != lib_symbols.end() ) {
                                        commit_symbols[commitid].insert(*p);
                                    }
                                }
**/
                            }
                        }
                    }
                }
            }
        } // for packages

        // ----------------------------------------
        // write results
        // ----------------------------------------

        // ----------------------------------------
        // data mining location.
        // ----------------------------------------
        string mining_path = cvsdata + "/" + root + "/db/mining2";

        // ----------------------------------------
        // printing # of commits to a file 
        // mining.count
        // transactions are in commit_symbols
        // ----------------------------------------
        ofstream out((mining_path + ".count").c_str());
        out << commit_symbols.size() << endl;
        out.close();
        
        // ----------------------------------------
        // write out the berkeley database
        // mainly the frequency of each symbol and
        // # of times it appeared.
        // ----------------------------------------
        write_DB_database( mining_path + ".db", commit_symbols);

        // ----------------------------------------
        // write out the Xapian database
        // index by comment terms, the info field 
        // should contain all the symbols
        // ----------------------------------------
        write_OM_database( mining_path + ".om", commit_symbols, commit_words);

    } catch(const Xapian::Error & error) {
        cerr << "Xapian Exception: " << error.get_msg() << endl;
    } catch( DbException& e ) {
        cerr << "Sleepy Cat Exception:  " << e.what() << endl;
    }
}

void
usage(char * prog_name)
{
  cerr << "Usage: " << prog_name << " [Options] ..." << endl
       << endl
       << "Options:" << endl
       << "  -h                     print out this message" << endl
    ;
  exit(0);
}

void write_DB_database( const string & database_file,
                        const map<unsigned int, set<string> > & commit_symbols)
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
    map<unsigned int, set<string> >::const_iterator t;
    for (t = commit_symbols.begin(); t!= commit_symbols.end(); ++t) {
        const set<string> & symbols = t->second;
        set<string>::const_iterator s;
        for (s = symbols.begin(); s != symbols.end(); ++s) {
            ++(item_count[*s]);
        }
    }

    // ----------------------------------------
    // write to a berkeley db, each symbol 
    // and # of times it has appeared.
    // ----------------------------------------
    system( ("rm -rf " + database_file ).c_str() );
    cerr << "... writing out item counts" << endl;
    
    Db db(0,0);
    db.open(database_file.c_str(), 0, DB_HASH, DB_CREATE, 0);

    map<string, unsigned int>::const_iterator i;
    for(i = item_count.begin(); i != item_count.end(); ++i) {
        string item = i->first;
        string count = convert(i->second);
        
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

void write_OM_database( const string & database_dir,
                        const map<unsigned int, set <string> > & commit_symbols, 
                        const map<unsigned int, list<string> > & commit_words ) {
    Xapian::WritableDatabase database(Xapian::Quartz::open(database_dir, Xapian::DB_CREATE_OR_OVERWRITE)); // open database 
    
    assert(commit_words.size() == commit_symbols.size());

    map<unsigned int, set <string> >::const_iterator i;
    for (i = commit_symbols.begin(); i != commit_symbols.end(); ++i)
    {
        const set<string> & symbols = i->second;
        string symbol_string;
        set<string>::iterator j;
        for (j = symbols.begin(); j != symbols.end(); ++j) {
            symbol_string += (*j) + " ";
        }
        
        map<unsigned int, list<string> >::const_iterator f = commit_words.find(i->first);
        if (f == commit_words.end()) {
            continue;
        }

        const list<string> & words = f->second;
        
        Xapian::Document newdocument;
        
        // ----------------------------------------
        // add terms for indexing
        // ----------------------------------------
        set<string> added;
        list<string>::const_iterator w;
        for (w = words.begin(); w != words.end(); ++w) {
            if ( added.find(*w) != added.end() ) {
                continue; // added already, save some space by skipping
            }
            newdocument.add_term_nopos(*w); 
            added.insert(*w);
        }
        
        // ----------------------------------------
        // add symbols for indexing (symbols get a 
        // : prefix to distinguish them from terms)
        // ----------------------------------------
        for(j = symbols.begin(); j != symbols.end(); ++j) {
            newdocument.add_term_nopos(":"+(*j)); 
        }
        
        // ----------------------------------------
        // put transaction contents in data
        // ----------------------------------------
        newdocument.set_data(  symbol_string );
        
        database.add_document(newdocument);
    }
}
