// util.h
//
// (c) 2001 Amir Michail (amir@users.sourceforge.net)

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

#include <om/om.h>
#include <strstream>
#include <string>
#include <map>
#include <set>
#include <list>

void lowercase_term(om_termname &term);
void lowercase_string(string &term);
void split (const string & text, const string & separators, list<string> & words);
void readTags( const string& fn, set<string>& S, map<string, set<string> >& parents
#if 0
      , map< string, string>& tag 
#endif
);

bool blankChar  (char c);
bool okFirstChar(char c);
bool okSubChar  (char c);
string get_cvsdata();

string convert(unsigned int count);
string convert(const string & input, char src, char dst);

void get_packages(istream & is, set<string> & packages);

#include "lines_cmt.h"
#include "lines_db.h"
