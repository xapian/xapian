// lines_db.C
//
// (c) 2001 Andrew Yao (andrewy@users.sourceforge.net)

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.


#include <fstream>
#include <strstream>
#include "util.h"
#include <om/om.h>

void lines_db::load_offset_file(  const string& file_offset, vector<string>& files, vector<string>& offsets ) {
    cerr << "... reading " << file_offset << endl;
    
    ifstream in(file_offset.c_str());
    assert(in);
    
    while (!in.eof()) {
        string file;
        string offset;
        in >> file;
        if ( file == "" ) {
            break;
        }
        in >> offset;
        files.push_back( file ); 
        offsets.push_back( offset ); 
    }
    in.close();
}

lines_db::lines_db( const string& p, const string& sroot, const string& pkg, const string& file_db, const string& file_offset, const string& mes ) {
  
  path = p;
  root = sroot;
  package = pkg;
  message = mes;
  
  
  load_offset_file( file_offset, files, offsets );
  
  in_comments = new ifstream(file_db.c_str());
  
  stemmer = new OmStem("english");
  
  line_no = 0;
  current_fn = "";
  current_offset = 1;
  
  file_count = 0;

  in_code = 0;
  
}

lines_db::~lines_db() {
    delete in_comments;
    delete stemmer;
    if ( in_code != 0 ) {
        delete in_code;
    }
}

void lines_db::updateRevisionComments( map< string, list<string> >& rcw ) {
  for( map< string, list<string > >::iterator i = revision_comment_words.begin(); i != revision_comment_words.end(); i++ ) {
      if ( rcw[i->first].empty() ) {
          rcw[ i->first ] = i->second;
      }
  }
}

// returns false when there is no next line
bool lines_db::ReadNextLine() {

    bool changedFiles = false;

    revision_comment_words.clear();
    revision_comment_string.clear();

    terms.clear();
    term_list.clear();
    symbols.clear();
    data = "";
    codelinedata = "";
    
    assert( !in_comments->eof() );
    
    line_no++;
    
    string line;
    string l;
    for(;;) {
        if ( getline( *in_comments, l, '\n' ).eof() ) {
            assert( l == "" );
            return false;
        }
        
        if ( line != "" ) {
            line += "\n";
        }
        line += l;
        if ( l.length() >=2 && l[l.length()-2] == '\003' && l[l.length()-1] == '\002' ) {
            break;
        }
    }
    
    //cerr << line << endl;
    
    // the line contains this information:
    
    // file #
    // followed by a list of:
    //  ^Crevision ...
    //  ^Cdate ...
    //  ^Cauthor...
    //  ^Clines...
    //  ^Ccomments...
    
    int file_no = 0;
    vector<string> revisions;
    vector<string> dates;
    vector<string> authors;
    vector<string> lines;
    vector<string> comments;
    string combined_comments;

    string idstr = string( line, 0, line.find(" ") );
    file_no = atoi(idstr.c_str());
    
    readVector(line, "revision", revisions);
    readVector(line, "date", dates);
    readVector(line, "author", authors);
    readVector(line, "lines", lines);
    readVector(line, "comments", comments);

    assert( revisions.size() == dates.size() );
    assert( revisions.size() == authors.size() );
    assert( revisions.size() == lines.size() );
    assert( revisions.size() == comments.size() );

    for(unsigned int i = 0; i < comments.size(); i++ ) {
        combined_comments += (" "+comments[i]);

        list<string> words;
        split( comments[i], " .,:;#%_*+&'\"/!()[]{}<>?-\t\n\002\003", words ); // we get 002 sometimes if ".^B"

        revision_comment_string[ revisions[i] ] = comments[i];

        list<string> term_list;
        stemWords( words, term_list );
        revision_comment_words[ revisions[i] ] = term_list;
    }


    // break up line into words
  
    list<string> words;
    split( combined_comments, " .,:;#%_*+&'\"/!()[]{}<>?-\t\n\002\003", words ); // we get 002 sometimes if ".^B"
  
    string fn = files[file_no-1];
    if ( fn != current_fn ) {
        changedFiles = true;


        file_count++;

        current_fn = fn;
        cerr << "..." << message << " " << current_fn << endl;

        int offset = atoi(offsets[file_no-1].c_str());
        if( line_no != offset ) {
            cerr << "found line_no = " << line_no << endl;
            cerr << "found offset = " << offset << endl;
            assert( line_no == offset );
        }
        current_offset = offset;

        if ( in_code != 0 ) {
            delete in_code;
        }
        if ( path != "" ) {
            //	cerr << "Opening " << path << "/" << current_fn << endl;
            in_code = new ifstream( (path + "/"  + current_fn).c_str() );
            if( ! *in_code ) {
                cerr << "** could not open " << path << "/" << current_fn << endl;
                assert(0);
            }
        }
      
    }

    // build data string
    ostrstream ost;
    ost << (line_no-current_offset + 1) << ":"<< root << " " << package << " " << file_no << ":" << ends;
    data = ost.str(); 
    ost.freeze(0);
    codelinedata = data;
  
    for(int i = revisions.size()-1; i >=0; i-- ) {
        data += revisions[i];
        if ( i > 0 ) {
            data += " ";
        }
    }
    //cerr << "data = -" << data << "-" << endl;
  
    stemWords( words, term_list );
  
  
  
    if ( path != "" ) {
        //////////////// now read code symbols
        line = "";
        getline( *in_code, line, '\n' );
        //      cerr <<"Just read -" << line << "-" << endl;
        code_line = line;
        extractSymbols( line );
        //      cerr << "...extracted # symbols = " << symbols.size() << endl;
    }

    return true;

}

