/* omqueryinternal.cc: Internals of query interface
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004 Olly Betts
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

#include <config.h>
#include "omdebug.h"
#include "omqueryinternal.h"
#include "utils.h"
#include "netutils.h"

#include <xapian/error.h>
#include <xapian/enquire.h>
#include <xapian/termiterator.h>
#include "vectortermlist.h"

#include <vector>
#include <set>
#include <algorithm>
#include <math.h>
#include <limits.h>

using namespace std;

// Properties for query operations.

static unsigned int
get_min_subqs(Xapian::Query::Internal::op_t op)
{
    switch (op) {
	case Xapian::Query::Internal::OP_LEAF:
	case Xapian::Query::OP_AND:
	case Xapian::Query::OP_OR:
	case Xapian::Query::OP_XOR:
	case Xapian::Query::OP_NEAR:
	case Xapian::Query::OP_PHRASE:
	case Xapian::Query::OP_ELITE_SET:
	    return 0;
	case Xapian::Query::OP_WEIGHT_CUTOFF:
	    return 1;
	case Xapian::Query::OP_FILTER:
	case Xapian::Query::OP_AND_MAYBE:
	case Xapian::Query::OP_AND_NOT:
	    return 2;
	default:
	    Assert(false);
	    throw Xapian::InvalidOperationError("get_min_subqs called with invalid operator type");
    }
}

static unsigned int
get_max_subqs(Xapian::Query::Internal::op_t op)
{
    switch (op) {
	case Xapian::Query::Internal::OP_LEAF:
	    return 0;
	case Xapian::Query::OP_WEIGHT_CUTOFF:
	    return 1;
	case Xapian::Query::OP_FILTER:
	case Xapian::Query::OP_AND_MAYBE:
	case Xapian::Query::OP_AND_NOT:
	    return 2;
	case Xapian::Query::OP_AND:
	case Xapian::Query::OP_OR:
	case Xapian::Query::OP_XOR:
	case Xapian::Query::OP_NEAR:
	case Xapian::Query::OP_PHRASE:
	case Xapian::Query::OP_ELITE_SET:
	    return UINT_MAX;
	default:
	    Assert(false);
	    throw Xapian::InvalidOperationError("get_max_subqs called with invalid operator type");
    }
}

static Xapian::termpos
get_min_window(Xapian::Query::Internal::op_t op)
{
    switch (op) {
	case Xapian::Query::OP_NEAR:
	case Xapian::Query::OP_PHRASE:
	    return 1;
	default:
	    return 0;
    }
}

static bool
is_leaf(Xapian::Query::Internal::op_t op)
{
    return (op == Xapian::Query::Internal::OP_LEAF);
}

static bool
can_replace_by_single_subq(Xapian::Query::Internal::op_t op)
{
    return (op == Xapian::Query::OP_AND ||
	    op == Xapian::Query::OP_OR ||
	    op == Xapian::Query::OP_XOR ||
	    op == Xapian::Query::OP_NEAR ||
	    op == Xapian::Query::OP_PHRASE ||
// Can't replace OP_ELITE_SET by a single subq since then set_elite_set_size will barf...
//	    op == Xapian::Query::OP_ELITE_SET ||
	    op == Xapian::Query::OP_FILTER ||
	    op == Xapian::Query::OP_AND_MAYBE ||
	    op == Xapian::Query::OP_AND_NOT);
}

static bool
can_reorder(Xapian::Query::Internal::op_t op)
{
    return (op == Xapian::Query::OP_OR ||
	    op == Xapian::Query::OP_AND ||
	    op == Xapian::Query::OP_XOR);
}

static bool
can_flatten(Xapian::Query::Internal::op_t op)
{
    return (op == Xapian::Query::OP_NEAR || op == Xapian::Query::OP_PHRASE);
}

// Methods for Xapian::Query::Internal

/** serialising method, for network matches.
 *
 *  The format is designed to be relatively easy
 *  to parse, as well as encodable in one line of text.
 *
 *  A single-term query becomes `[<encodedtname> @<termpos>#<wqf>'
 *  where:
 *  	<wqf> is the decimal within query frequency (1 if omitted),
 *  	<termpos> is the decimal term position (index of term if omitted).
 *
 *  A compound query becomes `(<subqueries><op>', where:
 *  	<subqueries> is the list of subqueries
 *  	<op> is one of: &|%+-^
 *  also ~N "N >F *N (N unsigned int; F floating point)
 * 
 *  If querylen != sum(wqf) we append `=len' (at present we always do this
 *  for compound queries as it's simpler than working out what sum(wqf) would
 *  be - FIXME).
 */
string
Xapian::Query::Internal::serialise() const
{
    Xapian::termpos curpos = 1;
    Xapian::termcount len = 0;
    string result;

    if (op == Xapian::Query::Internal::OP_LEAF) {
	result += "[" + encode_tname(tname);
	result += ' ';
       	if (term_pos != curpos) result += '@' + om_tostring(term_pos);
	if (wqf != 1) result += '#' + om_tostring(wqf);
	++curpos;
	len += wqf;
	if (qlen != len) result += '=' + om_tostring(qlen);
    } else {
	result += "(";
	for (subquery_list::const_iterator i = subqs.begin();
	     i != subqs.end();
	     ++i) {
	    result += (*i)->serialise();
	}
	switch (op) {
	    case Xapian::Query::Internal::OP_LEAF:
		Assert(false);
		break;
	    case Xapian::Query::OP_AND:
		result += "&";
		break;
	    case Xapian::Query::OP_OR:
		result += "|";
		break;
	    case Xapian::Query::OP_FILTER:
		result += "%";
		break;
	    case Xapian::Query::OP_AND_MAYBE:
		result += "+";
		break;
	    case Xapian::Query::OP_AND_NOT:
		result += "-";
		break;
	    case Xapian::Query::OP_XOR:
		result += "^";
		break;
	    case Xapian::Query::OP_NEAR:
		result += "~" + om_tostring(window);
		break;
	    case Xapian::Query::OP_PHRASE:
		result += "\"" + om_tostring(window);
		break;
	    case Xapian::Query::OP_WEIGHT_CUTOFF:
		result += ">" + om_tostring(cutoff);
		break;
	    case Xapian::Query::OP_ELITE_SET:
		result += "*" + om_tostring(elite_set_size);
		break;
	}
	/*if (qlen != len)*/ result += '=' + om_tostring(qlen);
    }
    return result;
}

string
Xapian::Query::Internal::get_op_name(Xapian::Query::Internal::op_t op)
{
    string name;
    switch (op) {
	case Xapian::Query::Internal::OP_LEAF:  name = "LEAF"; break;
	case Xapian::Query::OP_AND:             name = "AND"; break;
	case Xapian::Query::OP_OR:              name = "OR"; break;
	case Xapian::Query::OP_FILTER:          name = "FILTER"; break;
	case Xapian::Query::OP_AND_MAYBE:       name = "AND_MAYBE"; break;
	case Xapian::Query::OP_AND_NOT:         name = "AND_NOT"; break;
	case Xapian::Query::OP_XOR:             name = "XOR"; break;
	case Xapian::Query::OP_NEAR:            name = "NEAR"; break;
	case Xapian::Query::OP_PHRASE:          name = "PHRASE"; break;
	case Xapian::Query::OP_WEIGHT_CUTOFF:   name = "WEIGHT_CUTOFF"; break;
	case Xapian::Query::OP_ELITE_SET:       name = "ELITE_SET"; break;
    }
    return name;
}

// Introspection
string
Xapian::Query::Internal::get_description() const
{
    string opstr;

    if (is_leaf(op)) {
	if (term_pos != 0) {
	    opstr += "pos=" + om_tostring(term_pos);
	}
	if (wqf != 1) {
	    if (opstr.size() != 0) opstr += ",";
	    opstr += "wqf=" + om_tostring(wqf);
	}
	if (opstr.size() != 0) opstr = ":(" + opstr + ")";
	return tname + opstr;
    } else {
	opstr = " " + get_op_name(op) + " ";
	if (op == Xapian::Query::OP_NEAR || op == Xapian::Query::OP_PHRASE)
	    opstr += om_tostring(window) + " ";
	if (op == Xapian::Query::OP_WEIGHT_CUTOFF)
	    opstr += om_tostring(cutoff) + " ";
	if (op == Xapian::Query::OP_ELITE_SET)
	    opstr += om_tostring(elite_set_size) + " ";
    }
    string description;
    subquery_list::const_iterator i;
    for (i = subqs.begin(); i != subqs.end(); i++) {
	if (!description.empty()) description += opstr;
	description += (**i).get_description();
    }
    return "(" + description + ")";
}

void
Xapian::Query::Internal::set_window(Xapian::termpos window_)
{
    window = window_;
}

void
Xapian::Query::Internal::set_cutoff(double cutoff_)
{
    if (op != Xapian::Query::OP_WEIGHT_CUTOFF)
	throw Xapian::InvalidOperationError("Can only set cutoff parameter for weight or percentage cutoff operators.");
    cutoff = cutoff_;
}

void
Xapian::Query::Internal::set_elite_set_size(Xapian::termcount size_)
{
    if (op != Xapian::Query::OP_ELITE_SET)
	throw Xapian::InvalidOperationError("Can only set elite set size for elite set operator.");
    if (size_ == 0)
	throw Xapian::InvalidArgumentError("Elite set size may not be zero.");
    elite_set_size = size_;
}

Xapian::termcount
Xapian::Query::Internal::set_length(Xapian::termcount qlen_)
{
    Xapian::termcount oldqlen = qlen;
    qlen = qlen_;
    return oldqlen;
}

/** Private function used to implement get_terms() */
void
Xapian::Query::Internal::accumulate_terms(
			vector<pair<string, Xapian::termpos> > &terms) const
{
    if (is_leaf(op)) {
        // We're a leaf, so just return our term.
        terms.push_back(make_pair(tname, term_pos));
    } else {
    	subquery_list::const_iterator end = subqs.end();
        // not a leaf, concatenate results from all subqueries
	for (subquery_list::const_iterator i = subqs.begin(); i != end; ++i) {
 	    (*i)->accumulate_terms(terms);
	}
    }
}

struct LessByTermpos {
    typedef const pair<string, Xapian::termpos> argtype;
    bool operator()(argtype &left, argtype &right) {
	if (left.second != right.second) {
	    return left.second < right.second;
	} else {
	    return left.first < right.first;
	}
    }
};

Xapian::TermIterator
Xapian::Query::Internal::get_terms() const
{
    vector<pair<string, Xapian::termpos> > terms;
    accumulate_terms(terms);

    sort(terms.begin(), terms.end(), LessByTermpos());

    // remove adjacent duplicates, and return an iterator pointing
    // to just after the last unique element
    vector<pair<string, Xapian::termpos> >::iterator newlast =
	    	unique(terms.begin(), terms.end());
    // and remove the rest...  (See Stroustrup 18.6.3)
    terms.erase(newlast, terms.end());

    vector<string> result;
    vector<pair<string, Xapian::termpos> >::const_iterator i;
    for (i = terms.begin(); i != terms.end(); ++i) {
	result.push_back(i->first);
    }

    return Xapian::TermIterator(new VectorTermList(result.begin(),
						   result.end()));
}

// Methods 

class QUnserial {
  private:
    const char *p;
    Xapian::termpos curpos;
    Xapian::termpos len;
 
    Xapian::Query::Internal * readquery();
    Xapian::Query::Internal * readcompound();
    
  public:
    QUnserial(const char *p_) : p(p_), curpos(1), len(0) { }
    Xapian::Query::Internal * decode();
};

Xapian::Query::Internal *
QUnserial::decode() {
    DEBUGLINE(UNKNOWN, "QUnserial::decode(" << p << ")");
    Xapian::Query::Internal * qint = readquery();
    if (*p == '=') {
	char *tmp; // avoid compiler warning
	qint->set_length(Xapian::termcount(strtol(p + 1, &tmp, 10)));
	p = tmp;
    } else {
	qint->set_length(len);
    }
    DEBUGLINE(UNKNOWN, "Remainder of query (should be none) is `" << p << "'");
    Assert(*p == '\0');
    return qint;
}

Xapian::Query::Internal *
QUnserial::readquery() {
    switch (*p++) {
	case '[': {
	    const char *q = strchr(p, ' ');
	    if (!q) q = p + strlen(p);
	    string tname = decode_tname(string(p, q - p));
	    Xapian::termpos term_pos = curpos;
	    Xapian::termcount wqf = 1;
	    p = q;
	    if (*p == ' ') ++p;
	    if (*p == '@') {
		char *tmp; // avoid compiler warning
		term_pos = strtol(p + 1, &tmp, 10);
		p = tmp;
	    }
	    if (*p == '#') {
		char *tmp; // avoid compiler warning
		wqf = strtol(p + 1, &tmp, 10);
		p = tmp;
	    }
	    ++curpos;
	    len += wqf;
	    return new Xapian::Query::Internal(tname, wqf, term_pos);
	}
	case '(':
	    return readcompound();
	default:
	    DEBUGLINE(UNKNOWN, "Can't parse remainder `" << p - 1 << "'");
	    throw Xapian::InvalidArgumentError("Invalid query string");
    }
}

static Xapian::Query::Internal *
qint_from_vector(Xapian::Query::op op, vector<Xapian::Query::Internal *> & vec) {
    Xapian::Query::Internal * qint = new Xapian::Query::Internal(op);
    vector<Xapian::Query::Internal *>::const_iterator i;
    for (i = vec.begin(); i != vec.end(); i++)
	qint->add_subquery(**i);
    qint->end_construction();
    return qint;
}

Xapian::Query::Internal *
QUnserial::readcompound() {
    vector<Xapian::Query::Internal *> subqs;
    while (true) {
	switch (*p++) {
	    case '[':
		--p;
		subqs.push_back(readquery());
		break;
	    case '(': {
		Xapian::Query::Internal * qint = readcompound();
		if (*p == '=') {
		    char *tmp; // avoid compiler warning
		    qint->set_length(Xapian::termcount(strtol(p + 1, &tmp, 10)));
		    p = tmp;
		}
		subqs.push_back(qint);
		break;
	    }
	    case '&':
		return qint_from_vector(Xapian::Query::OP_AND, subqs);
	    case '|':
		return qint_from_vector(Xapian::Query::OP_OR, subqs);
	    case '%':
		return qint_from_vector(Xapian::Query::OP_FILTER, subqs);
	    case '^':
		return qint_from_vector(Xapian::Query::OP_XOR, subqs);
	    case '+':
		return qint_from_vector(Xapian::Query::OP_AND_MAYBE, subqs);
	    case '-':
		return qint_from_vector(Xapian::Query::OP_AND_NOT, subqs);
	    case '~': {
		Xapian::Query::Internal * qint;
		qint = qint_from_vector(Xapian::Query::OP_NEAR, subqs);
		char *tmp; // avoid compiler warning
		qint->set_window(Xapian::termpos(strtol(p, &tmp, 10)));
		p = tmp;
		return qint;
	    }
	    case '"': {
		Xapian::Query::Internal * qint;
		qint = qint_from_vector(Xapian::Query::OP_PHRASE, subqs);
		char *tmp; // avoid compiler warning
		qint->set_window(Xapian::termpos(strtol(p, &tmp, 10)));
		p = tmp;
		return qint;
	    }
	    case '>': {
		Xapian::Query::Internal * qint;
		qint = new Xapian::Query::Internal(Xapian::Query::OP_WEIGHT_CUTOFF);
		Assert(subqs.size() == 1);
		qint->add_subquery(*subqs[0]);
		qint->end_construction();
		char *tmp; // avoid compiler warning
		qint->set_cutoff(strtod(p, &tmp));
		p = tmp;
		return qint;
	    }
	    case '*': {
		Xapian::Query::Internal * qint;
		qint = qint_from_vector(Xapian::Query::OP_ELITE_SET, subqs);
		char *tmp; // avoid compiler warning
		qint->set_elite_set_size(Xapian::termcount(strtol(p, &tmp, 10)));
		p = tmp;
		return qint;
	    }
	    default:
		DEBUGLINE(UNKNOWN, "Can't parse remainder `" << p - 1 << "'");
		throw Xapian::InvalidArgumentError("Invalid query string");
	}
    }
}

Xapian::Query::Internal *
Xapian::Query::Internal::unserialise(const string &s)
{
    Assert(s.length() > 1);
    QUnserial u(s.c_str());
    Xapian::Query::Internal * qint = u.decode();
    AssertEq(s, qint->serialise());
    return qint;
}

/** swap the contents of this with another Xapian::Query::Internal,
 *  in a way which is guaranteed not to throw.  This is
 *  used with the assignment operator to make it exception
 *  safe.
 *  It's important to adjust swap with any addition of
 *  member variables!
 */
void
Xapian::Query::Internal::swap(Xapian::Query::Internal &other)
{
    std::swap(op, other.op);
    subqs.swap(other.subqs);
    std::swap(qlen, other.qlen);
    std::swap(window, other.window);
    std::swap(cutoff, other.cutoff);
    std::swap(elite_set_size, other.elite_set_size);
    std::swap(tname, other.tname);
    std::swap(term_pos, other.term_pos);
    std::swap(wqf, other.wqf);
}

Xapian::Query::Internal::Internal(const Xapian::Query::Internal &copyme)
	: Xapian::Internal::RefCntBase(),
	  op(copyme.op),
	  subqs(),
	  qlen(copyme.qlen),
	  window(copyme.window),
	  cutoff(copyme.cutoff),
	  elite_set_size(copyme.elite_set_size),
	  tname(copyme.tname),
	  term_pos(copyme.term_pos),
	  wqf(copyme.wqf)
{
    for (subquery_list::const_iterator i = copyme.subqs.begin();
	 i != copyme.subqs.end();
	 ++i) {
	subqs.push_back(new Xapian::Query::Internal(**i));
    }
}

//////////////////////////////////////////
// Methods for making new query objects

Xapian::Query::Internal::Internal(const string & tname_, Xapian::termcount wqf_,
		 Xapian::termpos term_pos_)
	: op(Xapian::Query::Internal::OP_LEAF),
	  subqs(),
	  qlen(wqf_),
	  window(0),
	  cutoff(0),
	  elite_set_size(0),
	  tname(tname_),
	  term_pos(term_pos_),
	  wqf(wqf_)
{
    if (tname.empty()) {
	throw Xapian::InvalidArgumentError("Termnames may not have zero length.");
    }
}

Xapian::Query::Internal::Internal(op_t op_)
	: op(op_),
	  subqs(),
	  qlen(0),
	  window(0),
	  cutoff(0),
	  elite_set_size(0),
	  tname(),
	  term_pos(0),
	  wqf(0)
{
}

Xapian::Query::Internal::~Internal()
{
#ifndef USE_DELETER_VECTOR
    subquery_list::iterator i;
    for (i = subqs.begin(); i != subqs.end(); i++) {
        delete *i;
    }
#endif
}

Xapian::Query::Internal *
Xapian::Query::Internal::end_construction()
{
    DEBUGCALL(API, void, "Xapian::Query::Internal::end_construction", "");
    prevalidate_query();
    Xapian::Query::Internal * qint = simplify_query();
    if (qint) qint->validate_query();
    return qint;
}

void
Xapian::Query::Internal::prevalidate_query() const
{
    DEBUGCALL(API, void, "Xapian::Query::Internal::prevalidate_query", "");

    // Check that the number of subqueries is in acceptable limits for this op
    if (subqs.size() < get_min_subqs(op) ||
	subqs.size() > get_max_subqs(op)) {
	throw Xapian::InvalidArgumentError("Xapian::Query: " + get_op_name(op) +
		" requires a minimum of " + om_tostring(get_min_subqs(op)) +
		" and a maximum of " + om_tostring(get_max_subqs(op)) +
		" sub queries, had " +
		om_tostring(subqs.size()) + ".");
    }

    // Check that the termname is not null in a leaf query
    Assert(!is_leaf(op) || !tname.empty());
    // Check that the termname is null in a branch query
    Assert(is_leaf(op) || tname.empty());
}

void
Xapian::Query::Internal::validate_query() const
{
    DEBUGCALL(API, void, "Xapian::Query::Internal::validate_query", "");
    prevalidate_query();

    // Check that the window size is in acceptable limits
    if (window < get_min_window(op)) {
	throw Xapian::InvalidArgumentError("Xapian::Query: " + get_op_name(op) +
		" requires a window size of at least " + 
		om_tostring(get_min_window(op)) + ", had " +
		om_tostring(window) + ".");
    }

    // Check that the cutoff parameter is in acceptable limits
    // FIXME: flakey and nasty.
    if (cutoff != 0 && op != Xapian::Query::OP_WEIGHT_CUTOFF) {
	throw Xapian::InvalidArgumentError("Xapian::Query: " + get_op_name(op) +
		" requires a cutoff of 0");
    }
    if (cutoff < 0) {
	throw Xapian::InvalidArgumentError("Xapian::Query: " + get_op_name(op) +
		" requires a cutoff of at least 0");
    }

    // Check that all subqueries are valid.
    subquery_list::const_iterator i;
    for (i = subqs.begin(); i != subqs.end(); ++i) {
	(**i).validate_query();
    }
}

Xapian::Query::Internal *
Xapian::Query::Internal::simplify_query()
{
    DEBUGCALL(API, bool, "Xapian::Query::Internal::simplify_query", "");

    // if window size is 0, then use number of subqueries
    // This is cheap, so we might as well always set it.
    if (window == 0) window = subqs.size();

    // if elite set size is 0, use sqrt of number of subqueries, or a minimum
    // of 10.  Gives a reasonable default.
    if (elite_set_size == 0) {
	elite_set_size = static_cast<Xapian::termcount>(ceil(sqrt(double(subqs.size()))));
	if (elite_set_size < 10) elite_set_size = 10;
    }

    // Remove duplicates if we can.
    if (subqs.size() > 1 && can_reorder(op)) {
	collapse_subqs();
    }

    // Flatten out sub queries if this is a phrase (or near) operation.
    if (can_flatten(op)) {
	flatten_subqs();
    }

    // If we have no subqueries, then we're simply an undefined query.
    if (subqs.empty() && !is_leaf(op)) return 0;

    // Some nodes with only one subquery can be replaced by the subquery.
    if (subqs.size() == 1 && can_replace_by_single_subq(op)) {
	Xapian::Query::Internal * qint = subqs[0];
	subqs[0] = 0;
	return qint;
    }

    return this;
}

struct SortPosName {
    bool operator()(const Xapian::Query::Internal * left,
		    const Xapian::Query::Internal * right) const {
	Assert(left != 0);
	Assert(right != 0);
	Assert(is_leaf(left->op));
	Assert(is_leaf(right->op));
	if (left->term_pos != right->term_pos) {
	    return left->term_pos < right->term_pos;
	} else {
	    return left->tname < right->tname;
	}
    }
};

/** Collapse occurrences of a term at the same position into a
 *  single occurrence with higher wqf.
 */
void
Xapian::Query::Internal::collapse_subqs()
{
    Assert(can_reorder(op));
    typedef set<Xapian::Query::Internal *, SortPosName> subqtable;
    subqtable sqtab;

    subquery_list::iterator sq = subqs.begin();
    while (sq != subqs.end()) {
	Assert(*sq != 0);
	if (is_leaf((*sq)->op)) {
	    Assert((*sq)->subqs.size() == 0);
	    subqtable::iterator s = sqtab.find(*sq);
	    if (sqtab.find(*sq) == sqtab.end()) {
		sqtab.insert(*sq);
		++sq;
	    } else {
		Assert((*s)->tname == (*sq)->tname);
		Assert((*s)->term_pos == (*sq)->term_pos);
		(*s)->wqf += (*sq)->wqf;
		(*s)->qlen += (*sq)->qlen;
		// rather than incrementing, delete the current
		// element, as it has been merged into the other
		// equivalent term.
#ifndef USE_DELETER_VECTOR
		delete *sq;
#endif
		sq = subqs.erase(sq);
	    }
	} else {
	    ++sq;
	}
    }
}

/// Change, eg, A NEAR (B AND C) to (A NEAR B) AND (A NEAR C)
void
Xapian::Query::Internal::flatten_subqs()
{
    Assert(op == Xapian::Query::OP_NEAR || op == Xapian::Query::OP_PHRASE);

    subquery_list::iterator sq;
    for (sq = subqs.begin(); sq != subqs.end(); sq++) {
	if (!is_leaf((*sq)->op)) break;
    }

    if (sq != subqs.end()) {
	if ((*sq)->op == Xapian::Query::OP_NEAR ||
	    (*sq)->op == Xapian::Query::OP_PHRASE) {
	    // FIXME: A PHRASE (B PHRASE C) -> (A PHRASE B) AND (B PHRASE C)?
	    throw Xapian::UnimplementedError("Can't use NEAR/PHRASE with a subexpression containing NEAR or PHRASE");
	}

	AutoPtr<Xapian::Query::Internal> flattenme(*sq);
	*sq = 0;

	// New query to build up.
	Xapian::Query::Internal newq(flattenme->op);

	subquery_list::iterator j;
	for (j = flattenme->subqs.begin(); j != flattenme->subqs.end(); ++j) {
	    *sq = *j;
	    *j = 0;
	    flatten_subqs();
	    newq.add_subquery(*this);
	    delete *sq;
	    *sq = 0;
	}

	Xapian::Query::Internal * newq2 = newq.end_construction();
	Assert(newq2);
	this->swap(*newq2);
    }
}

void
Xapian::Query::Internal::add_subquery(const Xapian::Query::Internal & subq)
{
    Assert(!is_leaf(op));
    if (can_reorder(op) && op == subq.op) {
	// Distribute the subquery.
	for (subquery_list::const_iterator i = subq.subqs.begin();
	     i != subq.subqs.end(); i++) {
	    add_subquery(**i);
	}
    } else {
	qlen += subq.qlen;
	subqs.push_back(new Xapian::Query::Internal(subq));
    }
}
