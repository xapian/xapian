/* textfile_indexer.h
 */

#ifndef _textfile_indexer_h_
#define _textfile_indexer_h_

#include "indexer.h"

class TextfileIndexerSource : public virtual IndexerSource {
    private:
	string filename;
    public:
	TextfileIndexerSource(const string &);
	istream * get_stream() const;
};

class TextfileIndexer : public virtual Indexer {
    private:
	IndexerDestination * dest;
    public:
	TextfileIndexer() : dest(NULL) { return; }
	void set_destination(IndexerDestination *newdest) {dest = newdest;}
	void add_source(const IndexerSource &);
};

#endif /* _textfile_indexer_h_ */
