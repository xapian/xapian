/* index_utils.cc
 */

#include "index_utils.h"
#include "omtypes.h"
#include <ctype.h>
#include <cctype>

void lowercase_term(termname &term)
{
    termname::iterator i = term.begin();
    while(i != term.end()) {
	*i = tolower(*i);
	i++;
    }
}

// Keep only the characters in keep
// FIXME - make this accept character ranges in "keep"
void select_characters(termname &term, const string & keep)
{
    string chars;
    if(keep.size() == 0) {
	chars ="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    } else {
	chars = keep;
    }
    string::size_type pos;
    while((pos = term.find_first_not_of(chars)) != string::npos)
    {
	string::size_type endpos = term.find_first_of(chars, pos);
	term.erase(pos, endpos - pos);
    }
}

// Read a paragraph from stream.
void get_paragraph(istream &input, string &para) {
    para = "";
    string line;
    unsigned linecount = 0;
    do {
	getline(input, line);
	para += line;
	linecount ++;
	if(linecount > 30) break;;
	para += " ";
    } while(line.find_first_not_of(" \t") != string::npos || linecount < 3);
}

// Read a line from stream.
void get_a_line(istream &input, string &line) {
    while(input) {
	getline(input, line);
	if(line.find_first_not_of(" \t") != string::npos) break;
    }
}
