/* queryparser_internal.h: The non-lemon-generated parts of the QueryParser
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

#ifndef XAPIAN_INCLUDED_QUERYPARSER_INTERNAL_H
#define XAPIAN_INCLUDED_QUERYPARSER_INTERNAL_H

#include <xapian/base.h>
#include <xapian/database.h>
#include <xapian/query.h>
#include <xapian/queryparser.h>
#include <xapian/stem.h>

#include <list>
#include <map>

using namespace std;

class State;

/** Information about how to handle a prefix in the query string.
 */
struct PrefixInfo {
    typedef enum {
	/** Handle the text following a prefix as free text. */
	FREE_TEXT,

	/** Handle the text following a prefix as a boolean filter. */
	BOOL_FILTER
    } prefix_type;

    /** Type of handling for the prefix; free text, or boolean.
     */
    PrefixInfo::prefix_type type;

    /** Prefix string.
     */
    string prefix;

    PrefixInfo(PrefixInfo::prefix_type t, const string &s)
	    : type(t), prefix(s)
    {}
};

/** A list of ways to handle a given prefix.
 *
 *  We define this as an explicit type, rather than just using list<PrefixInfo>
 *  directly, partly to keep symbol names shorter, and partly to add the
 *  convenience constructor which takes an initial item.
 */
struct PrefixInfoList {
    list<PrefixInfo> items;

    /** Construct an empty PrefixInfoList. */
    PrefixInfoList() : items() {}

    /** Construct a new PrefixInfoList with an initial item. */
    PrefixInfoList(PrefixInfo item) : items() {
	items.push_back(item);
    }
};

namespace Xapian {

class Utf8Iterator;

class QueryParser::Internal : public Xapian::Internal::RefCntBase {
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
    // "foobar" -> "XFOO"
    map<string, PrefixInfoList> prefixes;

    list<ValueRangeProcessor *> valrangeprocs;

    string corrected_query;

    std::string parse_term(Utf8Iterator &it, const Utf8Iterator &end,
			   bool &was_acronym);

  public:
    Internal() : stem_action(STEM_NONE), stopper(NULL),
	default_op(Query::OP_OR), errmsg(NULL) { }
    Query parse_query(const string & query_string, unsigned int flags, const string & default_prefix);
};

}

#endif // XAPIAN_INCLUDED_QUERYPARSER_INTERNAL_H
