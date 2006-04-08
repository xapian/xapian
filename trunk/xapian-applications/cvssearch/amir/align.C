// align.C (for executables align and search)
//
// (c) 2001 Amir Michail (amir@users.sourceforge.net)
// Copyright (C) 2004 Olly Betts

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// align file1 file2
//  output in delta format
//
// search block file => returns lines in file that correspond to lines in block
//  first line is score
//  output in delta format

// uses all of block and part of file
// if HYBRID_MODE is 0, does a pure
// local similarity that looks at part
// of block and part of file
#define HYBRID_MODE 0

// was +2
#define MATCH_SCORE +2

// in search mode, I believe we can speed this program
// up by specifying a threshold below which it will just
// give up
//
// for example, if there is no way of getting a score > 0,
// then we just give up and return no delta at all.
//




// in delta format, line numbers refer to original
// line numbers in file1 and file2 

#define DEBUG_MODE 0 

static bool search_mode = false;

#include <string>
#include <vector>
#include <list>
#include <iostream.h>
#include <fstream.h>

#include "pstream.h"

template<class type>
class LocalAlignment {
  type &S;
  type &T;
  vector< vector< int > > V;
  int max_row;
  int max_col;
  int max_val;
public:
  LocalAlignment( type &s, type &t ) : S(s), T(t) {

    max_row = 0;
    max_col = 0;
    max_val = 0;

    // initialize V to zero's 
    //    if ( DEBUG_MODE ) cerr << "Initializing." << endl;

    for(int i = 0; i <= S.size(); i++ ) {

      vector<int> empty;
      V.push_back(empty);

      for( int j = 0; j <= T.size(); j++ ) {
	//	if ( DEBUG_MODE ) cerr << "i = " << i << " and j = " << j << endl;
	V[i].push_back(0);
      }
      assert( V[i].size() == T.size()+1 );
    }
    assert( V.size() == S.size()+1 );

    //    if ( DEBUG_MODE ) cerr << "Done." << endl;

  }



  // V[i][j] is the maximum value of an optimal (global) alignment
  // of S[1]...S[i] and all suffixes t' of T[1]..T[j].

  int findOptimalAlignment( int debug = 0 ) {

    // we do not have the option of suffixes for S
    // so we have to do this
#if HYBRID_MODE
      for( int i = 1; i <= S.size(); i++ ) {
	V[ i ][0] = V[i-1][0] + S.score( S[i], S.space() );
      }
#endif

    // with T, we can have suffixes
    // so V[0][j] == 0 for all j
    
    // recurrence

    for ( int i = 1; i <= S.size(); i++ ) {
      if ( debug ) if ( DEBUG_MODE ) cerr << "..." << i << " / " << S.size() << endl;
      for ( int j = 1; j <= T.size(); j++ ) {

	//	int v = 0; // we can't always start over here, we can only start over with T

	// instead, we start over with T only
#if HYBRID_MODE
	int v = V[i][0]; // optimal alignment of S[1]..S[i] and empty suffix of T[1]..T[j]
#else
	int v = 0;
#endif	

                v = max( v, V[i-1][j-1] + S.score( S[i], T[j] ) );
	v = max( v, V[i-1][j] + S.score(S[i], T.space()) );
	v = max( v, V[i][j-1] + S.score(S.space(), T[j] ) );
	V[i][j] = v;

#if HYBRID_MODE
	if ( V[i][j] > max_val || i > max_row ) { // we require a maximum in bottom row
#else
	  if ( V[i][j] > max_val ) {
#endif
	  if (DEBUG_MODE ) cerr << "max of " << max_val  << " now in row " << i << endl;
	  if (DEBUG_MODE) cerr << "...col = " << j << endl;
	  max_row = i;
	  max_col = j;
	  max_val = V[i][j];
	} 
      }
    }
    return max_val;
  }

  void dump() {
    if ( DEBUG_MODE ) cerr << "..dump.." << endl;
    for (int i = 0; i <= S.size(); i++ ) {
      for (int j = 0; j <= T.size(); j++ ) {
	if ( DEBUG_MODE ) cerr << V[i][j] << " ";
      }
      if ( DEBUG_MODE ) cerr << endl;
    }
  }

  int optimalAlignmentValue() {
    return V[ max_row ][ max_col ];
  }

  string reconstruct( int& start1, int& start2 ) { // returns markerSequence;

    // comes up with one alignment, not necessarily
    // all alignments

    int i = max_row;
    int j = max_col;

    if (DEBUG_MODE) {
      cerr << "reconstruct called with max row " << i << " and max col " << j << endl;
    }

    list<string> s;
    list<string> t;

    string markerSequence;

#if HYBRID_MODE
    while ( i >0 ) { 
#else 
      for(;;) {
#endif

      int v = V[i][j];
      //      if ( DEBUG_MODE ) cerr << "v = " << v << endl;

#if (!HYBRID_MODE) 
      if ( v == 0 ) {
	break; // all done
      }
#endif


      if ( i>0 && j >0 && v == V[i-1][j-1] + S.score( S[i], T[j] ) ) {
	s.push_front( S[i] );
	t.push_front( T[j] );
	i--;
	j--;

	markerSequence = '|' + markerSequence;

      } else if ( i>0 && v == V[i-1][j] + S.score(S[i], T.space()) ) {
	s.push_front( S[i] );
	t.push_front( "$" );
	i--;

	markerSequence = '<' + markerSequence;

      } else if ( j > 0 && v == V[i][j-1] + S.score(S.space(), T[j])) {
	assert( v == V[i][j-1] + S.score(S.space(), T[j] )) ;
	s.push_front( "$" );
	t.push_front( T[j] );
	j--;

	markerSequence = '>' + markerSequence;
      }  else {
	assert(0);
      }


    }

#if (!HYBRID_MODE)
      start1 = i+1;
#else
      start1 = 1;
#endif

    start2 = j+1;

    assert( s.size() == t.size() );

    if ( DEBUG_MODE ) {
      list<string>::iterator is = s.begin();
      list<string>::iterator it = t.begin();
      for ( int i = 0; i < s.size(); i++ ) {
	type l1;
	l1.addLine(*is);
	type l2;
	l2.addLine(*it);
	int score = l1.score( l1[1], l2[1] );
	cerr << (*is) << "  |  " << (*it) << " with ****** score " << score <<  endl;
	is++;
	it++;
      }
    }
   
    return markerSequence;
  }

};



template<class type>
class Alignment {
  type &S;
  type &T;
  vector< vector< int > > V;
public:
  Alignment( type &s, type &t ) : S(s), T(t) {

    // initialize V to zero's 
    //    if ( DEBUG_MODE ) cerr << "Initializing." << endl;

    for(int i = 0; i <= S.size(); i++ ) {

      vector<int> empty;
      V.push_back(empty);

      for( int j = 0; j <= T.size(); j++ ) {
	//	if ( DEBUG_MODE ) cerr << "i = " << i << " and j = " << j << endl;
	V[i].push_back(0);
      }
      assert( V[i].size() == T.size()+1 );
    }
    assert( V.size() == S.size()+1 );

    //    if ( DEBUG_MODE ) cerr << "Done." << endl;

  }

  void findOptimalAlignment( int debug = 0 ) {

    for( int i = 1; i <= S.size(); i++ ) {
      V[ i ][0] = V[i-1][0] + S.score( S[i], S.space() );
    }

    for( int j = 1; j <= T.size(); j++ ) {
      V[0][j] = V[0][j-1] + T.score( T.space(), T[j] );
    }
    
    // recurrence

    for ( int i = 1; i <= S.size(); i++ ) {
      if ( debug ) if ( DEBUG_MODE ) cerr << "..." << i << " / " << S.size() << endl;
      for ( int j = 1; j <= T.size(); j++ ) {
	int v = V[i-1][j-1] + S.score( S[i], T[j] );
	v = max( v, V[i-1][j] + S.score(S[i], T.space()) );
	v = max( v, V[i][j-1] + S.score(S.space(), T[j] ) );
	V[i][j] = v;
      }
    }
  }

  void dump() {
    if ( DEBUG_MODE ) cerr << "..dump.." << endl;
    for (int i = 0; i <= S.size(); i++ ) {
      for (int j = 0; j <= T.size(); j++ ) {
	if ( DEBUG_MODE ) cerr << V[i][j] << " ";
      }
      if ( DEBUG_MODE ) cerr << endl;
    }
  }

  int optimalAlignmentValue() {
    return V[ S.size()][ T.size() ];
  }

  string reconstruct() { // returns markerSequence;

    // comes up with one alignment, not necessarily
    // all alignments

    int i = S.size();
    int j = T.size();

    list<string> s;
    list<string> t;

    string markerSequence;

    while ( i >0 || j > 0 ) {

      int v = V[i][j];
      //      if ( DEBUG_MODE ) cerr << "v = " << v << endl;
      
      if ( i>0 && j >0 && v == V[i-1][j-1] + S.score( S[i], T[j] ) ) {
	s.push_front( S[i] );
	t.push_front( T[j] );
	i--;
	j--;

	markerSequence = '|' + markerSequence;

      } else if ( i>0 && v == V[i-1][j] + S.score(S[i], T.space()) ) {
	s.push_front( S[i] );
	t.push_front( "$" );
	i--;

	markerSequence = '<' + markerSequence;

      } else {
	assert( v == V[i][j-1] + S.score(S.space(), T[j] )) ;
	s.push_front( "$" );
	t.push_front( T[j] );
	j--;

	markerSequence = '>' + markerSequence;

      }

    }
    assert( s.size() == t.size() );

    if ( DEBUG_MODE ) {
      list<string>::iterator is = s.begin();
      list<string>::iterator it = t.begin();
      for ( int i = 0; i < s.size(); i++ ) {
	cerr << (*is) << "  |  " << (*it) << endl;
	is++;
	it++;
      }
    }   

    return markerSequence;
  }

};


class CharSequence {
  vector<char> chars;
public:
  CharSequence( string l ) {
    for( string::iterator c = l.begin(); c != l.end(); c++ ) {
      chars.push_back(*c);
    }

    
  }

  char space() {
    return (char)1;
  }

  int score( const char c1, const char c2 ) {
    assert( c1 != space() || c2 != space() );
    if ( c1 == c2 ) {
      return MATCH_SCORE;
    }
    return -1;
  }

  char operator[](int i) {
    assert( i > 0);
    return chars[i-1];
  }

  int size() {
    return chars.size();
  } 
};

class LineSequence {
  vector<string> lines;

  string trim(const string& s) {
    if(s.length() == 0)
      return s;
    int b = s.find_first_not_of(" \t");
    int e = s.find_last_not_of(" \t");
    if(b == -1) // No non-spaces
      return "";
    return string(s, b, e - b + 1);
  }


public:
  string space() {
    string s;
    s = "\002";
    return s;
  }

  LineSequence( const string& f ) {
    ifstream in(f.c_str());
    if ( ! in ) {
      if ( DEBUG_MODE ) cerr << "Couldn't open " << f << endl;
      abort();
    }
    
    while ( ! in.eof() ) {
      string line;
      if (getline( in, line, '\n' ).eof()) {
	if ( line == "" ) {
	  break;
	}
      }

      // strip whitespace at beginning & end of line
      // (otherwise, matching whitespace is rewarded!)

      string line2 = trim(line);

      lines.push_back(line2);
      // if ( DEBUG_MODE ) cerr << "pushed line -" << line << "-" << endl;
    }

    in.close();

    if ( DEBUG_MODE ) cerr << "...read " << lines.size() << " lines from " << f << endl;
  }

  LineSequence() {
  }

  void addLine( const string& line ) {
    string line2 = trim(line);
    lines.push_back(line2);
  }

  int size() {
    return lines.size();
  }

  string operator[](int i) {
    assert( i > 0);
    return lines[i-1];
  }

  int score( const string& l1, const string& l2 ) {

    assert( l1 != space() || l2 != space() );

    CharSequence c1(l1);
    CharSequence c2(l2);

    Alignment<CharSequence> alignment(c1,c2);

    alignment.findOptimalAlignment();

    if ( DEBUG_MODE ) cerr << "optimal alignment of " << l1 << " and " << l2 << " is " << alignment.optimalAlignmentValue() << endl;
    //    alignment.dump();
    return alignment.optimalAlignmentValue();

  }

};


void outputDelta( string markerSequence, int blockStart1, int blockStart2 ) {
  //  cerr << "outputDelta called with " << markerSequence << " at " << blockStart1 << " and " << blockStart2 << endl;

  int lineno1 = blockStart1;
  int lineno2 = blockStart2;

  while ( markerSequence != "" ) {
    char marker = markerSequence[0];
    int count = 0;
    while ( markerSequence != "" ) {
      if ( markerSequence[0] == marker ) {
	//	cout << markerSequence << endl;
	markerSequence = string( markerSequence, 1, markerSequence.length()-1 );
	count++;
      } else {
	break;
      }
    }

    //    cout << "marker " << marker << " with count " << count << endl;
    
    if ( marker == '|' ) {
      if ( count == 1 ) {
	cout << lineno1 << "c" << lineno2 << endl;
      } else {
	cout << lineno1 << "," << (lineno1 + count-1) << "c" << lineno2 << "," << (lineno2+count-1) << endl;
      }
      lineno1 += count;
      lineno2 += count;

    } else if ( marker == '<' ) {
      // file1 has extra lines, so delete them
      if ( count == 1 ) {
	cout << lineno1 << "d" << (lineno2-1) << endl;
      } else {
	cout << lineno1 << "," << (lineno1 + count-1) << "d" << (lineno2-1) << endl;
      }

    lineno1 += count;

    } else if ( marker == '>' ) {
      // file1 has is missing lines, so add them
      if ( count == 1 ) {
	cout << (lineno1-1) << "a" << lineno2 << endl;
      } else {
	cout << (lineno1-1) << "a" << lineno2 << "," << (lineno2+count-1) << endl;
      }

    lineno2 += count;

    } else {
      assert(0);
    }



  }
}

void processDiffOutput(ifstream & in, LineSequence& l1, LineSequence& l2) {
  int blockSize1 = 0;
  int blockSize2 = 0;

  bool foundChangeMarker = false;

  int line_num_1 = 1;
  int line_num_2 = 1;

  int blockStart1 = 0;
  int blockStart2 = 0;

  string markerSequence;
  
  while ( ! in.eof() ) {
    string line;
    if(getline( in, line, '\n' ).eof()) {
      //      if ( line == "" ) {
	//	break; // we miss last block with this
      //      }
    }

    // strange diff bug
    // sometimes we get things like ^M| or ^M<
    // we change these to | or <
    
    if ( line.length() == 2 && line[0] == 13 ) {
      //      cerr << "** found line " << line << endl;
      line = string( line, 1 );
      //      cerr << "** changed to line " << line << endl;
      assert( line.length() == 1 );
    }

    char marker = 0;
    if ( line.length() > 0 ) {
      marker = line[0];
    } 
        
    if ( marker == '|' ) {
      foundChangeMarker = true;
    }
    if ( marker == '|' || marker == '<' || marker == '>' ) {
      markerSequence += marker;
      if ( marker == '|' ) {
	blockSize1++;
	blockSize2++;
      } else if ( marker == '<' ) {
	blockSize1++;
      } else if ( marker == '>' ) {
	blockSize2++;
      }

      if ( blockStart1 == 0 ) {
	assert( blockStart2 == 0 );
	blockStart1 = line_num_1;
	blockStart2 = line_num_2;
      }
    } else { // didn't find |, <, or >
      if( marker != 0 && marker != ' ' ) {
	cerr << "found marker '" << marker << "' ascii = " << (int)marker << endl;
	cerr << "in line " << endl;
	cerr << "-" << line << "-" << endl;
	assert(0);
      }
      if ( foundChangeMarker ) {
	assert( blockSize1 > 0 );
	assert( blockSize2 > 0 );

	if ( DEBUG_MODE ) cerr << "\n\n\n*********** Found change block at lines " << blockStart1 << " and " << blockStart2 << endl;
	if ( DEBUG_MODE ) cerr << "..marker sequence " << endl;
	if ( DEBUG_MODE) {
	  for (int i = 0; i < markerSequence.length(); i++ ) {
	    cerr << "..." << markerSequence[i] << endl;
	  }
	}

	LineSequence b1, b2;

	// line numbers are with respect to line sequences...
	if ( DEBUG_MODE ) cerr << "Block 1 source" << endl;
	for (int i = blockStart1; i < blockStart1 + blockSize1; i++ ) {
	  if ( DEBUG_MODE ) cerr << "=>" + l1[i] << endl;
	  b1.addLine(l1[i]);
	}

	if ( DEBUG_MODE ) cerr << endl;
	if ( DEBUG_MODE ) cerr << "Block 2 source" << endl;
	for (int i = blockStart2; i < blockStart2 + blockSize2; i++ ) {
	  if ( DEBUG_MODE ) cerr << "=>" + l2[i] << endl;
	  b2.addLine(l2[i]);
	}	

	Alignment<LineSequence> alignment( b1, b2 );
	alignment.findOptimalAlignment( );
	if ( DEBUG_MODE ) cerr << "*** optimal alignment value is " << alignment.optimalAlignmentValue() << endl;
	string marker_seq = alignment.reconstruct();

	//	cout << "marker sequence from alignment:  " << marker_seq << endl;
	outputDelta( marker_seq, blockStart1, blockStart2 );

      } else {
	if ( !markerSequence.empty() ) {
	  if ( DEBUG_MODE ) cerr << "\n\n******* Didn't find change marker in block" << endl;

	  outputDelta( markerSequence, blockStart1, blockStart2 );
	}
      }
      blockStart1 = 0;
      blockStart2 = 0;
      blockSize1 = 0;
      blockSize2 = 0;
      foundChangeMarker = false;
      markerSequence = "";
    }

    if ( marker != '|' && marker != ' ' && marker != 0 ) {
      if ( marker == '<' ) {
	line_num_1++;
      } else if ( marker == '>' ) {
	line_num_2++;
      } else {
	if ( DEBUG_MODE ) cerr << "** found marker '" << marker << "'" << endl;
	assert(0);
      }
    } else {
      // (may also have blank line here, not just `|')
      line_num_1++;
      line_num_2++;
    }
  }
}

int main( int argc, char *argv[] ) {
  
  //  assert( argc == 3 );

  string f1 = argv[1];
  
  for( int i = 2; i < argc; i++ ) {
    
    string f2 = argv[i];

    if ( argc > 3 ) {
      cerr << "** Looking at " << f2 << endl;
    }
    
    assert( string(argv[0]).find("search") == -1 ||
	    string(argv[0]).find("align") == -1 ); // don't want both in path
    
    search_mode = (string(argv[0]).find("search")  != -1 );
    
    //  cerr << "Search mode = " << search_mode << endl;
    
    if ( DEBUG_MODE ) cerr << "Comparing " << f1 << " with " << f2 << "." << endl;
    
    // the first thing to do is to call GNU diff to get an approximate
    // alignment
    
    if ( ! search_mode ) {
	vector<string> diff_args;
	diff_args.push_back("--width=1");
	diff_args.push_back("-tby"); // --expand-tabs --ignore-space-change --side-by-side
	diff_args.push_back(f1);
	diff_args.push_back(f2);
	redi::ipstream diff("diff", diff_args);
      
	LineSequence l1(f1);
	LineSequence l2(f2);

	processDiffOutput(diff, l1, l2);
    } else {
      
      // find block in file
      // uses local alignment
      
      LineSequence l1(f1);
      LineSequence l2(f2);
      
      if (DEBUG_MODE) cerr << "l1 size = " << l1.size() << endl;
      
      LocalAlignment<LineSequence> local_alignment( l1, l2 );
      int score = local_alignment.findOptimalAlignment();
      int start1, start2;
      string marker_seq = local_alignment.reconstruct( start1, start2 );
      cout << score << endl;
      outputDelta( marker_seq, start1, start2 );
    }

  }

  return 0;

}
