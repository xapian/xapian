/* textfile_indexer.cc
 */

#include "omerror.h"
#include "textfile_indexer.h"
#include <fstream>
#include <cstdlib>

TextfileIndexerSource::TextfileIndexerSource(string &fname)
	: filename(fname)
{ return; }

istream *
TextfileIndexerSource::get_stream() const {
    std::ifstream * from = new std::ifstream(filename.c_str());
    if(!from) throw OmError("Cannot open file " + filename + " for indexing");
    return from;
};

