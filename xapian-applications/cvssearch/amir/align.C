// align.C
//
// (c) 2001 Amir Michail (amir@users.sourceforge.net)

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// align file1 file2

#define GNU_DIFF_MARKER_COL 64
#define DIFF_OUTPUT "/tmp/diff_output"
#define DEBUG_MODE 1

#include <string>
#include <vector>
#include <list>
#include <iostream.h>
#include <fstream.h>


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

  void reconstruct() {
    // comes up with one alignment, not necessarily
    // all alignments

    int i = S.size();
    int j = T.size();

    list<string> s;
    list<string> t;

    while ( i >0 || j > 0 ) {

      int v = V[i][j];
      //      if ( DEBUG_MODE ) cerr << "v = " << v << endl;
      
      if ( i>0 && j >0 && v == V[i-1][j-1] + S.score( S[i], T[j] ) ) {
	s.push_front( S[i] );
	t.push_front( T[j] );
	i--;
	j--;
      } else if ( i>0 && v == V[i-1][j] + S.score(S[i], T.space()) ) {
	s.push_front( S[i] );
	t.push_front( "$" );
	i--;
      } else {
	assert( v == V[i][j-1] + S.score(S.space(), T[j] )) ;
	s.push_front( "$" );
	t.push_front( T[j] );
	j--;
      }

    }
    assert( s.size() == t.size() );

    list<string>::iterator is = s.begin();
    list<string>::iterator it = t.begin();
    for ( int i = 0; i < s.size(); i++ ) {
      if ( DEBUG_MODE ) cerr << (*is) << "  |  " << (*it) << endl;
      is++;
      it++;
    }
    
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
      return +2;
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
      getline( in, line, '\n' );

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

    //    if ( DEBUG_MODE ) cerr << "optimal alignment of " << l1 << " and " << l2 << " is " << alignment.optimalAlignmentValue() << endl;
    //    alignment.dump();
    return alignment.optimalAlignmentValue();

  }

};

void processDiffOutput( const string& f, LineSequence& l1, LineSequence& l2 ) {

  ifstream in(f.c_str());
  if ( ! in ) {
    if ( DEBUG_MODE ) cerr << "Couldn't open " << f << endl;
    abort();
  }

  int blockSize1 = 0;
  int blockSize2 = 0;

  bool foundChangeMarker = false;

  int line_num_1 = 1;
  int line_num_2 = 1;

  int blockStart1 = 0;
  int blockStart2 = 0;
  
  while ( ! in.eof() ) {
    string line;
    getline( in, line, '\n' );
    
    //    if ( DEBUG_MODE ) cerr << line << endl;
    char marker = 0;
    if ( line != "" ) {
      marker = line[GNU_DIFF_MARKER_COL];
    }
    //    if ( DEBUG_MODE ) cerr << marker << endl;
        
    if ( marker == '|' ) {
      foundChangeMarker = true;
    }
    if ( marker == '|' || marker == '<' || marker == '>' ) {

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
    } else {
      if ( foundChangeMarker ) {
	assert( blockSize1 > 0 );
	assert( blockSize2 > 0 );

	if ( DEBUG_MODE ) cerr << "\n\n\n*********** Found change block at lines " << blockStart1 << " and " << blockStart2 << endl;

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
	alignment.reconstruct();

      }
      blockStart1 = 0;
      blockStart2 = 0;
      blockSize1 = 0;
      blockSize2 = 0;
      foundChangeMarker = false;
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
      // blank line found in diff
      line_num_1++;
      line_num_2++;
    }
  }
  
  in.close();
  
}

int main( int argc, char *argv[] ) {
  
  assert( argc == 3 );

  string f1 = argv[1];
  string f2 = argv[2];

  if ( DEBUG_MODE ) cerr << "Comparing " << f1 << " with " << f2 << "." << endl;

  // the first thing to do is to call GNU diff to get an approximate
  // alignment

  string diff_cmd = "diff --expand-tabs --ignore-space-change --side-by-side " + f1 + "  " + f2 + "> " + DIFF_OUTPUT;
  int rc = system(diff_cmd.c_str());

  if ( DEBUG_MODE ) cerr << "GNU diff returned " << rc << endl;

  LineSequence l1(f1);
  LineSequence l2(f2);

  processDiffOutput( DIFF_OUTPUT, l1, l2 );
  
}
