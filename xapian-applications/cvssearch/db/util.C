// util.C
//
// (c) 2001 Amir Michail (amir@users.sourceforge.net)

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

#include <om/om.h>
#include <fstream.h>
#include <stdio.h>
#include "util.h"
#include <strstream>

#define SHOW_WARNINGS 0 

#if SHOW_WARNINGS
#warning "use stemming on symbols"
#endif

void lowercase_term(om_termname &term)
{
  om_termname::iterator i = term.begin();
  while(i != term.end()) {
    *i = tolower(*i);
    i++;
  }
}


void lowercase_string(string &term)
{
  om_termname::iterator i = term.begin();
  while(i != term.end()) {
    *i = tolower(*i);
    i++;
  }
}

void split (const string & text, const string & separators, list<string> & words)
{
  int n = text.length();
  int start, stop;

  start = text.find_first_not_of(separators);
  while ((start >= 0) && (start < n)) {
    stop = text.find_first_of(separators, start);
    if ((stop < 0) || (stop > n)) {
      stop = n;
    }
    string word = text.substr(start, stop - start);
    words.push_back(word);
    start = text.find_first_not_of(separators, stop+1);
  }
}


bool Lines::blankChar(char c) {
  return ( c == ' ' || c == '\t' );
}

bool Lines::okFirstChar(char c) {
  return ((c >= 'a' && c <='z') || (c >='A' && c <= 'Z' ) || c == '_');
}

bool Lines::okSubChar(char c) {
  return (okFirstChar(c) || (c >= '0' && c <= '9' ));
}

//
// Given a line of source code, pick up all classes and functions in that line
// and insert them into the symbols set.
//
void Lines::extractSymbols( const string& s ) {
  string current = "";
  bool foundBlank = false;
  for ( string::const_iterator i = s.begin(); i != s.end(); i++ ) {
    char c = *i;
    
    if ( blankChar(c) ) {
      if ( current != "" ) {
	foundBlank = true;
      }
      continue;
    }
    
    if ( current == "" ) {
      if ( okFirstChar(c) ) {
	current = c;
      }
    } else {
      // already started something
      if (! okSubChar(c) ) {
	if ( c == '(' ) {
	  assert( current != "" );
	  current += "()";
	  //cerr << "... found " << current << endl;
	  symbols.insert(current);
	  current = "";
	  foundBlank = false;
	} else {
	  // identifier ended
	  //cerr << "... found " << current << endl;
	  assert( current != "" );
	  symbols.insert(current);
	  current = "";
	  foundBlank = false;
	}
      } else { // okay subsequent character
	if ( foundBlank ) {
	  assert( current != "" );
	  symbols.insert(current);
	  current = "";	  
	  foundBlank = false;
	}
	current += c;
      }
    }
  } 
  if ( current != "" ) {
    //    cerr << "...found " << current << endl;
    symbols.insert(current);
  }
}

//
// loads contents of offset file into file and offset vectors
//
void Lines::load_offset_file(  const string& file_offset, vector<string>& files, vector<string>& offsets ) {
  
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

//
// takes a list of words, lower cases and stems each word, and puts the result
// in another list
//
void Lines::stemWords( const list<string>& words, list<string>& term_list ) {
  for( list<string>::const_iterator i = words.begin(); i != words.end(); i++ ) {
     
    string word = *i;
    
    om_termname term = word;
    lowercase_term(term);
    term = stemmer->stem_word(term);
      
    terms.insert(term);
    term_list.push_back(term);
  }
}

//
// constructor 
//
//    * loads offset file info into vectors files and offsets
//
//    * in_comment is the stream for reading from cmt file
//
Lines::Lines( const string& p,           // path (e.g., "cvsdata/root0/src/")
	      const string& sroot,       // ?????????
	      const string& pkg,         // package 
	      const string& file_db,     // name of cmt file
	      const string& file_offset, // name of offset file 
	      const string& mes          // message used for progress indicator
	      ) {
  
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

Lines::~Lines() {
  delete in_comments;
  delete stemmer;
  if ( in_code != 0 ) {
    delete in_code;
  }
}

//
// returns file containing line just read
//
// if no line read yet, then this is the empty string
// 
string Lines::currentFile() {
  assert( current_fn != "" );
  return current_fn;
}

//
// given a line and field from a cmt file, read a vector of 
// information for that field in that line
//
// example:  if a line is involved with 3 commits and the field is "author",
//           then returns a vector of length 3 containing the 3 authors in order
// 
void Lines::readVector( const string& line, const string& field, vector<string>& field_vector ) {
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

//
// this is the data string returned by the cvssearch command
//
// it looks something like this:
//
//  65 1074:root0 kdenetwork_kmail 63:1.71 1.52 1.42 1.41 1.34      
//
string Lines::getData() {
  return data;
}

//
// This is the data string that may be used in the future to do grep
// searches by using a single file per application
//
// It looks a lot like data from previous member but also includes 
// the line content afterwards.  The idea is to just grep it and 
// look at the data preceding each line in the grep results.
//
string Lines::getCodeLineData() {
  return codelinedata;
}

//
//
// Suppose a line is associated with revision comments C1, C2, C3 for revisions R1, R2, R3.
//
// then the map is updated as follows:
//
// R1 -> list of lowercased, stemmed C1 words
// R2 -> list of lowercased, stemmed C2 words
// R3 -> list of lowercased, stemmed C3 words
//
// Observe that the map is not cleared.  That is, calling this method
// multiple times just keeps making the map bigger (so that we can build a map for all revisions of interest)
//
//
void Lines::updateRevisionComments( map< string, list<string> >& rcw ) {
  for( map< string, list<string > >::iterator i = revision_comment_words.begin(); i != revision_comment_words.end(); i++ ) {
    if ( rcw[i->first].empty() ) {
      rcw[ i->first ] = i->second;
    }
  }
}

//
// reads next line from file; moves on to next file when previous file done
//
// returns false when there are no more files to read lines from
//
bool Lines::ReadNextLine() {

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

//
// Gets number of line just read from current file.
//
int Lines::getLineNumber() {
  return line_no-current_offset+1;
}

//
// Gets the actual contents of the line just read.
// 
string Lines::getCodeLine() {
  return code_line;
}

//
// Lowercased, stemmed terms from all comments associated with line just read.
//
set<string> Lines::getCommentTerms() {
  return terms;
}
 
//
// Code symbols (functions/classes) from line of code just read.
//
set<string> Lines::getCodeSymbols() {
  assert( path != "" );
  return symbols;
}

//
// Lowercased, stemmed term *list* from all comments associated associated with line just read.
//
// Observe that this list preserves word order and frequency.
//
list<string> Lines::getTermList() {
  return term_list;
}

//
// Suppose a line is associated with revision comments C1, C2, C3 for revisions R1, R2, R3.
//
// We return a map of the form:
//
// R1 -> list of lowercased, stemmed C1 words
// R2 -> list of lowercased, stemmed C2 words
// R3 -> list of lowercased, stemmed C3 words
//
// Observe this contains information for the line just read only.
//
map< string, list<string> > Lines::getRevisionCommentWords() { 
  return revision_comment_words;
}

//
// Like above, but we have a map of the form:
//
// R1 -> C1 string
// R2 -> C2 string
// R3 -> C3 string
//
map< string, string > Lines::getRevisionCommentString() { 
  return revision_comment_string;
}

#if SHOW_WARNINGS
#warning "doesn't handle all upper case yet"
#endif

//
// Looks at all the functions/classes associated with the line
// just read, and returns the set of all words in all these functions/classes.
//
// For example, if the symbols are:  startTimer() and TimerEvent, it returns
// the set { start, timer, event }.
//
set<string> Lines::getCodeSymbolTerms() {
  // computed here, since may not be required by some apps
  
  set<string> code_terms;
 
  for( set<string>::iterator s = symbols.begin(); s != symbols.end(); s++ ) {
 
    string w = "";
    for( string::const_iterator c = s->begin(); c != s->end(); c++ ) {
    
      if ( (*c) == '(' || (*c) == ')' ) {
	continue;
      }

      if ( ((*c) >= 'A' && (*c) <= 'Z') || (*c) == '_' ) {
	if ( w != "" ) {
	  lowercase_string(w);
	  w = stemmer->stem_word(w);
	  //		  cerr << "........inserting " << w << endl;
	  code_terms.insert(w);
	  w = "";
	}
      }
      if ( (*c) != '_' ) {
	w += (*c);
      }
    
    }
    if ( w != "" ) {
      lowercase_string(w);
      w = stemmer->stem_word(w);
      //	      cerr << "........inserting " << w << endl;
      code_terms.insert(w);
    }
  
  }

  return code_terms;
}

//
// reads information from ctags file
//
// takes file name of tag file
//
// returns the set of classes/functions mentioned in the file in S
//
// also returns a map which contains an entry for each class with parents;
// in that case, the map takes the class and returns its parents.
//
void readTags( const string& fn, set<string>& S, map<string, set<string> >& symbol_parents ) {
  cerr << "readTags " << fn << endl;
  ifstream in(fn.c_str());
  assert (in);
  string s;
  while ( getline( in, s ) ) {
    if ( s == "" || s[0] == '!' ) {
      continue;
    }
    //    cerr << "FOUND -" << s << "-" << endl;
    bool function =

      (s.find("\tfunction\t") != string::npos) ||
      (s.find("\tfunction")+string("\tfunction").length() == s.length()) ||

      (s.find("\tmethod\t") != string::npos ) ||
      (s.find("\tmethod")+string("\tmethod").length() == s.length()) ||

      ( s.find("\tmember\t") != string::npos ) ||
      (s.find("\tmember")+string("\tmember").length() == s.length() );

#warning "should we look for member not method?"

    string symbol = s.substr( 0, s.find("\t") );
    //    string osymbol = symbol;

    if ( symbol.find("::") != string::npos ) {
      symbol = symbol.substr( symbol.find("::")+2 );
    }

    // skip it if still has ::
    if ( symbol.find("::") != string::npos ) {
      continue;
    }

    if ( function ) {
      symbol += "()";
    } else {

      // this is a class

      int k1 = s.find("\tinherits:");
      if ( k1 != -1 ) {
	k1 += 10;
	int k2 = s.length()-1;
	for( int i = k1; i <= k2; i++ ) {
	  if ( s[i] == '\t' ) {
	    k2 = i-1;
	    break;
	  }
	}
	string parent_string = s.substr( k1, k2-k1+1 );
	//	cerr << symbol << " has parent string -" << parent_string << "-" << endl;
	list<string> parents;
	split( parent_string, ",", parents );
	for( list<string>::iterator i = parents.begin(); i != parents.end(); i++ ) {
	  if ( i->find("::") == -1 ) {
	    //	    cerr << "..." << (*i) << endl;
	    symbol_parents[symbol].insert(*i);
	  }
	}
      }
      
    }
    S.insert(symbol);
    //      cerr << "** found symbol -" << symbol << "-" << endl;
  }
  in.close();
}

string get_cvsdata() 
{
  string cvsdata;
  char *s = getenv("CVSDATA");
  if ( s==0 ) {
    cerr <<" Warning:  $CVSDATA not set!" << endl;
    exit(1);
  } else {
    cvsdata = s;
    // ----------------------------------------
    // strip trailing / if any
    // ----------------------------------------
    if ( cvsdata[cvsdata.length()-1] == '/' ) {
      cvsdata = cvsdata.substr( 0, cvsdata.length()-1 );
    }
  }
  return cvsdata;
}
