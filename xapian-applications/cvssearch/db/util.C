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

#define INCLUDE_TERM_STOP_LIST_IN_SYMBOL_STOP_LIST true

#warning "use stemming on symbols"

// http://www.dcs.gla.ac.uk/idom/ir_resources/linguistic_utils/stop_words
const static char *term_stoplist[] = {
  "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", 
  "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
  "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",

  "a",
  "about",
  "above",
  "across",
  "after",
  "afterwards",
  "again",
  "against",
  "all",
  "almost",
  "alone",
  "along",
  "already",
  "also",
  "although",
  "always",
  "am",
  "among",
  "amongst",
  "amoungst",
  "amount",
  "an",
  "and",
  "another",
  "any",
  "anyhow",
  "anyone",
  "anything",
  "anyway",
  "anywhere",
  "are",
  "around",
  "as",
  "at",
  "back",
  "be",
  "became",
  "because",
  "become",
  "becomes",
  "becoming",
  "been",
  "before",
  "beforehand",
  "behind",
  "being",
  "below",
  "beside",
  "besides",
  "between",
  "beyond",
  "bill",
  "both",
  "bottom",
  "but",
  "by",
  "call",
  "can",
  "cannot",
  "cant",
  "co",
  "computer",
  "con",
  "could",
  "couldnt",
  "cry",
  "de",
  "describe",
  "detail",
  "do",
  "done",
  "down",
  "due",
  "during",
  "each",
  "eg",
  "eight",
  "either",
  "eleven",
  "else",
  "elsewhere",
  "empty",
  "enough",
  "etc",
  "even",
  "ever",
  "every",
  "everyone",
  "everything",
  "everywhere",
  "except",
  "few",
  "fifteen",
  "fify",
  "fill",
  "find",
  "fire",
  "first",
  "five",
  "for",
  "former",
  "formerly",
  "forty",
  "found",
  "four",
  "from",
  "front",
  "full",
  "further",
  "get",
  "give",
  "go",
  "had",
  "has",
  "hasnt",
  "have",
  "he",
  "hence",
  "her",
  "here",
  "hereafter",
  "hereby",
  "herein",
  "hereupon",
  "hers",
  "herself",
  "him",
  "himself",
  "his",
  "how",
  "however",
  "hundred",
  "i",
  "ie",
  "if",
  "in",
  "inc",
  "indeed",
  "interest",
  "into",
  "is",
  "it",
  "its",
  "itself",
  "keep",
  "last",
  "latter",
  "latterly",
  "least",
  "less",
  "ltd",
  "made",
  "many",
  "may",
  "me",
  "meanwhile",
  "might",
  "mill",
  "mine",
  "more",
  "moreover",
  "most",
  "mostly",
  "move",
  "much",
  "must",
  "my",
  "myself",
  "name",
  "namely",
  "neither",
  "never",
  "nevertheless",
  "next",
  "nine",
  "no",
  "nobody",
  "none",
  "noone",
  "nor",
  "not",
  "nothing",
  "now",
  "nowhere",
  "of",
  "off",
  "often",
  "on",
  "once",
  "one",
  "only",
  "onto",
  "or",
  "other",
  "others",
  "otherwise",
  "our",
  "ours",
  "ourselves",
  "out",
  "over",
  "own",
  "part",
  "per",
  "perhaps",
  "please",
  "put",
  "rather",
  "re",
  "same",
  "see",
  "seem",
  "seemed",
  "seeming",
  "seems",
  "serious",
  "several",
  "she",
  "should",
  "show",
  "side",
  "since",
  "sincere",
  "six",
  "sixty",
  "so",
  "some",
  "somehow",
  "someone",
  "something",
  "sometime",
  "sometimes",
  "somewhere",
  "still",
  "such",
  "system",
  "take",
  "ten",
  "than",
  "that",
  "the",
  "their",
  "them",
  "themselves",
  "then",
  "thence",
  "there",
  "thereafter",
  "thereby",
  "therefore",
  "therein",
  "thereupon",
  "these",
  "they",
  "thick",
  "thin",
  "third",
  "this",
  "those",
  "though",
  "three",
  "through",
  "throughout",
  "thru",
  "thus",
  "to",
  "together",
  "too",
  "top",
  "toward",
  "towards",
  "twelve",
  "twenty",
  "two",
  "un",
  "under",
  "until",
  "up",
  "upon",
  "us",
  "very",
  "via",
  "was",
  "we",
  "well",
  "were",
  "what",
  "whatever",
  "when",
  "whence",
  "whenever",
  "where",
  "whereafter",
  "whereas",
  "whereby",
  "wherein",
  "whereupon",
  "wherever",
  "whether",
  "which",
  "while",
  "whither",
  "who",
  "whoever",
  "whole",
  "whom",
  "whose",
  "why",
  "will",
  "with",
  "within",
  "without",
  "would",
  "yet",
  "you",
  "your",
  "yours",
  "yourself",
  "yourselves"
};

// C++ keywords for now (uses () for functions?)
// www.cs.pdx.edu/~annieg/cs145/handouts/reservedwords.html 
// probably not necessary
const static char *symbol_stoplist[] = {
  "include", 
  "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", 
  "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
  "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
  "asm",
  "auto",
  "bool",
  "break",
  "case",
  "catch",
  "char",
  "class",
  "const",
  "continue",
  "default",
  "delete",
  "do",
  "double",
  "else",
  "enum",
  "extern",
  "false",
  "float",
  "for",
  "friend",
  "goto",
  "if",
  "inline",
  "int",
  "long",
  "mutable",
  "new",
  "operator",
  "private",
  "protected",
  "public",
  "register",
  "return",
  "short",
  "signed",
  "sizeof",
  "static",
  "struct",
  "switch",
  "template",
  "this",
  "throw",
  "true",
  "try",
  "typedef",
  "union",
  "unsigned",
  "virtual",
  "void",
  "volatile",
  "while"
};

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
	  if ( symbolStopWords.find(current) == symbolStopWords.end() ) {
	    current += "()";
	    //cerr << "... found " << current << endl;
	    symbols.insert(current);
	  }
	  current = "";
	  foundBlank = false;
	} else {
	  // identifier ended
	  //cerr << "... found " << current << endl;
	  assert( current != "" );
	  if ( symbolStopWords.find(current) == symbolStopWords.end() ) {
	    symbols.insert(current);
	  }
	  current = "";
	  foundBlank = false;
	}
      } else { // okay subsequent character
	if ( foundBlank ) {
	  assert( current != "" );
	  if ( symbolStopWords.find(current) == symbolStopWords.end() ) {
	    symbols.insert(current);
	  }
	  current = "";	  
	  foundBlank = false;
	}
	current += c;
      }
    }
  } 
  if ( current != "" ) {
    //    cerr << "...found " << current << endl;
    if ( symbolStopWords.find(current) == symbolStopWords.end() ) {
      symbols.insert(current);
    }
  }
}

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

void Lines::stemWords( const list<string>& words, list<string>& term_list ) {
    for( list<string>::const_iterator i = words.begin(); i != words.end(); i++ ) {
     
      string word = *i;
    
      om_termname term = word;
      lowercase_term(term);
      term = stemmer->stem_word(term);
      
      if ( termStopWords.find(term) == termStopWords.end() ) {
	//      cerr << "inserting word " << term << endl;
	terms.insert(term);
	term_list.push_back(term);
      }
    }
}

#warning "we should be able to specify granularity here:  line/file/app"
#warning "perhaps we should rename this class"
#warning "the default should work on a line by line basis"
Lines::Lines( const string& p, const string& sroot, const string& pkg, const string& file_db, const string& file_offset, const string& gran, bool use_stop_words ) {
  
  granularity = gran;

  cerr << "** granularity " << granularity << endl;

  path = p;
  root = sroot;
  package = pkg;
  
  load_offset_file( file_offset, files, offsets );
  
  in_comments = new ifstream(file_db.c_str());
  
  stemmer = new OmStem("english");
  
  line_no = 0;
  current_fn = "";
  current_offset = 1;
  
  file_count = 0;

  if ( use_stop_words ) {
  
    for ( unsigned int i = 0; i < sizeof(term_stoplist)/sizeof(char*); i++ ) {
      string word = term_stoplist[i];
      lowercase_term(word);
      word = stemmer->stem_word(word);
      cerr << "adding " << word << endl;
      termStopWords.insert( word );

#if  INCLUDE_TERM_STOP_LIST_IN_SYMBOL_STOP_LIST
      symbolStopWords.insert( word );
#endif
    }
    
    for ( unsigned int i = 0; i < sizeof(symbol_stoplist)/sizeof(char*); i++ ) {
      string word = symbol_stoplist[i];
      lowercase_term(word);
      cerr << "adding " << word << endl;
      symbolStopWords.insert( word );
    }

  } else {
    cerr << "[ no stop words ]" << endl;
  }
  
  in_code = 0;
  
}

Lines::~Lines() {
  delete in_comments;
  delete stemmer;
  if ( in_code != 0 ) {
    delete in_code;
  }
}


string Lines::currentFile() {
  assert( current_fn != "" );
  return current_fn;
}

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

string Lines::getData() {
  return data;
}

void Lines::updateRevisionComments( map< string, list<string> >& rcw ) {
  for( map< string, list<string > >::iterator i = revision_comment_words.begin(); i != revision_comment_words.end(); i++ ) {
    if ( rcw[i->first].empty() ) {
      rcw[ i->first ] = i->second;
    } else {
      /**
#warning "should take out this assert"
      if ( rcw[i->first] != i->second ) {
	cerr << "Revision " << i->first << endl;
	cerr << "rcw size " << rcw[i->first].size() << endl;
	cerr << "new size " << (i->second).size() << endl;
	assert(0);
      }
      **/
    }
  }
}

// returns false when there is no next line
bool Lines::ReadNextLine() {

  bool changedFiles = false;

  revision_comment_words.clear();

  do {

    if ( granularity == "line" ) {
      terms.clear();
      term_list.clear();
      symbols.clear();
      data = "";
    }
    
    // if in file mode, this call returns okay, but the one after it should
    // fail
    if ( in_comments->eof() ) {
      goto eof_found;
    }
  
    line_no++;
    
    string line;
    string l;
    for(;;) {
      if ( getline( *in_comments, l, '\n' ).eof() ) {
	assert( l == "" );
	goto eof_found;
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

    for( int i = 0; i < comments.size(); i++ ) {
      combined_comments += (" "+comments[i]);

      list<string> words;
      split( comments[i], " .,:;#%_*+&'\"/!()[]{}<>?-\t\n\002\003", words ); // we get 002 sometimes if ".^B"
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
      cerr << "... processing " << current_fn << endl;

      if ( granularity == "file" ) {
	terms_return = terms;
	symbols_return = symbols;
	term_list_return = term_list;
	terms.clear();
	symbols.clear();
	term_list.clear();
      }

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
	assert( *in_code );
      }
      
    }

    // build data string
    // static char str[4096];
    ostrstream ost;
    ost << (line_no-current_offset + 1) << ":"<< root << " " << package << " " << file_no << ":" << ends;
    // sprintf(str, "%d:%d", (line_no-current_offset+1), file_no );
    data = ost.str(); // string(str) +" " + root + " " + package + ":";
  
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

  } while (!changedFiles && granularity != "line" );

  if ( granularity == "line" ) {
    terms_return = terms;
    symbols_return = symbols;
    term_list_return = term_list;
  } 

  return true;

 eof_found: ;

  if ( granularity == "line" ) {
    return false;
  }

  // a bit tricker in file granularity
  if ( !terms.empty() || !symbols.empty() || !term_list.empty() ) {
    terms_return = terms;
    symbols_return = symbols;
    term_list_return = term_list;
    terms.clear();
    symbols.clear();
    term_list.clear();
    return true;
  }
  return false; // second time files
}

string Lines::getCodeLine() {
  assert( granularity == "line" );
  return code_line;
}

set<string> Lines::getCommentTerms() {
  return terms_return;
}
 
set<string> Lines::getCodeSymbols() {
  assert( path != "" );
  return symbols_return;
}

list<string> Lines::getTermList() {
  return term_list_return;
}

#warning "doesn't handle all upper case yet"
set<string> Lines::getCodeSymbolTerms() {
  // computed here, since may not be required by some apps
  
  set<string> code_terms;
 
  for( set<string>::iterator s = symbols_return.begin(); s != symbols_return.end(); s++ ) {
 
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

void readTags( const string& fn, set<string>& S ) {
  cerr << "readTags " << fn << endl;
  ifstream in(fn.c_str());
  assert (in);
  string s;
  while ( getline( in, s ) ) {
    if ( s == "" || s[0] == '!' ) {
      continue;
    }
    //    cerr << "FOUND -" << s << "-" << endl;
    bool function = (s.find("\tfunction\t") != -1) || (s.find("\tfunction")+string("\tfunction").length() == s.length()) || (s.find("\tmethod\t") != -1);
    string symbol = s.substr( 0, s.find("\t") );
    if ( symbol.find("::") != -1 ) {
      symbol = symbol.substr( symbol.find("::")+2 );
    }

    // skip it if still has ::
    if ( symbol.find("::") != -1 ) {
      continue;
    }

    if ( function ) {
      symbol += "()";
    }
    S.insert(symbol);
    //    cerr << "** found symbol -" << symbol << "-" << endl;
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
