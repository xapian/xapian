/* index_utils.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003,2004,2005 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include <config.h>
#include "index_utils.h"
#include "utils.h"
#include <iostream>

using namespace std;

void lowercase_term(string &term)
{
    string::iterator i = term.begin();
    while (i != term.end()) {
	*i = C_tolower(*i);
	i++;
    }
    string::size_type a;
    while ((a = term.find("^s")) != term.npos) {
	term.replace(a, 2, " ");
    }
    while ((a = term.find("^n")) != term.npos) {
	term.replace(a, 2, "\n");
    }
    while ((a = term.find("^b")) != term.npos) {
	term.replace(a, 2, "\\");
    }
    while ((a = term.find("^t")) != term.npos) {
	term.replace(a, 2, "\t");
    }
    while ((a = term.find("^0")) != term.npos) {
	term.replace(a, 2, string("\0", 1));
    }
    while ((a = term.find("^x")) != term.npos) {
	if (a > term.size() - 4) return;
	char b = term[a + 2];
	char c = term[a + 3];
	if (C_isdigit(b))
	    b -= '0';
	else if (C_isupper(b) <= 'Z')
	    b -= ('A' - 10);
	else
	    b -= ('a' - 10);
	if (C_isdigit(c))
	    c -= '0';
	else if (C_isupper(c) <= 'Z')
	    c -= ('A' - 10);
	else
	    c -= ('a' - 10);
	b = b * 16 + c;
	term.replace(a, 4, string(&b, 1));
    }
}

// Keep only the characters in keep
// FIXME - make this accept character ranges in "keep"
void select_characters(string &term, const string & keep)
{
    string chars;
    if (keep.empty()) {
	chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
		"0123456789^";
    } else {
	chars = keep;
    }
    string::size_type pos;
    while ((pos = term.find_first_not_of(chars)) != string::npos) {
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
	para += "\n";
	linecount ++;
	if (linecount > 30) break;
    } while (line.find_first_not_of(" \t\r\f") != string::npos || linecount < 3);
}

// Read a non-blank line from stream.
void get_a_line(istream &input, string &line) {
    while (input) {
	getline(input, line);
	if (line.find_first_not_of(" \t\r\f") != string::npos) break;
    }
}
