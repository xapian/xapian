/* index_utils.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003,2004 Olly Betts
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

void lowercase_term(string &term)
{
    string::iterator i = term.begin();
    while (i != term.end()) {
	*i = C_tolower(*i);
	i++;
    }
    std::string::size_type a;
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
	term.replace(a, 2, std::string("\t", 1));
    }
    while ((a = term.find("^0")) != term.npos) {
	term.replace(a, 2, std::string("\0", 1));
    }
    while ((a = term.find("^x")) != term.npos) {
	if (a > term.size() - 4) return;
	char b = term[a + 2];
	char c = term[a + 3];
	if (b <= '9') b = b - '0';
	else if (b <= 'Z') b = b - 'A';
	else b = b - 'a';
	if (c <= '9') c = c - '0';
	else if (c <= 'Z') c = c - 'A';
	else c = c - 'a';
	b = b * 16 + c;
	term.replace(a, 4, std::string(&b, 1));
    }
}

// Keep only the characters in keep
// FIXME - make this accept character ranges in "keep"
void select_characters(string &term, const std::string & keep)
{
    std::string chars;
    if(keep.size() == 0) {
	chars ="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789^";
    } else {
	chars = keep;
    }
    std::string::size_type pos;
    while((pos = term.find_first_not_of(chars)) != std::string::npos)
    {
	std::string::size_type endpos = term.find_first_of(chars, pos);
	term.erase(pos, endpos - pos);
    }
}

// Read a paragraph from stream.
void get_paragraph(std::istream &input, std::string &para) {
    para = "";
    std::string line;
    unsigned linecount = 0;
    do {
	std::getline(input, line);
	para += line;
	para += "\n";
	linecount ++;
	if(linecount > 30) break;;
    } while(line.find_first_not_of(" \t\f") != std::string::npos || linecount < 3);
}

// Read a line from stream.
void get_a_line(std::istream &input, std::string &line) {
    while(input) {
	std::getline(input, line);
	if(line.find_first_not_of(" \t\f") != std::string::npos) break;
    }
}
