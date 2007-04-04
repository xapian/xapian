/* queryparser_internal.h: The non-lemon-generated parts of the QueryParser
 * class.
 *
 * Copyright (C) 2005,2006 Olly Betts
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

#include <xapian/base.h>
#include <xapian/database.h>
#include <xapian/query.h>
#include <xapian/queryparser.h>
#include <xapian/stem.h>

#include <list>
#include <map>

using namespace std;

class State;

struct BoolAndString {
    bool flag;
    string str;
    BoolAndString(bool f, const string &s) : flag(f), str(s) { }
};

class Xapian::QueryParser::Internal : public Xapian::Internal::RefCntBase {
    friend class Xapian::QueryParser;
    friend class ::State;
    Xapian::Stem stemmer;
    stem_strategy stem_action;
    const Xapian::Stopper * stopper;
    Xapian::Query::op default_op;
    const char * errmsg;
    Xapian::Database db;
    list<string> stoplist;
    multimap<string, string> unstem;

    // Map "from" -> "A" ; "subject" -> "C" ; "newsgroups" -> "G" ;
    // "foobar" -> "XFOO"
    // bool is true if this is a boolean filter.
    map<string, BoolAndString> prefixes;
  public:
    Internal() : stem_action(STEM_NONE), stopper(NULL),
	default_op(Xapian::Query::OP_OR), errmsg(NULL) { }
    Xapian::Query parse_query(const string & query_string, unsigned int flags);
};
