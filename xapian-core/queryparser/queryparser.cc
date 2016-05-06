/* queryparser.cc: The non-lemon-generated parts of the QueryParser
 * class.
 *
 * Copyright (C) 2005,2006,2007,2008,2010,2011,2012,2013,2015,2016 Olly Betts
 * Copyright (C) 2010 Adam Sjøgren
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include "xapian/error.h"
#include <xapian/queryparser.h>
#include <xapian/termiterator.h>

#include "api/vectortermlist.h"
#include "omassert.h"
#include "queryparser_internal.h"

#include <cstring>

using namespace Xapian;

using namespace std;

// Default implementation in case the user hasn't implemented it.
string
Stopper::get_description() const
{
    return "Xapian::Stopper subclass";
}

string
SimpleStopper::get_description() const
{
    string desc("Xapian::SimpleStopper(");
    set<string>::const_iterator i;
    for (i = stop_words.begin(); i != stop_words.end(); ++i) {
	if (i != stop_words.begin()) desc += ' ';
	desc += *i;
    }
    desc += ')';
    return desc;
}

RangeProcessor::~RangeProcessor() { }

ValueRangeProcessor::~ValueRangeProcessor() { }

FieldProcessor::~FieldProcessor() { }

QueryParser::QueryParser(const QueryParser & o) : internal(o.internal) { }

QueryParser &
QueryParser::operator=(const QueryParser & o)
{
    internal = o.internal;
    return *this;
}

QueryParser::QueryParser() : internal(new QueryParser::Internal) { }

QueryParser::~QueryParser() { }

void
QueryParser::set_stemmer(const Xapian::Stem & stemmer)
{
    internal->stemmer = stemmer;
}

void
QueryParser::set_stemming_strategy(stem_strategy strategy)
{
    internal->stem_action = strategy;
}

void
QueryParser::set_stopper(const Stopper * stopper)
{
    internal->stopper = stopper;
}

void
QueryParser::set_default_op(Query::op default_op)
{
    switch (default_op) {
	case Query::OP_AND:
	case Query::OP_OR:
	case Query::OP_NEAR:
	case Query::OP_PHRASE:
	case Query::OP_ELITE_SET:
	case Query::OP_SYNONYM:
	case Query::OP_MAX:
	    // These are OK.
	    break;
	default:
	    throw Xapian::InvalidArgumentError(
		    "QueryParser::set_default_op() only accepts "
		    "OP_AND"
		    ", "
		    "OP_OR"
		    ", "
		    "OP_NEAR"
		    ", "
		    "OP_PHRASE"
		    ", "
		    "OP_ELITE_SET"
		    ", "
		    "OP_SYNONYM"
		    " or "
		    "OP_MAX");
    }
    internal->default_op = default_op;
}

Query::op
QueryParser::get_default_op() const
{
    return internal->default_op;
}

void
QueryParser::set_database(const Database &db) {
    internal->db = db;
}

void
QueryParser::set_max_expansion(Xapian::termcount max_expansion,
			       int max_type,
			       unsigned flags)
{
    if (flags & FLAG_WILDCARD) {
	internal->max_wildcard_expansion = max_expansion;
	internal->max_wildcard_type = max_type;
    }
    if (flags & FLAG_PARTIAL) {
	internal->max_partial_expansion = max_expansion;
	internal->max_partial_type = max_type;
    }
}

Query
QueryParser::parse_query(const string &query_string, unsigned flags,
			 const string &default_prefix)
{
    internal->stoplist.clear();
    internal->unstem.clear();
    internal->errmsg = NULL;

    if (query_string.empty()) return Query();

    Query result = internal->parse_query(query_string, flags, default_prefix);
    if (internal->errmsg && strcmp(internal->errmsg, "parse error") == 0) {
	flags &= FLAG_CJK_NGRAM;
	result = internal->parse_query(query_string, flags, default_prefix);
    }

    if (internal->errmsg) throw Xapian::QueryParserError(internal->errmsg);
    return result;
}

void
QueryParser::add_prefix(const string &field, const string &prefix)
{
    Assert(internal.get());
    internal->add_prefix(field, prefix);
}

void
QueryParser::add_prefix(const string &field, Xapian::FieldProcessor * proc)
{
    Assert(internal.get());
    internal->add_prefix(field, proc);
}

void
QueryParser::add_boolean_prefix(const string &field, const string &prefix,
				const string* grouping)
{
    Assert(internal.get());
    internal->add_boolean_prefix(field, prefix, grouping);
}

void
QueryParser::add_boolean_prefix(const string &field,
				Xapian::FieldProcessor * proc,
				const string* grouping)
{
    Assert(internal.get());
    internal->add_boolean_prefix(field, proc, grouping);
}

TermIterator
QueryParser::stoplist_begin() const
{
    const list<string> & sl = internal->stoplist;
    return TermIterator(new VectorTermList(sl.begin(), sl.end()));
}

TermIterator
QueryParser::unstem_begin(const string &term) const
{
    pair<multimap<string, string>::iterator,
	 multimap<string, string>::iterator> range;
    range = internal->unstem.equal_range(term);
    list<string> l;
    multimap<string, string>::iterator & i = range.first;
    while (i != range.second) {
	l.push_back(i->second);
	++i;
    }
    return TermIterator(new VectorTermList(l.begin(), l.end()));
}

void
QueryParser::add_rangeprocessor(Xapian::RangeProcessor * range_proc,
				const std::string* grouping)
{
    Assert(internal.get());
    internal->rangeprocs.push_back(RangeProc(range_proc, grouping));
}

string
QueryParser::get_corrected_query_string() const
{
    return internal->corrected_query;
}

string
QueryParser::get_description() const
{
    // FIXME : describe better!
    return "Xapian::QueryParser()";
}
