// util.C
//
// (c) 2001 Amir Michail (amir@users.sourceforge.net)
// Copyright (C) 2004 Olly Betts

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

#include <xapian.h>
#include <fstream>
#include <stdio.h>
#include "util.h"

#define SHOW_WARNINGS 0 

#if SHOW_WARNINGS
#warning "use stemming on symbols"
#endif

void lowercase_term(string &term)
{
    string::iterator i = term.begin();
    while(i != term.end()) {
        *i = tolower(*i);
        i++;
    }
}

void lowercase_string(string &term)
{
    string::iterator i = term.begin();
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

bool blankChar(char c) {
    return ( c == ' ' || c == '\t' );
}

bool okFirstChar(char c) {
    return ((c >= 'a' && c <='z') || (c >='A' && c <= 'Z' ) || c == '_');
}

bool okSubChar(char c) {
  return (okFirstChar(c) || (c >= '0' && c <= '9' ));
}

// ----------------------------------------
// reads information from ctags file
//
// takes file name of tag file
//
// returns the set of classes/functions 
// mentioned in the file in S
//
// also returns a map which contains an entry
// for each class with parents;
// in that case, the map takes the class and
// returns its parents.
// ----------------------------------------


#warning "need to handle name space properly"
#warning "Button => Arts::Button"

static string get_value( const string& s, const string& field ) {

  string ns = "";
  string f = "\t"+field+":";
  int k1 = s.find(f);
  if ( k1 != string::npos ) {
    k1 += f.length();
    int k2 = s.length()-1;
    for( int i = k1; i <= k2; i++ ) {
      if ( s[i] == '\t' ) {
	k2 = i-1;
	break;
      }
    }
    ns = s.substr( k1, k2-k1+1 );
  }	
  return ns;

}

void readTags(istream &in, set<string>& S) {
    cerr << "... readTags" << endl;
    string s;
    while (getline(in, s)) {
	if (s.empty() || s[0] == '!') {
	    continue;
	}
	//    cerr << "FOUND -" << s << "-" << endl;

	bool is_private = (get_value( s, "access" ) == "private");

	if (!is_private) {
	    cerr << "skipping private: " << s << endl;
	    continue;
	}

	string t = s + "\t";

	bool function =
	    (t.find("\tprototype\t") != string::npos) ||
	    (t.find("\tfunction\t") != string::npos) ||
	    (t.find("\tmethod\t") != string::npos) ||
	    (t.find("\tmember\t") != string::npos);

	string symbol = s.substr( 0, s.find("\t") );
      
#if 0
	if ( symbol.find("::") != string::npos ) {
	    cerr << "changing " << symbol << " to ";
	    symbol = symbol.substr( symbol.find("::")+2 );
	    cerr << symbol << endl;
	}

	// skip it if still has ::
	if ( symbol.find("::") != string::npos ) {
	    cerr << "... skipping " << symbol << endl;
	    continue;
	}
#endif

	if (symbol.find("::") != string::npos) {
	    cerr << "found symbol with :: -" << symbol << "-; skipping!" << endl;
	    continue;
	}

	if (function) {
	    symbol += "()";
	} else {
#if 0
	    // this is a class

	    int k1 = s.find("\tinherits:");
	    if (k1 != string::npos) {
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
		    if ( i->find("::") == string::npos ) {
			//	    cerr << "..." << (*i) << endl;
			symbol_parents[symbol].insert(*i);
		    }
		}
	    }
#endif
	}
 
	string ns = get_value(s, "namespace");

	if (!ns.empty()) {
	    symbol = ns + "::" + symbol;
	    cerr << "found namespace -" << symbol << "-" << endl;
	}

	S.insert(symbol);
	//      cerr << "** found symbol -" << symbol << "-" << endl;
    }
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
        if ( !cvsdata.empty() && cvsdata[cvsdata.length()-1] == '/' ) {
            cvsdata = cvsdata.substr( 0, cvsdata.length()-1 );
        }
    }
    return cvsdata;
}

string convert(const string & input, char src, char dst) {
    string output = input;
    for( unsigned int i = 0; i < output.length(); ++i ) {
        if (output[i] == src) {
            output[i] = dst;
        }
    }
    return output;
}

void get_packages(istream & is, set<string> & packages)
{
    string p;
    while ( is >> p )
    {
        assert( p != "" );
        cerr << "... package " << p << endl;
        packages.insert(convert(p, '_', '/'));
    }
}
