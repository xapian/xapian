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
void readTags( const string& fn, set<string>& S, map<string, set<string> >& symbol_parents ) {
    cerr << "... readTags " << fn << endl;
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
                    if ( i->find("::") == string::npos ) {
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

string convert(unsigned int count) {
    ostrstream ost;
    ost << count << ends;
    string s = ost.str(); 
    ost.freeze(0);
    return s;
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
        packages.insert(p);
    }
}
