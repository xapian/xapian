#include "line_sequence.h"
#include "char_sequence.h"
#include "alignment.h"
#include <fstream>

string
line_sequence::trim(const string& s) 
{
    if(s.length() == 0)
        return s;
    int b = s.find_first_not_of(" \t");
    int e = s.find_last_not_of(" \t");
    if(b == -1) // No non-spaces
        return "";
    return string(s, b, e - b + 1);
}

string
line_sequence::space() 
{
    string s;
    s = "\002";
    return s;
}

istream & 
operator >>(istream & in, line_sequence & r) 
{
    while ( ! in.eof() ) {
        string line;
        getline( in, line, '\n' );
        
        // strip whitespace at beginning & end of line
        // (otherwise, matching whitespace is rewarded!)
        
        r._entries.push_back(r.trim(line));
        // cerr << "pushed line -" << line << "-" << endl;
    }
    return in;
}

line_sequence::line_sequence(istream & is) 
{
    is >> *this;
}

line_sequence::line_sequence(const string & f) 
{
    ifstream in(f.c_str());
    if ( ! in ) {
        cerr << "Couldn't open " << f << endl;
        abort();
    }
    
    in >> *this;

    in.close();

//    cerr << "...read " << _entries.size() << " lines from " << f <<endl;
}

int
line_sequence::score(const string & l1, const string & l2 ) 
{
    assert( l1 != space() || l2 != space() );
    
    char_sequence c1(l1);
    char_sequence c2(l2);

    alignment<char_sequence> align(c1,c2);

    align.find_optimal_alignment(false);

    return align.optimal_alignment_value();
}
