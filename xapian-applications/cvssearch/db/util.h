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

  map< string, list<string> > revision_comment_words;

  ifstream *in_comments;
  ifstream *in_code;
  OmStem *stemmer;

  set<string> terms;
  list<string> term_list;
  set<string> symbols;
  set<string> symbol_terms;

  set<string> terms_return;
  list<string> term_list_return;
  set<string> symbols_return;
  set<string> symbol_terms_return;
  
  string code_line;
  string prev_file;

  int line_no;
  string current_fn;
  string path;
  int current_offset;
  
  int file_count;

  string data;
  string codelinedata;

  vector<string> files;
  vector<string> offsets;

  set<string> termStopWords;
  set<string> symbolStopWords;

  string granularity; // line/file/app

  bool blankChar(char c);
  bool okFirstChar(char c);
  bool okSubChar(char c);
  void extractSymbols( const string& s );
  void load_offset_file(  const string& file_offset, vector<string>& files, vector<string>& offsets );
  void Lines::readVector( const string& line, const string& field, vector<string>& field_vector );
    string root;
    string package;
    
    void stemWords( const list<string>& words, list<string>& term_list );

public:
  Lines( const string& p, const string& root, const string& package, const string& file_db, const string& file_offset, const string& granularity = "line", bool use_stop_words = true );
  ~Lines();

  void updateRevisionComments( map< string, list<string> >& revision_comment_words );

  string currentFile();
  // returns false when there is no next line
  bool ReadNextLine();
  set<string> getCommentTerms();
  list<string> getTermList();
  set<string> getCodeSymbols();
  set<string> getCodeSymbolTerms();
  string getData();
  string getCodeLineData();
  string getCodeLine();
};

void readTags( const string& fn, set<string>& S );

string get_cvsdata();
