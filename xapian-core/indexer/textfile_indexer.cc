/* textfile_indexer.cc
 */

#include "omassert.h"
#include "omerror.h"
#include "textfile_indexer.h"
#include "stem.h"
#include "index_utils.h"
#include <fstream>
#include <cstdlib>
#include <string>

TextfileIndexerSource::TextfileIndexerSource(const string &fname)
	: filename(fname)
{ return; }

istream *
TextfileIndexerSource::get_stream() const
{
    std::ifstream * from = new std::ifstream(filename.c_str());
    if(!*from) throw OmError("Cannot open file " + filename + " for indexing");
    return from;
};


void
TextfileIndexer::add_source(const IndexerSource &source)
{
    Assert(dest != NULL);
    istream *from = source.get_stream();


    // Read lines, each line is a document, split lines into words,
    // each word is a term
    // FIXME - This is just a temporary hack - we want to make a toolkit
    // of indexing "bits" and allow the user to specify how to put them
    // together.

    StemEn stemmer;

    while(*from) {
	string line;
	getline(*from, line);
	cout << "Line: `" << line << "'" << endl;
	
	docid did = dest->make_doc();
	termcount position = 1;

	string::size_type spacepos;
	termname word;
	while((spacepos = line.find_first_not_of(" \t")) != string::npos) {
	    if(spacepos) line = line.erase(0, spacepos);
	    spacepos = line.find_first_of(" \t");
	    word = line.substr(0, spacepos);
	    select_characters(word, "");
	    lowercase_term(word);
	    word = stemmer.stem_word(word);
	    cout << "Word: `" << word << "'" << endl;
	    dest->make_posting(dest->make_term(word), did, position++);
	    line = line.erase(0, spacepos);
	}
    }

    delete from;
}
