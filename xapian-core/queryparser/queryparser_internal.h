/** @file queryparser_internal.h
 * @brief The non-lemon-generated parts of the QueryParser class.
 */
/* Copyright (C) 2005,2006,2007,2010,2011,2012 Olly Betts
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

    /// Field prefix strings.
    list<string> prefixes;

    /// Field processors.  Currently only one is supported.
    list<Xapian::FieldProcessor*> procs;

    FieldInfo(filter_type type_, const string & prefix)
	: type(type_)
    {
	prefixes.push_back(prefix);
    }

    FieldInfo(filter_type type_, Xapian::FieldProcessor *proc)
	: type(type_)
    {
	procs.push_back(proc);
    }
};

namespace Xapian {

class Utf8Iterator;

class QueryParser::Internal : public Xapian::Internal::intrusive_base {
    friend class QueryParser;
    friend class ::State;
    Stem stemmer;
    stem_strategy stem_action;
    const Stopper * stopper;
    Query::op default_op;
    const char * errmsg;
    Database db;
    list<string> stoplist;
    multimap<string, string> unstem;

    // Map "from" -> "A" ; "subject" -> "C" ; "newsgroups" -> "G" ;
    // "foobar" -> "XFOO". FIXME: it does more than this now!
    map<string, FieldInfo> field_map;

    list<ValueRangeProcessor *> valrangeprocs;

    string corrected_query;

    Xapian::termcount max_wildcard_expansion;

    void add_prefix(const string &field, const string &prefix,
		    filter_type type);

    void add_prefix(const string &field, Xapian::FieldProcessor *proc,
		    filter_type type);

    std::string parse_term(Utf8Iterator &it, const Utf8Iterator &end,
			   bool cjk_ngram, bool &is_cjk_term,
			   bool &was_acronym);

  public:
    Internal() : stem_action(STEM_SOME), stopper(NULL),
	default_op(Query::OP_OR), errmsg(NULL), max_wildcard_expansion(0) { }

    Query parse_query(const string & query_string, unsigned int flags, const string & default_prefix);

    void set_fieldproc();
};

}

#endif // XAPIAN_INCLUDED_QUERYPARSER_INTERNAL_H
