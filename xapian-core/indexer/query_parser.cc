/* query_parser.cc
 *
 * ----START-LICENCE----
 * Copyright 1999 Dialog Corporation
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

void
QueryParser::make_term(const termname &tname)
{ 
    map<termname,termid>::const_iterator p = termidmap.find(tname);

    if (p == termidmap.end()) {
	termvec.push_back(QueryTerm(tname));
	termidmap[tname] = termvec.size();
    }
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
