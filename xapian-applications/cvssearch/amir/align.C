// align file1 file2

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
    //    cerr << "Initializing." << endl;

    for(int i = 0; i <= S.size(); i++ ) {

      vector<int> empty;
      V.push_back(empty);

      for( int j = 0; j <= T.size(); j++ ) {
	//	cerr << "i = " << i << " and j = " << j << endl;
	V[i].push_back(0);
      }
      assert( V[i].size() == T.size()+1 );
    }
    assert( V.size() == S.size()+1 );

    //    cerr << "Done." << endl;

  }

  void findOptimalAlignment() {

    for( int i = 1; i <= S.size(); i++ ) {
      V[ i ][0] = V[i-1][0] + S.score( S[i], S.space() );
    }

    for( int j = 1; j <= T.size(); j++ ) {
      V[0][j] = V[0][j-1] + T.score( T.space(), T[j] );
    }
    
    // recurrence

    for ( int i = 1; i <= S.size(); i++ ) {
      for ( int j = 1; j <= T.size(); j++ ) {
	int v = V[i-1][j-1] + S.score( S[i], T[j] );
	v = max( v, V[i-1][j] + S.score(S[i], T.space()) );
	v = max( v, V[i][j-1] + S.score(S.space(), T[j] ) );
	V[i][j] = v;
      }
    }
  }

  void dump() {
    cerr << "..dump.." << endl;
    for (int i = 0; i <= S.size(); i++ ) {
      for (int j = 0; j <= T.size(); j++ ) {
	cerr << V[i][j] << " ";
      }
      cerr << endl;
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
      //      cerr << "v = " << v << endl;
      
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
      cerr << (*is) << "  |  " << (*it) << endl;
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
    if ( c1 != space() && c1 == c2 ) {
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
    s = "\001";
    return s;
  }

  LineSequence( const string& f ) {
    ifstream in(f.c_str());
    if ( ! in ) {
      cerr << "Couldn't open " << f << endl;
      abort();
    }
    
    while ( ! in.eof() ) {
      string line;
      getline( in, line, '\n' );

      // strip whitespace at beginning & end of line
      // (otherwise, matching whitespace is rewarded!)

      string line2 = trim(line);

      lines.push_back(line2);
      // cerr << "pushed line -" << line << "-" << endl;
    }

    in.close();

    cerr << "...read " << lines.size() << " lines from " << f << endl;
  }

  int size() {
    return lines.size();
  }

  string operator[](int i) {
    assert( i > 0);
    return lines[i-1];
  }

  int score( const string& l1, const string& l2 ) {

    CharSequence c1(l1);
    CharSequence c2(l2);

    Alignment<CharSequence> alignment(c1,c2);

    alignment.findOptimalAlignment();

    //    cerr << "optimal alignment of " << l1 << " and " << l2 << " is " << alignment.optimalAlignmentValue() << endl;
    //    alignment.dump();
    return alignment.optimalAlignmentValue();

    /***

    if ( l1 == l2 ) {
    return +2;
    }

    return -1;

    **/


  }

};


int main( int argc, char *argv[] ) {
  
  assert( argc == 3 );

  string f1 = argv[1];
  string f2 = argv[2];

  cerr << "Comparing " << f1 << " with " << f2 << "." << endl;
  
  LineSequence l1(f1);
  LineSequence l2(f2);
  Alignment<LineSequence> alignment( l1, l2 );
  alignment.findOptimalAlignment();
  cerr << "*** optimal alignment value is " << alignment.optimalAlignmentValue() << endl;
  alignment.reconstruct();

}
