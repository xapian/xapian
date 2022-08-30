/** @file
 * @brief utility functions for indexing testcase data
 */
/* Copyright (C) 2005,2007,2013 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <config.h>

#include "index_utils.h"

#include "errno_to_string.h"
#include "stringutils.h"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <fstream>

using namespace std;

static string munge_term(const string &term);

/// Read a paragraph from stream @a input.
static string
get_paragraph(istream &input)
{
    string para, line;
    while (true) {
	getline(input, line);
	if (find_if(line.begin(), line.end(), C_isnotspace) == line.end())
	    return para;
	para += line;
	para += '\n';
    }
}

void
FileIndexer::index_to(Xapian::WritableDatabase & db)
{
    Xapian::Stem stemmer("english");

    while (file != end || (input.is_open() && !input.eof())) {
	if (input.eof()) next_file();

	Xapian::Document doc;
	string para = get_paragraph(input);
	doc.set_data(para);

	// Value 0 contains all possible character values so we can check that
	// none of them cause problems.
	string value0("X\0\0\0 \1\t"
	    "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
	    "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"
	    "\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f"
	    "\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\x3e\x3f"
	    "\x40\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4a\x4b\x4c\x4d\x4e\x4f"
	    "\x50\x51\x52\x53\x54\x55\x56\x57\x58\x59\x5a\x5b\x5c\x5d\x5e\x5f"
	    "\x60\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f"
	    "\x70\x71\x72\x73\x74\x75\x76\x77\x78\x79\x7a\x7b\x7c\x7d\x7e\x7f"
	    "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f"
	    "\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f"
	    "\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf"
	    "\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf"
	    "\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf"
	    "\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf"
	    "\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef"
	    "\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff",
	    7 + 256);
	if (para.size() > 2) value0[0] = para[2];
	value0 += para;
	doc.add_value(0, value0);

	for (Xapian::valueno i = min(para.length(), size_t(10)); i >= 1; --i) {
	    doc.add_value(i, para.substr(i, 1));
	}
	// Value 11 is useful for tests of sorting
	doc.add_value(11, Xapian::sortable_serialise(para.size()));

	// Value 12 is useful for tests of collapsing
	doc.add_value(12, Xapian::sortable_serialise(para.size() % 5));

	// Value 13 contains the first 3 letters of the paragraph
	doc.add_value(13, para.substr(0, 3));

	Xapian::termpos pos = 0;
	string::const_iterator word_end = para.begin();
	// Need a const_iterator version of para.end() for find_if.
	const string::const_iterator para_end = para.end();
	while (word_end != para_end) {
	    string::const_iterator word_start;
	    word_start = find_if(word_end, para_end, C_isnotspace);
	    word_end = find_if(word_start, para_end, C_isspace);
	    string word = stemmer(munge_term(string(word_start, word_end)));
	    if (!word.empty()) doc.add_posting(word, ++pos);
	}

	db.add_document(doc);
    }
}

// Strip unwanted characters, force to lower case, and handle \ escapes.
static string
munge_term(const string &term)
{
    string result;
    for (string::const_iterator i = term.begin(); i != term.end(); ++i) {
	char ch = *i;
	if (C_isalnum(ch))
	    result += C_tolower(ch);
	else if (ch == '\\') {
	    ++i;
	    if (i != term.end()) {
		switch (*i) {
		    case '\\': ch = '\\'; break;
		    case '0': ch = '\0'; break;
		    case 'n': ch = '\n'; break;
		    case 'r': ch = '\r'; break;
		    case 't': ch = '\t'; break;
		    case 'x': {
			// Check we can read the next two characters.
			if (size_t(i - term.begin()) >= term.size() - 2) {
			    --i;
			    break;
			}
			string::const_iterator j = i;
			char b = *++i;
			char c = *++i;
			if (!C_isxdigit(b) || !C_isxdigit(c)) {
			    i = j - 1;
			} else {
			    ch = hex_decode(b, c);
			}
			break;
		    }
		}
	    }
	    result += ch;
	}
    }
    return result;
}

void
FileIndexer::next_file()
{
    if (input.is_open()) {
	input.close();
	// MSVC doesn't clear fail() on close() and re-open().
	input.clear();
    }

    // Find the next non-empty filename.
    while (file != end && (*file).empty()) {
	++file;
    }
    if (file == end) return;

    string filename;
    if (!datadir.empty()) {
	filename = datadir;
	bool need_slash = true;
	for (char dir_sep : DIR_SEPS_LIST) {
	    if (filename.back() == dir_sep) {
		need_slash = false;
		break;
	    }
	}
	if (need_slash) filename += '/';
    }
    filename += *file++;
    filename += ".txt";

    input.open(filename.c_str());
    // Need to check is_open() - just using operator! fails with MSVC.
    if (!input.is_open()) {
	string msg = "Can't read file '";
	msg += filename;
	msg += "' for indexing (";
	errno_to_string(errno, msg);
	msg += ')';
	throw msg;
    }
}
