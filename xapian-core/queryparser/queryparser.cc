/* queryparser.cc: The non-lemon-generated parts of the QueryParser
 * class.
 *
 * Copyright (C) 2005 Olly Betts
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
 */

#include <config.h>

#include "queryparser_internal.h"
#include <xapian/termiterator.h>
#include "vectortermlist.h"

using namespace Xapian;

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
QueryParser::set_stemming_options(stem_strategy strategy)
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

static const unsigned default_flags =
    QueryParser::FLAG_PHRASE |
    QueryParser::FLAG_BOOLEAN |
    QueryParser::FLAG_LOVEHATE;

Query
QueryParser::parse_query(const string &query_string)
{
    internal->termlist.clear();
    internal->unstem.clear();
    internal->errmsg = NULL;

    if (query_string.empty()) return Query();

    Query result = internal->parse_query(query_string, default_flags);
    if (internal->errmsg && strcmp(internal->errmsg, "parse error") == 0) {
	result = internal->parse_query(query_string, 0);
    }

    if (internal->errmsg) throw internal->errmsg;
    return result;
}

void
QueryParser::add_prefix(const std::string &field, const std::string &prefix)
{
    internal->prefixes.insert(make_pair(field, make_pair(false, prefix)));
}

void
QueryParser::add_boolean_prefix(const std::string &field,
				const std::string &prefix)
{
    internal->prefixes.insert(make_pair(field, make_pair(true, prefix)));
}

TermIterator
QueryParser::termlist_begin() const
{
    list<std::string> & tl = internal->termlist;
    return TermIterator(new VectorTermList(tl.begin(), tl.end()));
}

TermIterator
QueryParser::termlist_end() const
{
    return TermIterator(NULL);
}

TermIterator
QueryParser::stoplist_begin() const
{
    list<std::string> & sl = internal->stoplist;
    return TermIterator(new VectorTermList(sl.begin(), sl.end()));
}

TermIterator
QueryParser::stoplist_end() const
{
    return TermIterator(NULL);
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

TermIterator
QueryParser::unstem_end(const std::string &/*term*/) const
{
    return TermIterator(NULL);
}

std::string
QueryParser::get_description() const
{
    DEBUGCALL(INTRO, std::string, "Xapian::QueryParser::get_description", "");
    // FIXME : describe better!
    RETURN("Xapian::QueryParser()");
}
