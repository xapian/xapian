// lines_db.C
//
// (c) 2001 Andrew Yao (andrewy@users.sourceforge.net)

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

#include <iostream>
#include <unistd.h>
using namespace std;

#include "lines_db.h"
#include "util.h"

#include "pstream.h"

lines_db::lines_db(const string & root, 
                   const string & pkg, 
                   const string & mes,
                   cvs_db_file  & db_file 
    ) 
    :lines(root, pkg, mes),
     _db_file(db_file)
{
    file_id = 0;
    file_length = 0;
    current_fn = "";
}

lines_db::~lines_db() {
}

// returns false when there is no next line
bool lines_db::readNextLine() {

    revision_comment_words.clear();
    revision_comment_string.clear();

    terms.clear();
    term_list.clear();
    symbols.clear();
    symbol_list.clear();
    qualified_classes.clear();
    qualified_class_list.clear();
    data = "";
    codelinedata = "";
    
    set<string, cvs_revision_less> revisions;
    if (0) {
    } else if (line_no < file_length) {
        // ----------------------------------------
        // try to get the revision info for 
        // the next line.
        // ----------------------------------------
        _db_file.get_revision(file_id, ++line_no, revisions);
    } else if (_db_file.get_filename(++file_id, current_fn) == 0) {
        if ( in_code != 0 ) {
            delete in_code;
        }
        current_fn = package + "/" + current_fn;
        string file_path = path + "/" + current_fn;
        in_code = new ifstream(file_path.c_str());
        if (! *in_code) {
            cerr << "*** could not open " << file_path << endl;
            assert(0);
        }

	// in_code->rdbuf()->setbuf(0, 0);
	{
	    vector<string> args;
	    args.push_back("-l");
	    args.push_back(file_path);
	    redi::ipstream wc("wc", args);
	    char buf[64];
	    wc.get(buf, sizeof(buf));
	    file_length = strtoul(buf, NULL, 10);
	}

        cerr << "..." << message << " " << current_fn << endl;
        _db_file.get_revision(file_id, line_no = 1, revisions);
    } else {
        return false;
    }

    string combined_comments;
    for(set<string, cvs_revision_less>::const_iterator itr = revisions.begin();
        itr != revisions.end(); ++itr)
    {
        string comment;
        if (_db_file.get_comment(file_id, *itr, comment) == 0) 
        {
            list<string> words;
	    // leaves apostraphe in
            split( comment, " .,:;#%_*+&\"/!()[]{}<>?-\t\n\002\003", words ); // we get 002 sometimes if ".^B"
            combined_comments += (" " + comment);
            revision_comment_string[*itr] = comment;

            list<string> term_list;
            stemWords( words, term_list );
            revision_comment_words[*itr] = term_list;
        }
    }

    list<string> words;
    // leaves apostraphe in
    split( combined_comments, " .,:;#%_*+&\"/!()[]{}<>?-\t\n\002\003", words ); // we get 002 sometimes if ".^B"
    stemWords( words, term_list );
        
    // build data string
    data = uint_to_string(line_no) + ':' + root + ' ' + package + ' ' + uint_to_string(file_id) + ':';
    codelinedata = data;

    for(set<string, cvs_revision_less>::const_iterator itr = revisions.begin();
        itr != revisions.end(); ++itr)
    {
        if (itr != revisions.begin()) {
            data += " ";
        }
        data += *itr;
    }
  
    if (in_code && *in_code)
    {
        string line = "";
        getline( *in_code, line, '\n' );
        codeline = line;
        extractSymbols( line, symbols, symbol_list, false );
	extractSymbols( line, qualified_classes, qualified_class_list, true );
    }
    return true;
}

