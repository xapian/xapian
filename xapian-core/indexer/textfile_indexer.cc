/* textfile_indexer.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#include "omassert.h"
#include "omerror.h"
#include "textfile_indexer.h"
#include "stemmer.h"
#include "index_utils.h"
#include <fstream>
#include <cstdlib>
#include <string>

TextfileIndexerSource::TextfileIndexerSource(const string & fname)
	: filename(fname)
{ return; }

istream *
TextfileIndexerSource::get_stream() const
{
    std::ifstream * from = new std::ifstream(filename.c_str());
    if(!*from) throw OmOpeningError("Cannot open file " + filename + " for indexing");
    return from;
};


void
TextfileIndexer::add_source(const IndexerSource & source)
{
    Assert(dest != NULL);
    istream *from = source.get_stream();

    // Read lines, each paragraph is a document, split lines into words,
    // each word is a term
    // FIXME - This is just a temporary hack - we want to make a toolkit
    // of indexing "bits" and allow the user to specify how to put them
    // together.

    Stemmer * stemmer = StemmerBuilder::create(STEMLANG_ENGLISH);

    while(*from) {
	string para;
	get_paragraph(*from, para);
	//get_a_line(*from, para);
	
	om_docid did = dest->make_doc(para);
	om_termcount position = 1;

	string::size_type spacepos;
	om_termname word;
	while((spacepos = para.find_first_not_of(" \t\n")) != string::npos) {
	    if(spacepos) para = para.erase(0, spacepos);
	    spacepos = para.find_first_of(" \t\n");
	    word = para.substr(0, spacepos);
	    select_characters(word, "");
	    lowercase_term(word);
	    word = stemmer->stem_word(word);
	    dest->make_term(word);
	    dest->make_posting(word, did, position++);
	    para = para.erase(0, spacepos);
	}
    }

    delete from;
}
