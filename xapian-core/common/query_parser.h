/* query_parser.h
 */

#ifndef _query_parser_h_
#define _query_parser_h_

#include "indexer.h"
#include <vector>
#include <map>

class QueryTerm {
    public:
	termname tname;
	vector<termcount> positions;
	
	QueryTerm(termname tname_new) : tname(tname_new)  { return; }

	void add_posting(termcount pos) {
	    positions.push_back(pos);
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
	
	termid  make_term(const termname &);
	docid   make_doc(const docname &);
	void    make_posting(termid, docid, termcount);
};

#endif /* _query_parser_h_ */
