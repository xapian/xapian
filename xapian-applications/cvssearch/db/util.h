// util.h
//
// (c) 2001 Amir Michail (amir@users.sourceforge.net)

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

#include <list>
#include <map>

extern void lowercase_term(om_termname &term);
extern void lowercase_string(string &term);
extern void split (const string & text, const string & separators, list<string> & words);

#include <fstream.h>
#include <vector>

class Lines {
  // this map contains comment words for each revision for last line read
  // R1 -> list of lowercased, stemmed C1 words
  // R2 -> list of lowercased, stemmed C2 words
  // R3 -> list of lowercased, stemmed C3 words
  // etc.
  map< string, list<string> > revision_comment_words;

  //
  // Like above, but we have a map of the form:
  //
  // R1 -> C1 string
  // R2 -> C2 string
  // R3 -> C3 string
  //
  map< string, string > revision_comment_string;
  
  ifstream *in_comments; // cmt file stream

  ifstream *in_code; // source code file stream

  OmStem *stemmer; // omsee stemmer

  //
  // Lowercased, stemmed terms from all comments associated with line just read.
  //
  set<string> terms;

  //
  // Lowercased, stemmed term *list* from all comments associated associated with line just read.
  //
  // Observe that this list preserves word order and frequency.
  //
  list<string> term_list;

  //
  // Code symbols (functions/classes) from line of code just read.
  //
  set<string> symbols;
  
  //
  // Gets the actual contents of the line just read.
  // 
  string code_line;

  // # lines read in total across all files up to this point
  int line_no;

  // file containing line just read
  //
  // if no line read yet, then this is the empty string
  // 
  string current_fn;
  
  // path (e.g., "cvsdata/root0/src
  string path;

  // line # in cmt file in which comments start to appear for this file
  int current_offset;
  
  //
  // this is the data string returned by the cvssearch command
  //
  // it looks something like this:
  //
  //  65 1074:root0 kdenetwork_kmail 63:1.71 1.52 1.42 1.41 1.34      
  //
  string data;

  //
  // This is the data string that may be used in the future to do grep
  // searches by using a single file per application
  //
  // It looks a lot like data from previous member but also includes 
  // the line content afterwards.  The idea is to just grep it and 
  // look at the data preceding each line in the grep results.
  //
  string codelinedata;

  // information from the offset file is stored in these two vectors
  vector<string> files;
  vector<string> offsets;

  string root; // ??
  
  string package; // package to process

  string message; // message used for progress indicator (e.g., indexing, mining, etc.)


  bool blankChar(char c);
  bool okFirstChar(char c);
  bool okSubChar(char c);
  void extractSymbols( const string& s );
  void load_offset_file(  const string& file_offset, vector<string>& files, vector<string>& offsets );
  void Lines::readVector( const string& line, const string& field, vector<string>& field_vector );

    
  void stemWords( const list<string>& words, list<string>& term_list );


 public:
  Lines( const string& p, const string& root, const string& package, const string& file_db, const string& file_offset, const string&  mes );
  ~Lines();

  void updateRevisionComments( map< string, list<string> >& revision_comment_words );

  string currentFile();
  int getLineNumber();
  // returns false when there is no next line
  bool ReadNextLine();
  set<string> getCommentTerms();
  list<string> getTermList();
  set<string> getCodeSymbols();
  set<string> getCodeSymbolTerms();
  string getData();
  string getCodeLineData();
  string getCodeLine();
  map< string, list<string> > getRevisionCommentWords();
  map< string, string > getRevisionCommentString();
};

void readTags( const string& fn, set<string>& S, map<string, set<string> >& parents);

string get_cvsdata();
