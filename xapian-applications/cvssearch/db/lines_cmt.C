#include "lines_cmt.h"
#include "util.h"
//
// loads contents of offset file into file and offset vectors
//
void
lines_cmt::load_offset_file(  const string& file_offset, vector<string>& files, vector<string>& offsets ) {
  
    cerr << "... reading " << file_offset << endl;
  
    ifstream in(file_offset.c_str());
    assert(in);
  
    string file;
    string offset;
    while ( in >> file ) {
        in >> offset;
        files.push_back( file ); 
        offsets.push_back( offset ); 
    }
    in.close();
    
}

//
// constructor 
//
//    * loads offset file info into vectors files and offsets
//
//    * in_comment is the stream for reading from cmt file
//
lines_cmt::lines_cmt( const string& root,        // e.g. root0, root1, local storage directory for a repository
                      const string& pkg,         // package 
                      const string& file_db,     // name of cmt file
                      const string& file_offset, // name of offset file 
                      const string& mes          // message used for progress indicator
    )
    :lines(root, pkg, mes)
{
    load_offset_file( file_offset, files, offsets );
    in_comments = new ifstream(file_db.c_str());
    current_offset = 1;
}

lines_cmt::~lines_cmt() {
    delete in_comments;
}

// ----------------------------------------
// given a line and field from a cmt file, 
// read a vector of information for that 
// field in that line
//
// example:  if a line is involved with 3
// commits and the field is "author",
//           then returns a vector of length
// 3 containing the 3 authors in order
// ----------------------------------------
void
lines_cmt::readVector( const string& line, const string& field, vector<string>& field_vector ) {
    // pick up revisions
    int i = -1;
    for(;;) {
        i = line.find( "\003"+field +" ", i+1);
        if ( i == -1 ) {
            break;
        }
        int j = line.find("\003", i+1)-1;
        string item =  string(line, i, j-i+1);
        //    cerr << "found item -" << item << "-" << endl;
        string val = string(item, field.length()+2);
        //    cerr << "found val -" << val << "-" << endl;
        field_vector.push_back(val);
    }
}


// ----------------------------------------
// reads next line from file; moves on to 
// next file when previous file done
//
// returns false when there are no more
// files to read lines from
// ----------------------------------------
bool
lines_cmt::readNextLine() {

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

        unsigned int offset = atoi(offsets[file_no-1].c_str());
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
    data = uint_to_string(line_no-current_offset + 1) + ':' + root + ' ' + package + ' ' + uint_to_string(file_no) + ':';
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
        codeline = line;
        extractSymbols( line, symbols, symbol_list, false );
        //      cerr << "...extracted # symbols = " << symbols.size() << endl;
    }

    return true;

}
