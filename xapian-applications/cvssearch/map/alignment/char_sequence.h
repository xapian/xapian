#ifndef __CHAR_SEQUENCE_H__
#define __CHAR_SEQUENCE_H__

#include <string>
using std::string;

class char_sequence
{
private:
    const string & _entries;
public:
    char_sequence(const string & l ) : _entries(l) {}
    static char space();
    static int score( const char c1, const char c2 );

    unsigned int size() const {  return _entries.size(); }
    const char operator[](unsigned int i) const { assert (i > 0 && i <= size()); return _entries[i-1]; }
};

#endif
