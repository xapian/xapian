/* omqueryinternal.cc: Internals of query interface
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003 Olly Betts
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

#include "xapian/error.h"
#include "om/omenquire.h"
#include <xapian/output.h>

#include "xapian/termiterator.h"
#include "omtermlistiteratorinternal.h"

#include <vector>
#include <set>
#include <algorithm>
#include <math.h>
#include <limits.h>

// Properties for query operations.

static unsigned int
get_min_subqs(OmQuery::Internal::op_t op)
{
    switch (op) {
	case OmQuery::Internal::OP_UNDEF:
	case OmQuery::Internal::OP_LEAF:
	case OmQuery::OP_AND:
	case OmQuery::OP_OR:
	case OmQuery::OP_XOR:
	case OmQuery::OP_NEAR:
	case OmQuery::OP_PHRASE:
	case OmQuery::OP_ELITE_SET:
	    return 0;
	case OmQuery::OP_WEIGHT_CUTOFF:
	    return 1;
	case OmQuery::OP_FILTER:
	case OmQuery::OP_AND_MAYBE:
	case OmQuery::OP_AND_NOT:
	    return 2;
	default:
	    Assert(false);
	    throw Xapian::InvalidOperationError("get_min_subqs called with invalid operator type");
    }
}

static unsigned int
get_max_subqs(OmQuery::Internal::op_t op)
{
    switch (op) {
	case OmQuery::Internal::OP_UNDEF:
	case OmQuery::Internal::OP_LEAF:
	    return 0;
	case OmQuery::OP_WEIGHT_CUTOFF:
	    return 1;
	case OmQuery::OP_FILTER:
	case OmQuery::OP_AND_MAYBE:
	case OmQuery::OP_AND_NOT:
	    return 2;
	case OmQuery::OP_AND:
	case OmQuery::OP_OR:
	case OmQuery::OP_XOR:
	case OmQuery::OP_NEAR:
	case OmQuery::OP_PHRASE:
	case OmQuery::OP_ELITE_SET:
	    return UINT_MAX;
	default:
	    Assert(false);
	    throw Xapian::InvalidOperationError("get_max_subqs called with invalid operator type");
    }
}

static om_termpos
get_min_window(OmQuery::Internal::op_t op)
{
    switch (op) {
	case OmQuery::OP_NEAR:
	case OmQuery::OP_PHRASE:
	    return 1;
	default:
	    return 0;
    }
}

static bool
is_leaf(OmQuery::Internal::op_t op)
{
    return (op == OmQuery::Internal::OP_LEAF);
}

static bool
can_replace_by_single_subq(OmQuery::Internal::op_t op)
{
    return (op == OmQuery::OP_AND ||
	    op == OmQuery::OP_OR ||
	    op == OmQuery::OP_XOR ||
	    op == OmQuery::OP_NEAR ||
	    op == OmQuery::OP_PHRASE ||
// Can't replace OP_ELITE_SET by a single subq since then set_elite_set_size will barf...
//	    op == OmQuery::OP_ELITE_SET ||
	    op == OmQuery::OP_FILTER ||
	    op == OmQuery::OP_AND_MAYBE ||
	    op == OmQuery::OP_AND_NOT);
}

static bool
can_reorder(OmQuery::Internal::op_t op)
{
    return (op == OmQuery::OP_OR ||
	    op == OmQuery::OP_AND ||
	    op == OmQuery::OP_XOR);
}

static bool
can_flatten(OmQuery::Internal::op_t op)
{
    return (op == OmQuery::OP_NEAR || op == OmQuery::OP_PHRASE);
}

///////////////////////////////////
// Methods for OmQuery::Internal //
///////////////////////////////////

/** serialising method, for network matches.
 *
 *  The format is designed to be relatively easy
 *  to parse, as well as encodable in one line of text.
 *
 *  A null query is represented as `%N'.
 *
 *  A single-term query becomes `%T<tname> <termpos>,<wqf>'
 *                           or `%T<tname> <termpos>'
 *  where:
 *  	<tname> is the encoded term name
 *  	<wqf> is the decimal within query frequency (default 1),
 *  	<termpos> is the decimal term position.
 *
 *  A compound query becomes `%(<subqueries> <op>%)', where:
 *  	<subqueries> is the space-separated list of subqueries
 *  	<op> is one of: %and, %or, %filter, %andmaybe, %andnot, %xor
 */
std::string
OmQuery::Internal::serialise() const
{
    if (op == OmQuery::Internal::OP_UNDEF) return "%N";

    std::string result;

    std::string qlens = std::string("%L") + om_tostring(qlen);
    result += qlens;
    if (op == OmQuery::Internal::OP_LEAF) {
	result += "%T" + encode_tname(tname) + ' ' + om_tostring(term_pos);
	if (wqf != 1) result += ',' + om_tostring(wqf);
    } else {
	result += "%(";
	for (subquery_list::const_iterator i=subqs.begin();
	     i != subqs.end();
	     ++i) {
	    result += (*i)->serialise() + " ";
	}
	switch (op) {
	    case OmQuery::Internal::OP_LEAF:
		Assert(false);
		break;
	    case OmQuery::Internal::OP_UNDEF:
		Assert(false);
		break;
	    case OmQuery::OP_AND:
		result += "%and";
		break;
	    case OmQuery::OP_OR:
		result += "%or";
		break;
	    case OmQuery::OP_FILTER:
		result += "%filter";
		break;
	    case OmQuery::OP_AND_MAYBE:
		result += "%andmaybe";
		break;
	    case OmQuery::OP_AND_NOT:
		result += "%andnot";
		break;
	    case OmQuery::OP_XOR:
		result += "%xor";
		break;
	    case OmQuery::OP_NEAR:
		result += "%near" + om_tostring(window);
		break;
	    case OmQuery::OP_PHRASE:
		result += "%phrase" + om_tostring(window);
		break;
	    case OmQuery::OP_WEIGHT_CUTOFF:
		result += "%wtcutoff" + om_tostring(cutoff);
		break;
	    case OmQuery::OP_ELITE_SET:
		result += "%eliteset" + om_tostring(elite_set_size);
		break;
	} // switch(op)
	result += "%)";
    }
    return result;
}

std::string
OmQuery::Internal::get_op_name(OmQuery::Internal::op_t op)
{
    std::string name;
    switch(op) {
	case OmQuery::Internal::OP_UNDEF: name = "UNDEF"; break;
	case OmQuery::Internal::OP_LEAF:  name = "LEAF"; break;
	case OmQuery::OP_AND:             name = "AND"; break;
	case OmQuery::OP_OR:              name = "OR"; break;
	case OmQuery::OP_FILTER:          name = "FILTER"; break;
	case OmQuery::OP_AND_MAYBE:       name = "AND_MAYBE"; break;
	case OmQuery::OP_AND_NOT:         name = "AND_NOT"; break;
	case OmQuery::OP_XOR:             name = "XOR"; break;
	case OmQuery::OP_NEAR:            name = "NEAR"; break;
	case OmQuery::OP_PHRASE:          name = "PHRASE"; break;
	case OmQuery::OP_WEIGHT_CUTOFF:   name = "WEIGHT_CUTOFF"; break;
	case OmQuery::OP_ELITE_SET:       name = "ELITE_SET"; break;
    }
    return name;
}

// Introspection
std::string
OmQuery::Internal::get_description() const
{
    if(op == OmQuery::Internal::OP_UNDEF) return "<NULL>";
    std::string opstr;

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
	if (op == OmQuery::OP_NEAR || op == OmQuery::OP_PHRASE)
	    opstr += om_tostring(window) + " ";
	if (op == OmQuery::OP_WEIGHT_CUTOFF)
	    opstr += om_tostring(cutoff) + " ";
	if (op == OmQuery::OP_ELITE_SET)
	    opstr += om_tostring(elite_set_size) + " ";
    }
    std::string description;
    subquery_list::const_iterator i;
    for (i = subqs.begin(); i != subqs.end(); i++) {
	if (!description.empty()) description += opstr;
	description += (**i).get_description();
    }
    return "(" + description + ")";
}

void
OmQuery::Internal::set_window(om_termpos window_)
{
    window = window_;
}

void
OmQuery::Internal::set_cutoff(double cutoff_)
{
    if (op != OmQuery::OP_WEIGHT_CUTOFF)
	throw Xapian::InvalidOperationError("Can only set cutoff parameter for weight or percentage cutoff operators.");
    cutoff = cutoff_;
}

void
OmQuery::Internal::set_elite_set_size(om_termcount size_)
{
    if (op != OmQuery::OP_ELITE_SET)
	throw Xapian::InvalidOperationError("Can only set elite set size for elite set operator.");
    if (size_ == 0)
	throw Xapian::InvalidArgumentError("Elite set size may not be zero.");
    elite_set_size = size_;
}

om_termcount
OmQuery::Internal::set_length(om_termcount qlen_)
{
    om_termcount oldqlen = qlen;
    qlen = qlen_;
    return oldqlen;
}

/** Private function used to implement get_terms() */
void
OmQuery::Internal::accumulate_terms(
			std::vector<std::pair<string, om_termpos> > &terms) const
{
    Assert(op != OmQuery::Internal::OP_UNDEF);

    if (is_leaf(op)) {
        // We're a leaf, so just return our term.
        terms.push_back(std::make_pair(tname, term_pos));
    } else {
    	subquery_list::const_iterator end = subqs.end();
        // not a leaf, concatenate results from all subqueries
	for (subquery_list::const_iterator i = subqs.begin(); i != end; ++i) {
 	    (*i)->accumulate_terms(terms);
	}
    }
}

struct LessByTermpos {
    typedef const std::pair<string, om_termpos> argtype;
    bool operator()(argtype &left, argtype &right) {
	if (left.second != right.second) {
	    return left.second < right.second;
	} else {
	    return left.first < right.first;
	}
    }
};

Xapian::TermIterator
OmQuery::Internal::get_terms() const
{
    std::vector<std::pair<string, om_termpos> > terms;
    if (op != OmQuery::Internal::OP_UNDEF) {
        accumulate_terms(terms);
    }

    sort(terms.begin(), terms.end(), LessByTermpos());

    // remove adjacent duplicates, and return an iterator pointing
    // to just after the last unique element
    std::vector<std::pair<string, om_termpos> >::iterator newlast =
	    	unique(terms.begin(), terms.end());
    // and remove the rest...  (See Stroustrup 18.6.3)
    terms.erase(newlast, terms.end());

    std::vector<string> result;
    std::vector<std::pair<string, om_termpos> >::const_iterator i;
    for (i = terms.begin(); i != terms.end(); ++i) {
	result.push_back(i->first);
    }

    return Xapian::TermIterator(new VectorTermList(result.begin(),
						   result.end()));
}

// Methods 

// Make an uninitialised query
OmQuery::Internal::Internal()
	: op(OmQuery::Internal::OP_UNDEF),
	  subqs(),
	  qlen(0),
	  window(0),
	  cutoff(0),
	  elite_set_size(0),
	  tname(),
	  term_pos(0),
	  wqf(0)
{}

/** swap the contents of this with another OmQuery::Internal,
 *  in a way which is guaranteed not to throw.  This is
 *  used with the assignment operator to make it exception
 *  safe.
 *  It's important to adjust swap with any addition of
 *  member variables!
 */
void
OmQuery::Internal::swap(OmQuery::Internal &other)
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

void
OmQuery::Internal::operator=(const OmQuery::Internal &copyme)
{
    OmQuery::Internal temp(copyme);
    this->swap(temp);
}

void
OmQuery::Internal::initialise_from_copy(const OmQuery::Internal &copyme)
{
    OmQuery::Internal temp(copyme);
    this->swap(temp);
}

OmQuery::Internal::Internal(const OmQuery::Internal &copyme)
	: op(copyme.op),
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
	subqs.push_back(new OmQuery::Internal(**i));
    }
}

//////////////////////////////////////////
// Methods for making new query objects

OmQuery::Internal::Internal(const string & tname_, om_termcount wqf_,
		 om_termpos term_pos_)
	: op(OmQuery::Internal::OP_LEAF),
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

OmQuery::Internal::Internal(op_t op_)
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

OmQuery::Internal::~Internal()
{
#ifndef USE_DELETER_VECTOR
    subquery_list::iterator i;
    for (i = subqs.begin(); i != subqs.end(); i++) {
        delete *i;
    }
#endif
}

void
OmQuery::Internal::end_construction()
{
    DEBUGCALL(API, void, "OmQuery::Internal::end_construction", "");
    prevalidate_query();
    simplify_query();
    validate_query();
}

void
OmQuery::Internal::prevalidate_query() const
{
    DEBUGCALL(API, void, "OmQuery::Internal::prevalidate_query", "");

    // Check that the number of subqueries is in acceptable limits for this op
    if (subqs.size() < get_min_subqs(op) ||
	subqs.size() > get_max_subqs(op)) {
	throw Xapian::InvalidArgumentError("OmQuery: " + get_op_name(op) +
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
OmQuery::Internal::validate_query() const
{
    DEBUGCALL(API, void, "OmQuery::Internal::validate_query", "");
    prevalidate_query();

    // Check that the window size is in acceptable limits
    if (window < get_min_window(op)) {
	throw Xapian::InvalidArgumentError("OmQuery: " + get_op_name(op) +
		" requires a window size of at least " + 
		om_tostring(get_min_window(op)) + ", had " +
		om_tostring(window) + ".");
    }

    // Check that the cutoff parameter is in acceptable limits
    // FIXME: flakey and nasty.
    if (cutoff != 0 && op != OmQuery::OP_WEIGHT_CUTOFF) {
	throw Xapian::InvalidArgumentError("OmQuery: " + get_op_name(op) +
		" requires a cutoff of 0");
    }
    if (cutoff < 0) {
	throw Xapian::InvalidArgumentError("OmQuery: " + get_op_name(op) +
		" requires a cutoff of at least 0");
    }

    // Check that all subqueries are valid.
    for (subquery_list::const_iterator i = subqs.begin();
	 i != subqs.end();
	 ++i) {
	(**i).validate_query();
    }
}

void
OmQuery::Internal::simplify_query()
{
    DEBUGCALL(API, void, "OmQuery::Internal::simplify_query", "");

    // if window size is 0, then use number of subqueries
    // This is cheap, so we might as well always set it.
    if (window == 0) window = subqs.size();

    // if elite set size is 0, use sqrt of number of subqueries, or a minimum
    // of 10.  Gives a reasonable default.
    if (elite_set_size == 0) {
	elite_set_size = static_cast<om_termcount>(ceil(sqrt(double(subqs.size()))));
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
    if (subqs.size() == 0 && !is_leaf(op)) {
	op = OmQuery::Internal::OP_UNDEF;
	return;
    }

    // Some nodes with only one subquery can be replaced by the subquery.
    if (subqs.size() == 1 && can_replace_by_single_subq(op)) {
	initialise_from_copy(**(subqs.begin()));
    }
}

struct SortPosName {
    bool operator()(const OmQuery::Internal * left,
		    const OmQuery::Internal * right) const {
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
OmQuery::Internal::collapse_subqs()
{
    Assert(can_reorder(op));
    typedef std::set<OmQuery::Internal *, SortPosName> subqtable;
    subqtable sqtab;

    subquery_list::iterator sq = subqs.begin();
    while (sq != subqs.end()) {
	Assert(*sq != 0);
	Assert((*sq)->op != OmQuery::Internal::OP_UNDEF);
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
OmQuery::Internal::flatten_subqs()
{
    Assert(op == OmQuery::OP_NEAR || op == OmQuery::OP_PHRASE);

    subquery_list::iterator sq;
    for (sq = subqs.begin(); sq != subqs.end(); sq++) {
	Assert((*sq)->op != OmQuery::Internal::OP_UNDEF);
	if (!is_leaf((*sq)->op)) break;
    }

    if (sq != subqs.end()) {
	if ((*sq)->op == OmQuery::OP_NEAR ||
	    (*sq)->op == OmQuery::OP_PHRASE) {
	    // FIXME: A PHRASE (B PHRASE C) -> (A PHRASE B) AND (B PHRASE C)?
	    throw Xapian::UnimplementedError("Can't use NEAR/PHRASE with a subexpression containing NEAR or PHRASE");
	}

	AutoPtr<OmQuery::Internal> flattenme(*sq);
	*sq = 0;

	// New query to build up.
	OmQuery::Internal newq(flattenme->op);

	subquery_list::iterator j;
	for (j = flattenme->subqs.begin(); j != flattenme->subqs.end(); j++) {
	    *sq = *j;
	    *j = 0;
	    flatten_subqs();
	    newq.add_subquery(*this);
	    delete *sq;
	    *sq = 0;
	}

	newq.end_construction();

	this->swap(newq);
    }
}

void
OmQuery::Internal::add_subquery(const OmQuery::Internal & subq)
{
    Assert(!is_leaf(op));
    if (can_reorder(op) && (op == subq.op)) {
	// Distribute the subquery.
	for (subquery_list::const_iterator i = subq.subqs.begin();
	     i != subq.subqs.end(); i++) {
	    add_subquery(**i);
	}
    } else {
	qlen += subq.qlen;
	subqs.push_back(new OmQuery::Internal(subq));
    }
}

