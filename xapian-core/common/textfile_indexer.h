/* textfile_indexer.h
 */

#ifndef _textfile_indexer_h_
#define _textfile_indexer_h_

#include "indexer.h"

class TextfileIndexerSource {
    private:
	string filename;
    public:
	TextfileIndexerSource(string &);
	istream * get_stream() const;
};

class TextfileIndexer {
    private:
	IndexerDestination * dest;
    public:
	TextfileIndexer() : dest(NULL) { return; }
	void set_destination(IndexerDestination *newdest) {dest = newdest;}
	void add_source(IndexerSource &);
};

#endif /* _textfile_indexer_h_ */
