/* queryparser_internal.h: The non-lemon-generated parts of the QueryParser
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

#include <xapian/base.h>
#include <xapian/query.h>
#include <xapian/queryparser.h>
#include <xapian/stem.h>

using namespace Xapian;

#include <list>
#include <map>

using namespace std;

class State;

class QueryParser::Internal : public Xapian::Internal::RefCntBase {
    friend QueryParser;
    friend State;
    Stem stemmer;
    stem_strategy stem_action;
    Stopper * stopper;
    Query::op default_op;
    const char * errmsg;
    // Database db;
    list<string> termlist;
    list<string> stoplist;
    multimap<string, string> unstem;

    // Map "from" -> "A" ; "subject" -> "C" ; "newsgroups" -> "G" ;
    // "foobar" -> "XFOO"
    // bool is true if this is a boolean filter.
    map<string, pair<bool, string> > prefixes;
  public:
    Internal() : stem_action(STEM_NONE), stopper(NULL),
	default_op(Query::OP_OR), errmsg(NULL) { }
    Query parse_query(const string & query_string, unsigned int flags);
};
