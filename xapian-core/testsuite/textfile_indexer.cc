/* textfile_indexer.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003 Olly Betts
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
#include "omdebug.h"
#include <xapian/error.h>
#include "textfile_indexer.h"
#include <xapian/stem.h>
#include "index_utils.h"
#include <fstream>
#include <cstdlib>
#include <string>
#include "autoptr.h"

using namespace std;

TextfileIndexerSource::TextfileIndexerSource(const string & fname)
	: filename(fname)
{
}

AutoPtr<istream>
TextfileIndexerSource::get_stream() const
{
    AutoPtr<istream> from(new ifstream(filename.c_str()));
    if (!*from)
       	throw Xapian::DatabaseOpeningError("Cannot open file " + filename +
				   " for indexing");
    return from;
}

void
TextfileIndexer::add_source(const IndexerSource & source)
{
    Assert(dest != NULL);
    AutoPtr<istream> from(source.get_stream());

    // Read lines, each paragraph is a document, split lines into words,
    // each word is a term.  Only used in the test suite.

    Xapian::Stem stemmer("english");

    while (*from) {
	string para;
	get_paragraph(*from, para);

	Xapian::Document document;
	document.set_data(para);
	Xapian::termcount position = 1;

	string::size_type spacepos;
	string word;
	while ((spacepos = para.find_first_not_of(" \t\n")) != string::npos) {
	    if (spacepos) para.erase(0, spacepos);
	    spacepos = para.find_first_of(" \t\n");
	    word = para.substr(0, spacepos);
	    select_characters(word, "");
	    lowercase_term(word);
	    word = stemmer.stem_word(word);
	    document.add_posting(word, position++);
	    para.erase(0, spacepos);
	}

	dest->add_document(document);
    }
}
