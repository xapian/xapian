// lines_db.h
//
// (c) 2001 Andrew Yao (andrewy@users.sourceforge.net)

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

#include <list>
#include <map>
#include <fstream>
#include <vector>

extern void lowercase_term(om_termname &term);
extern void lowercase_string(string &term);
extern void split (const string & text, const string & separators, list<string> & words);

class lines_db 
{
private:
    map< string, list<string> > revision_comment_words;
    map< string, string > revision_comment_string;

    ifstream *in_comments;
    ifstream *in_code;
    OmStem *stemmer;

    set<string> terms;
    list<string> term_list;
    set<string> symbols;
    set<string> symbol_terms;

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

    bool blankChar(char c);
    bool okFirstChar(char c);
    bool okSubChar(char c);
    void extractSymbols( const string& s );
    void load_offset_file(  const string& file_offset, vector<string>& files, vector<string>& offsets );
    void readVector( const string& line, const string& field, vector<string>& field_vector );
    string root;
    string package;
    
    void stemWords( const list<string>& words, list<string>& term_list );

    string message;

public:
    
    lines_db (cvs_db_file & db_file);
    ~lines_db();
    bool ReadNextLine();

    void updateRevisionComments( map< string, list<string> >& revision_comment_words );
    int getLineNumber() const;
    const string & currentFile() const;
    const set<string> & getCommentTerms()    const;
    const list<string>& getTermList()        const;
    const set<string> & getCodeSymbols()     const;
    const set<string> & getCodeSymbolTerms() const;
    const string      & getData()            const;
    const string      & getCodeLineData()    const;
    const string      & getCodeLine()        const;
    const map< string, list<string> > & getRevisionCommentWords() const;
    const map< string, string > & getRevisionCommentString() const;
};

string get_cvsdata();
