/* simpleindex.cc: Index each paragraph in a textfile as a document.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004 Olly Betts
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

#include <xapian.h>

#include <algorithm>
#include <iostream>
#include <string>

using namespace Xapian;
using namespace std;

#include <ctype.h>

// Put a limit on the size of terms to help prevent the index being bloated
// by useless junk terms
static const unsigned int MAX_PROB_TERM_LENGTH = 64;

inline static bool
p_alnum(unsigned int c)
{
    return isalnum(c);
}

inline static bool
p_notalnum(unsigned int c)
{
    return !isalnum(c);
}

inline static bool
p_notplusminus(unsigned int c)
{
    return c != '+' && c != '-';
}

static void
lowercase_term(string &term)
{
    string::iterator i = term.begin();
    while (i != term.end()) {
	*i = tolower(*i);
	i++;
    }
}

int main(int argc, char **argv)
{
    if (argc != 2) {
	cout << "usage: " << argv[0] << " <path to database>" << endl;
	exit(1);
    }

    WritableDatabase database;
    try {
	// Open the database
	database = Auto::open(argv[1], DB_CREATE_OR_OPEN);
    } catch (const Error &error) {
	cerr << "Exception: "  << error.get_msg() << endl;
	exit(1);
    }
    
    Stem stemmer("english");
    string para;
    while (true) {
	string line;
	if (cin.eof()) {
	    if (para.empty()) break;
	} else {
	    getline(cin, line); 
	}
	if (line.empty()) {
	    if (!para.empty()) {
		try {
		    Document doc;
		    doc.set_data(para);

		    termcount pos = 0;
		    string::iterator i, j = para.begin(), k;
		    while ((i = find_if(j, para.end(), p_alnum)) != para.end())
		    {
			j = find_if(i, para.end(), p_notalnum);
			k = find_if(j, para.end(), p_notplusminus);
			if (k == para.end() || !isalnum(*k)) j = k;
			string::size_type len = j - i;
			if (len <= MAX_PROB_TERM_LENGTH) {
			    string term = para.substr(i - para.begin(), len);
			    lowercase_term(term);
			    term = stemmer.stem_word(term);
			    doc.add_posting(term, pos++);
			}
		    }

		    // Add the document to the database
		    database.add_document(doc);
		} catch (const Error &error) {
		    cerr << "Exception: "  << error.get_msg() << endl;
		    exit(1);
		}

		para = "";
	    }
	}
	if (!para.empty()) para += ' ';
	para += line;
    }
}
