/* query_parser.cc
 */

#include "omassert.h"
#include "omerror.h"
#include "query_parser.h"
#include <cstdlib>
#include <string>

// FIXME - strstream isn't in the C++ standard - we want to use stringstreams
// (ie, <sstream>), but they're not in any compiler that I've found so far...
// (eg: gcc version egcs-2.91.66)
#include <strstream.h>

QueryParserSource::QueryParserSource(const string &q)
	: query(q)
{ return; }

istream *
QueryParserSource::get_stream() const
{
    istrstream * from = new istrstream(query.c_str(), query.size());
    return from;
};


vector<QueryTerm>
QueryParser::parse_query(const string &query)
{
    Assert(idx != NULL);
    termidmap.clear();
    termvec.clear();

    QueryParserSource source(query);

    idx->set_destination(this);
    idx->add_source(source);

    return termvec;
}

termid
QueryParser::make_term(const termname &tname)
{ 
    map<termname,termid>::const_iterator p = termidmap.find(tname);

    termid tid = 0;
    if (p == termidmap.end()) {
	tid = termvec.size() + 1;
	termvec.push_back(QueryTerm(tname));
	termidmap[tname] = tid;
    } else {
	tid = (*p).second;
    }

    return tid;
}

docid
QueryParser::make_doc(const docname &dname)
{ return 1; }

void
QueryParser::make_posting(const termname &tname, docid did, termcount tpos)
{
    map<termname,termid>::const_iterator p = termidmap.find(tname);
    Assert(p != termidmap.end());

    termid tid = (*p).second;
    Assert(tid > 0 && tid <= termvec.size());

    termvec[tid - 1].add_posting(tpos);
}
