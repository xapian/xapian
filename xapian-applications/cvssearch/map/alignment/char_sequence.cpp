#include "char_sequence.h"

char
char_sequence::space() 
{
    return (char)1;
}

int
char_sequence::score( const char c1, const char c2 ) 
{
    assert( c1 != space() || c2 != space() );
    if ( c1 == c2 ) {
        return +2;
    }
    return -1;
}
