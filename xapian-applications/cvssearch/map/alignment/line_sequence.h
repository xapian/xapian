#ifndef __LINE_SEQUENCE_H__
#define __LINE_SEQUENCE_H__

#include "sequence.h"
#include <string>
#include <iostream>
using std::string;
using std::vector;
using std::istream;

class line_sequence : public sequence<string>
{
protected:
    static string trim(const string & s);
public:
    line_sequence(istream & is);
    line_sequence(const string & filename);
    line_sequence(const vector<string> & entries) : sequence<string>(entries) {}
    friend istream & operator >>(istream & in, line_sequence & r);
    static int score(const string & l1, const string & l2 );
    static string space();
};

#endif;
