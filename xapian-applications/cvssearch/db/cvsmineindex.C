/********************************************************************************
 * cvsmineindex.C
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
 *                        $CVSDATA/root0/db/mining.om with quartz database
 *                         inside
 *           => builds a directory
 *                        $CVSDATA/root0/db/mining.db with berkerley db 
 *           => builds a file
 *                        $CVSDATA/root0/db/mining.count with # of commits
 * 
 ********************************************************************************/


// ??????????? is this info still correct below?
//     Generates omsee databases each page.
//
//     If library directories given, also generates a "mining" omsee database
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
#warning "requires omsee 0.4.1"


#include <unistd.h>
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


// support C/C++/Java for now
// ctags 5.0 flags (see http://ctags.sourceforge.net/ctags.html)
const string CTAGS_OUTPUT = "/tmp/tags";
const string CTAGS_FLAGS = "-R -n --file-scope=no --fields=aiKs --c-types=cfsu --java-types=cim -f" + CTAGS_OUTPUT;

// ----------------------------------------
// function declarations.
// ----------------------------------------
static void usage(char * prog_name);
static void write_OM_database( const string & database_dir,
                               const map<unsigned int, set <string> > & commit_symbols,
                               const map<unsigned int, set<string> > & commit_words
                               );

static void write_DB_database( const string & database_file,
                               const map<unsigned int, set<string> > & commit_symbols
                               );

int main(unsigned int argc, char *argv[]) {

    string cvsdata = get_cvsdata();
    string root = "";
    set<string> packages;
    bool read_library = false;
    system( ("rm -f " + CTAGS_OUTPUT).c_str() );
    for (unsigned int i = 1; i < argc; ++i) {
        if (0) {
        } else if (!strcmp(argv[i],"-f") && i+1 < argc) {
            ifstream fin(argv[++i]);
            get_packages(fin, packages);
        } else if (!strcmp(argv[i],"-r") && i+1 < argc) {
            root = argv[++i];
        } else if (!strcmp(argv[i],"-h")) {
            usage(argv[0]);
        } else {
            // ----------------------------------------
            // get libraries if any from cmd line
            // and run ctags on it.
            // ----------------------------------------
            string dir = argv[i];
            cerr << "... running ctags on library " << dir << endl;
            string cmd = string("ctags -a ") + string(CTAGS_FLAGS) + " " + dir; // append mode
            cerr << "... invoking " << cmd << endl;
            system(cmd.c_str());
            read_library = true;
        }
    }
    if (root.length() == 0) {
        usage(argv[0]);
    }

    // ----------------------------------------
    // get symbols from library
    // ----------------------------------------
    set<string> lib_symbols;
    map<string, set<string> > lib_symbol_parents;
    if (read_library) {
        cerr << "... reading library tags" << endl;
        readTags( CTAGS_OUTPUT, lib_symbols, lib_symbol_parents );
        cerr << "... reading library tags done" << endl;
    }

    // ----------------------------------------
    // This is the key:   It takes a commit id
    // (global) to all the symbols under that commit.
    // ----------------------------------------
    // commit_symbols[commit_id] -> code symbols.
    // commit_words  [commit_id] -> stemmed words.

    set<unsigned int> commit_id_set; // set of all commit ids
    map<unsigned int, set <string> > commit_symbols;
    map<unsigned int, set<string> > commit_words;


    unsigned int offset = 0;

    string commit_path = cvsdata + "/" + root + "/commit.offset";
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

            // ----------------------------------------
            // run ctags on that package
            // ----------------------------------------
            system( ("rm -f " + CTAGS_OUTPUT).c_str() );
            string cmd = string("ctags ") + CTAGS_FLAGS + " " + package_src_path;
            cerr << "... invoking " << cmd << endl;
            system(cmd.c_str());

            // ----------------------------------------
            // read symbols from each application
            // ----------------------------------------
            set<string> app_symbols;
            map<string, set<string> > app_symbol_parents;
            cerr << "... reading application tags" << endl;
            readTags( CTAGS_OUTPUT, app_symbols, app_symbol_parents );


            // ------------------------------------------------------------
            // need first input to be
            // "...cvsdata/root0/db/pkg" + ".db/" + pkg.db"
            // ------------------------------------------------------------
            cvs_db_file db_file(package_db_path + ".db/" + package_name + ".db", true);
            lines_db lines(root, package_path, " mining", db_file);
            unsigned int count;

            // ----------------------------------------
            // writing to the offset of each package
            // commits here. (pkg, commit_offset)
            // so later we can do
            // global commit id-> (pkg, local commit id)
            // ----------------------------------------
            if (db_file.get_commit_count(count) == 0) {
                fout << package_path << " " << offset << endl;
                offset += count;
            }

            unsigned int commitid = 0;
            unsigned int fileid = 0;
            string filename = "";
            while ( lines.readNextLine() ) {
                string data = lines.getData();
                set<string> symbols = lines.getCodeSymbols();

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
                    if (db_file.get_commit(fileid, i->first, commitid) == 0) {

                        commit_id_set.insert( commitid+offset ); // set of all commit ids


                        if ( commit_words.find( commitid+offset ) == commit_words.end() ) {
				set<string> empty;
				commit_words[ commitid+offset ] = empty;
                        }
                        if ( commit_symbols.find( commitid+offset ) == commit_symbols.end() ) {
				set<string> empty;
				commit_symbols[ commitid+offset ] = empty;
                        }

                        // ----------------------------------------
                        // have we entered info for this commit ?
                        // ----------------------------------------
                        if (commit_words[commitid+offset].empty())
                        {
                            // ----------------------------------------
                            // nope, we have not. so need to set
                            // commit_words[commitid]
                            // ----------------------------------------
                            const map<string, list<string> > & terms = lines.getRevisionCommentWords();
                            map<string, list<string> >::const_iterator itr = terms.find(i->first);
                            if (itr != terms.end()) {
                                const list<string>& words = itr->second;
                                for( list<string>::const_iterator i = words.begin(); i != words.end(); i++ ) {
                                        commit_words[commitid+offset].insert(*i);
                                }
                            }
                        }
                        // ----------------------------------------
                        // now go through each symbol,
                        // and add it to the commit_symbols mapping
                        // ----------------------------------------
                        for( set<string>::iterator s = symbols.begin(); s != symbols.end(); ++s ) {
                            if ( lib_symbols.find(*s) != lib_symbols.end() ) {
                                commit_symbols[commitid+offset].insert(*s);
                            } else {
                                // ----------------------------------------
                                // this symbol is not in the library, so
                                // let's see if its parents are;
                                // if so, we add every such parent
                                // ----------------------------------------
#warning "doesn't look at parents now"
#if 0 // took it out as it messes up rankings; need special support in ranking function for parents/ancestors
                                set<string> parents = app_symbol_parents[*s];
                                for( set<string>::iterator p = parents.begin(); p != parents.end(); ++p ) {
                                    if ( lib_symbols.find(*p) != lib_symbols.end() ) {
                                        commit_symbols[commitid+offset].insert(*p);
                                    }
                                }
#endif
                            }
                        }
                    }
                }
            }
        } // for packages
        fout.close();

        // ----------------------------------------
        // write results
        // ----------------------------------------

        // ----------------------------------------
        // data mining location.
        // ----------------------------------------
        string mining_path = cvsdata + "/" + root + "/db/mining";

        // ----------------------------------------
        // printing # of commits to a file 
        // mining.count
        // transactions are in commit_symbols
        // ----------------------------------------
        assert( commit_id_set.size() == commit_symbols.size() );
        assert( commit_id_set.size() == commit_words.size() );
        ofstream out((mining_path + ".count").c_str());
        out << commit_id_set.size() << endl; 
        out.close();
        
        // ----------------------------------------
        // write out the berkeley database
        // mainly the frequency of each symbol and
        // # of times it appeared.
        // ----------------------------------------
        write_DB_database( mining_path + ".db", commit_symbols);

        // ----------------------------------------
        // write out the omsee database
        // index by comment terms, the info field 
        // should contain all the symbols
        // ----------------------------------------
        write_OM_database( mining_path + ".om", commit_symbols, commit_words);

    } catch(OmError & error) {
        cerr << "OmSee Exception: " << error.get_msg() << endl;
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
                        const map<unsigned int, set<string> > & commit_words
    )
{
    system( ("rm -rf " + database_dir).c_str() );
    system( ("mkdir " + database_dir).c_str() );

    OmSettings db_parameters;
    db_parameters.set("backend", "quartz");
    db_parameters.set("quartz_dir", database_dir);
    db_parameters.set("database_create", true);
    OmWritableDatabase database(db_parameters); // open database 


    int transactions_written = 0;

    map<unsigned int, set <string> >::const_iterator i;

    // iterate over commits
    for (i = commit_symbols.begin(); i != commit_symbols.end(); ++i)
    {

        // find symbols associated with commit
        const set<string> & symbols = i->second;
        string symbol_string = convert(i->first) + " ";
        set<string>::iterator j;
        for (j = symbols.begin(); j != symbols.end(); ++j) {
            symbol_string += (*j) + " ";
        }

        //cerr << "DATA = " << symbol_string << endl;

        // find comment words associated with that commit
        map<unsigned int, set<string> >::const_iterator f = commit_words.find(i->first);
        set<string> words;
        assert( f != commit_words.end() );
        if ( f->second.empty() ) { // no commit words..., create transaction anyways...
		words.insert("EMPTY"); // won't match anything since upper case & no preceding :
        } else {
		words = f->second;
        }


        OmDocument newdocument;
        int pos = 0;

        // ----------------------------------------
        // add terms for indexing
        // ----------------------------------------
        set<string>::const_iterator w;
        for (w = words.begin(); w != words.end(); ++w) {
            newdocument.add_posting(*w, ++pos);
                //cerr << "... term " << (*w) << endl;
        }

        // ----------------------------------------
        // add symbols for indexing (symbols get a
        // : prefix to distinguish them from terms)
        // ----------------------------------------
        for(j = symbols.begin(); j != symbols.end(); ++j) {
            newdocument.add_posting(":"+(*j), ++pos);
            cerr << "... symbol " << (":"+(*j)) << endl;
        }

        // ----------------------------------------
        // put transaction contents in data
        // ----------------------------------------
        newdocument.set_data(  symbol_string );

        database.add_document(newdocument);
        transactions_written++;
    }
    cerr << "transactions written = " << transactions_written << endl;
}

//             {
//                 lines_db lines1(root, package_path, " mining", db_file);

//                 // ------------------------------------------------------------
//                 // need first input to be
//                 // "...cvsdata/root0/db/pkg" + ".db/" + pkg.db"
//                 // ------------------------------------------------------------

//                 unsigned int commitid = 0;
//                 unsigned int fileid = 0;
//                 string filename = "";
//                 while ( lines1.readNextLine() ) {
//                     string data = lines1.getData();
//                     set<string> symbols = lines1.getCodeSymbols();
                
//                     if (strcmp(filename.c_str(), lines1.getCurrentFile().c_str())) {
//                         filename = lines1.getCurrentFile();
//                         filename = filename.substr(package_name.length() + 1, filename.length() - package_name.length() - 1);
//                         // ----------------------------------------
//                         // need to obtain file id
//                         // only when the file has changed.
//                         // ----------------------------------------
//                         if (db_file.get_fileid(fileid, filename) != 0) {
//                             fileid = 0;
//                         }
//                     }
//                     if (fileid == 0) {
//                         continue;
//                     }
//                     // ----------------------------------------
//                     // here we have a mapping between revision
//                     // for this line and the associated CVS comment.
//                     // ----------------------------------------
//                     const map<string, string > & revisions = lines1.getRevisionCommentString();
//                     map<string, string >::const_iterator i;
                
//                     for(i = revisions.begin(); i != revisions.end(); ++i ) {
//                         if (db_file.get_commit(fileid, i->first, commitid) == 0) {
//                             // ----------------------------------------
//                             // have we entered info for this commit ?
//                             // ----------------------------------------
//                             if (commit_words1.find(commitid+offset) == commit_words1.end()) 
//                             {
//                                 // ----------------------------------------
//                                 // nope, we have not. so need to set
//                                 // commit_words[commitid]
//                                 // ----------------------------------------
//                                 const map<string, list<string> > & terms = lines1.getRevisionCommentWords();
//                                 map<string, list<string> >::const_iterator itr = terms.find(i->first);
//                                 if (itr != terms.end()) {
//                                     commit_words1[commitid+offset] = itr->second;
//                                 }
//                             }
//                             // ----------------------------------------
//                             // now go through each symbol,
//                             // and add it to the commit_symbols mapping
//                             // ----------------------------------------
//                             for( set<string>::iterator s = symbols.begin(); s != symbols.end(); ++s ) {
//                                 if ( lib_symbols.find(*s) != lib_symbols.end() ) {
//                                     commit_symbols1[commitid+offset].insert(*s);
//                                 } else {
//                                 // ----------------------------------------
//                                 // this symbol is not in the library, so 
//                                 // let's see if its parents are;
//                                 // if so, we add every such parent
//                                 // ----------------------------------------
//                                     set<string> parents = app_symbol_parents[*s];
//                                     for( set<string>::iterator p = parents.begin(); p != parents.end(); ++p ) {
//                                         if ( lib_symbols.find(*p) != lib_symbols.end() ) {
//                                             commit_symbols1[commitid+offset].insert(*p);
//                                         }
//                                     }
//                                 }
//                             }
//                         }
//                     }
//                 }
//             }

//         cerr << "commit_symbols1 size" << commit_symbols1.size() << endl;
//         cerr << "commit_symbols  size" << commit_symbols.size() << endl;
//         cerr << "commit_words1   size" << commit_words1.size() << endl;
//         cerr << "commit_words    size" << commit_words.size() << endl;

//         for (map<unsigned int, set<string> >::const_iterator itr = commit_symbols.begin(),
//                  itr1 = commit_symbols1.begin();
//              itr != commit_symbols.end(); ++itr, ++itr1) {
//             cerr << "symbols  size " << (itr->second).size() << endl;
//             cerr << "symbols1 size " << (itr1->second).size() << endl;
//             for(set<string>::const_iterator sitr = (itr->second).begin(),
//                     sitr1 = (itr1->second).begin();
//                 sitr != (itr->second).end();
//                 ++sitr, ++sitr1)
//             {
//                 cerr << "from symbols  " << *sitr << endl;
//                 cerr << "from symbols1 " << *sitr1<< endl;
//                 assert(!strcmp ((*sitr).c_str(), (*sitr1).c_str()));
//             }
//         }

//         for (map<unsigned int, list<string> >::const_iterator itr = commit_words.begin(),
//                  itr1 = commit_words1.begin();
//              itr != commit_words.end(); ++itr, ++itr1) {
//             cerr << "words    size " << (itr->second).size() << endl;
//             cerr << "words1   size " << (itr1->second).size() << endl;
//             for(list<string>::const_iterator sitr = (itr->second).begin(),
//                     sitr1 = (itr1->second).begin();
//                 sitr != (itr->second).end();
//                 ++sitr, ++sitr1)
//             {
//                 cerr << "from words  " << *sitr << endl;
//                 cerr << "from words1 " << *sitr1<< endl;
//                 assert(!strcmp ((*sitr).c_str(), (*sitr1).c_str()));
//             }
//         }
