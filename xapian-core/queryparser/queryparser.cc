/* queryparser.cc: The non-lemon-generated parts of the QueryParser
 * class.
 *
 * Copyright (C) 2005,2006,2007 Olly Betts
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

#include <xapian/queryparser.h>
#include <xapian/termiterator.h>

#include "queryparser_internal.h"
#include "vectortermlist.h"

using namespace Xapian;

// Default implementation in case the user hasn't implemented it.
std::string
Stopper::get_description() const
{
    DEBUGCALL(INTRO, std::string, "Xapian::Stopper::get_description", "");
    return "Xapian::Stopper subclass";
}

std::string
SimpleStopper::get_description() const
{
    DEBUGCALL(INTRO, std::string, "Xapian::SimpleStopper::get_description", "");
    std::string desc("Xapian::SimpleStopper(");
    std::set<string>::const_iterator i;
    for (i = stop_words.begin(); i != stop_words.end(); ++i) {
	if (i != stop_words.begin()) desc += ' ';
	desc += *i;
    }
    desc += ')';
    return desc;
}

QueryParser::QueryParser(const QueryParser & o) : internal(o.internal) { }

QueryParser &
QueryParser::operator=(const QueryParser & o)
{
    internal = o.internal;
    return *this;
}

QueryParser::QueryParser()
{
    internal = new QueryParser::Internal();
}

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

Query
QueryParser::parse_query(const string &query_string, unsigned flags,
			 const string &default_prefix)
{
    internal->unstem.clear();
    internal->errmsg = NULL;

    if (query_string.empty()) return Query();

    Query result = internal->parse_query(query_string, flags, default_prefix);
    if (internal->errmsg && strcmp(internal->errmsg, "parse error") == 0) {
	result = internal->parse_query(query_string, 0, default_prefix);
    }

    if (internal->errmsg) throw Xapian::QueryParserError(internal->errmsg);
    return result;
}

void
QueryParser::add_prefix(const std::string &field, const std::string &prefix)
{
    internal->prefixes.insert(make_pair(field, BoolAndString(false, prefix)));
}

void
QueryParser::add_boolean_prefix(const std::string &field,
				const std::string &prefix)
{
    internal->prefixes.insert(make_pair(field, BoolAndString(true, prefix)));
}

TermIterator
QueryParser::stoplist_begin() const
{
    list<std::string> & sl = internal->stoplist;
    return TermIterator(new VectorTermList(sl.begin(), sl.end()));
}

TermIterator
QueryParser::unstem_begin(const std::string &term) const
{
    pair<multimap<std::string, std::string>::iterator,
	 multimap<std::string, std::string>::iterator> range;
    range = internal->unstem.equal_range(term);
    list<std::string> l;
    multimap<std::string, std::string>::iterator & i = range.first;
    while (i != range.second) {
	l.push_back(i->second);
	++i;
    }
    return TermIterator(new VectorTermList(l.begin(), l.end()));
}

std::string
QueryParser::get_description() const
{
    DEBUGCALL(INTRO, std::string, "Xapian::QueryParser::get_description", "");
    // FIXME : describe better!
    RETURN("Xapian::QueryParser()");
}
