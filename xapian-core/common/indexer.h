/* indexer.h
 *
 * ----START-LICENCE----
 * -----END-LICENCE-----
 */

#ifndef _indexer_h_
#define _indexer_h_

#include <string>
#include <iostream>

#include "omtypes.h"

// A source of data for indexing (eg, a file)
class IndexerSource {
    public:
	virtual istream * get_stream() const = 0;  // Get the stream of data
};

// Something which wants the indexed terms (eg, a database, or a query)
class IndexerDestination {
    public:
	virtual void make_term(const termname &) = 0;
	virtual docid make_doc(const docname &) = 0;
	virtual void make_posting(const termname &, docid, termcount) = 0;
};

// A way to generate terms from sources
class Indexer {
    protected:
	IndexerDestination * dest;
    public:
	Indexer() : dest(NULL) { return; }
	virtual ~Indexer() { return; }
	// Set the destination
	void set_destination(IndexerDestination *newdest) {dest = newdest;}

	// Generate terms from the source, and put them in the destination
	virtual void add_source(const IndexerSource &) = 0;
};

#endif /* _indexer_h_ */
