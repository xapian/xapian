/** @file queryparser_internal.h
 * @brief The non-lemon-generated parts of the QueryParser class.
 */
/* Copyright (C) 2005,2006,2007,2010,2011,2012,2013,2015,2016 Olly Betts
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

#ifndef XAPIAN_INCLUDED_QUERYPARSER_INTERNAL_H
#define XAPIAN_INCLUDED_QUERYPARSER_INTERNAL_H

#include "xapian/intrusive_ptr.h"
#include <xapian/database.h>
#include <xapian/query.h>
#include <xapian/queryparser.h>
#include <xapian/stem.h>

#include <list>
#include <map>

using namespace std;

class State;

typedef enum { NON_BOOLEAN, BOOLEAN, BOOLEAN_EXCLUSIVE } filter_type;

/** Information about how to handle a field prefix in the query string. */
struct FieldInfo {
    /// The type of this field.
    filter_type type;

    string grouping;

    /// Field prefix strings.
    list<string> prefixes;

    /// Field processors.  Currently only one is supported.
    list<Xapian::Internal::opt_intrusive_ptr<Xapian::FieldProcessor> > procs;

    FieldInfo(filter_type type_, const string& prefix,
	      const string& grouping_ = string())
	: type(type_), grouping(grouping_)
    {
	prefixes.push_back(prefix);
    }

    FieldInfo(filter_type type_, Xapian::FieldProcessor* proc,
	      const string& grouping_ = string())
	: type(type_), grouping(grouping_)
    {
	procs.push_back(proc);
    }
};

namespace Xapian {

class Utf8Iterator;

struct RangeProc {
    Xapian::Internal::opt_intrusive_ptr<RangeProcessor> proc;
    std::string grouping;
    bool default_grouping;

    RangeProc(RangeProcessor * range_proc, const std::string* grouping_)
	: proc(range_proc),
	  grouping(grouping_ ? *grouping_ : std::string()),
	  default_grouping(grouping_ == NULL) { }
};

class QueryParser::Internal : public Xapian::Internal::intrusive_base {
    friend class QueryParser;
    friend class ::State;
    Stem stemmer;
    stem_strategy stem_action;
    Xapian::Internal::opt_intrusive_ptr<const Stopper> stopper;
    Query::op default_op;
    const char * errmsg;
    Database db;
    list<string> stoplist;
    multimap<string, string> unstem;

    // Map "from" -> "A" ; "subject" -> "C" ; "newsgroups" -> "G" ;
    // "foobar" -> "XFOO". FIXME: it does more than this now!
    map<string, FieldInfo> field_map;

    list<RangeProc> rangeprocs;

    string corrected_query;

    Xapian::termcount max_wildcard_expansion;

    Xapian::termcount max_partial_expansion;

    int max_wildcard_type;

    int max_partial_type;

    void add_prefix(const string &field, const string &prefix);

    void add_prefix(const string &field, Xapian::FieldProcessor *proc);

    void add_boolean_prefix(const string &field, const string &prefix,
			    const string* grouping);

    void add_boolean_prefix(const string &field, Xapian::FieldProcessor *proc,
			    const string* grouping);

    std::string parse_term(Utf8Iterator &it, const Utf8Iterator &end,
			   bool cjk_ngram, bool &is_cjk_term,
			   bool &was_acronym);

  public:
    Internal() : stem_action(STEM_SOME), stopper(NULL),
	default_op(Query::OP_OR), errmsg(NULL),
	max_wildcard_expansion(0), max_partial_expansion(100),
	max_wildcard_type(Xapian::Query::WILDCARD_LIMIT_ERROR),
	max_partial_type(Xapian::Query::WILDCARD_LIMIT_MOST_FREQUENT) { }

    Query parse_query(const string & query_string, unsigned int flags, const string & default_prefix);
};

}

#endif // XAPIAN_INCLUDED_QUERYPARSER_INTERNAL_H
