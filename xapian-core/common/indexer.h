/* indexer.h
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

#ifndef OM_HGUARD_INDEXER_H
#define OM_HGUARD_INDEXER_H

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
	/* Place term in database if not already there.
	 * FIXME - merge into make_posting()?
         */
	virtual void make_term(const om_termname &) = 0;

	/* Add a document to the database, before making the postings.  */
	virtual om_docid make_doc(const om_docname &) = 0;

	/* Post an occurrence of a term in a document to a database */
	virtual void make_posting(const om_termname & tname,
                                  om_docid did,
				  om_termpos tpos) = 0;
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

#endif /* OM_HGUARD_INDEXER_H */
