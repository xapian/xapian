/* query_parser.h
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

#ifndef _query_parser_h_
#define _query_parser_h_

#include "indexer.h"
#include <vector>
#include <map>
#include <algorithm>

class QueryTerm {
    public:
	termname tname;
	vector<termcount> positions;
	
	QueryTerm(termname tname_new) : tname(tname_new)  { return; }

	void add_posting(termcount pos) {
	    // FIXME - inefficient (speed)
	    // FIXME - inefficient (space, if we don't need the positional info)
	    positions.push_back(pos);
	    sort(positions.begin(), positions.end());
	}
	
	termcount get_wqf() {
	    return positions.size();
	}
};

class QueryParserSource : public virtual IndexerSource {
    private:
	string query;
    public:
	QueryParserSource(const string &);
	istream * get_stream() const;
};

class QueryParser : public virtual IndexerDestination {
    private:
	map<termname, termid> termidmap;
	vector<QueryTerm> termvec;

	Indexer *idx;
    public:
	QueryParser() : idx(NULL) { return; }

	void    set_indexer(Indexer *newidx) {idx = newidx;}

	vector<QueryTerm> parse_query(const string &);
	
	void  make_term(const termname &);
	docid make_doc(const docname &);
	void  make_posting(const termname &, docid, termcount);
};

#endif /* _query_parser_h_ */
