/* textfile_indexer.cc
 */

#include "omassert.h"
#include "omerror.h"
#include "textfile_indexer.h"
#include <fstream>
#include <cstdlib>

TextfileIndexerSource::TextfileIndexerSource(const string &fname)
	: filename(fname)
{ return; }

istream *
TextfileIndexerSource::get_stream() const
{
    std::ifstream * from = new std::ifstream(filename.c_str());
    if(!from) throw OmError("Cannot open file " + filename + " for indexing");
    return from;
};


void
TextfileIndexer::add_source(const IndexerSource &source)
{
    Assert(dest != NULL);
    istream *from = source.get_stream();

    termname word;
    docid did;
    termcount position;

    word = "thou";
    did = dest->make_doc();
    position = 1;

    dest->make_posting(dest->make_term(word), did, position++);
    dest->make_posting(dest->make_term("things"), did, position++);
    dest->make_posting(dest->make_term(word), did, position++);
    did = dest->make_doc();
    position = 1;

    dest->make_posting(dest->make_term("things"), did, position++);
    dest->make_posting(dest->make_term("junk"), did, position++);


    delete from;
}
