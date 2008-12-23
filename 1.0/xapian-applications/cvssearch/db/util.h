// util.h
//
// (c) 2001 Amir Michail (amir@users.sourceforge.net)
// Copyright (C) 2004 Olly Betts

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

#include <string>
#include <map>
#include <set>
#include <list>
#include <fstream>
using namespace std;

void lowercase_term(string &term);
void lowercase_string(string &term);
void split (const string & text, const string & separators, list<string> & words);
void readTags(istream & in, set<string>& S);

bool blankChar  (char c);
bool okFirstChar(char c);
bool okSubChar  (char c);
string get_cvsdata();

inline string uint_to_string(unsigned int v) {
    char buf[32];
    sprintf(buf, "%u", v);
    return string(buf);
}

string convert(const string & input, char src, char dst);

void get_packages(istream & is, set<string> & packages);

#include "lines_cmt.h"
#include "lines_db.h"
