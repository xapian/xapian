/* omqueryinternal.cc: Internals of query interface
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#include "omdebug.h"
#include "omlocks.h"
#include "omqueryinternal.h"
#include "utils.h"
#include "netutils.h"

#include "om/omerror.h"
#include "om/omenquire.h"
#include "om/omoutput.h"

#include "om/omtermlistiterator.h"
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
	    return 0;
	case OmQuery::OP_FILTER:
	case OmQuery::OP_AND_MAYBE:
	case OmQuery::OP_AND_NOT:
	    return 2;
	default:
	    Assert(false);
    };
}

static unsigned int
get_max_subqs(OmQuery::Internal::op_t op)
{
    switch (op) {
	case OmQuery::Internal::OP_UNDEF:
	case OmQuery::Internal::OP_LEAF:
	    return 0;
	case OmQuery::OP_FILTER:
	case OmQuery::OP_AND_MAYBE:
	case OmQuery::OP_AND_NOT:
	    return 2;
	case OmQuery::OP_AND:
	case OmQuery::OP_OR:
	case OmQuery::OP_XOR:
	case OmQuery::OP_NEAR:
	case OmQuery::OP_PHRASE:
	    return UINT_MAX;
	default:
	    Assert(false);
    };
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
    };
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
can_remove_nulls(OmQuery::Internal::op_t op)
{
    return (op == OmQuery::OP_OR ||
	    op == OmQuery::OP_AND ||
	    op == OmQuery::OP_XOR ||
	    op == OmQuery::OP_NEAR ||
	    op == OmQuery::OP_PHRASE ||
	    op == OmQuery::OP_FILTER ||
	    op == OmQuery::OP_AND_MAYBE ||
	    op == OmQuery::OP_AND_NOT);
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
    if (isbool) {
	result = "%B";
    }
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
    }
    std::string description;
    subquery_list::const_iterator i;
    for (i = subqs.begin(); i != subqs.end(); i++) {
	if (!description.empty()) description += opstr;
	description += (**i).get_description();
    }
    return "(" + description + ")";
}

bool
OmQuery::Internal::set_bool(bool isbool_)
{
    bool oldbool = isbool;
    isbool = isbool_;
    return oldbool;
}

void
OmQuery::Internal::set_window(om_termpos window_)
{
    window = window_;
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
			std::vector<std::pair<om_termname, om_termpos> > &terms) const
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
    typedef const std::pair<om_termname, om_termpos> argtype;
    bool operator()(argtype &left, argtype &right) {
	if (left.second != right.second) {
	    return left.second < right.second;
	} else {
	    return left.first < right.first;
	}
    }
};

OmTermIterator
OmQuery::Internal::get_terms() const
{
    std::vector<std::pair<om_termname, om_termpos> > terms;
    if (op != OmQuery::Internal::OP_UNDEF) {
        accumulate_terms(terms);
    }

    sort(terms.begin(), terms.end(), LessByTermpos());

    // remove adjacent duplicates, and return an iterator pointing
    // to just after the last unique element
    std::vector<std::pair<om_termname, om_termpos> >::iterator newlast =
	    	unique(terms.begin(), terms.end());
    // and remove the rest...  (See Stroustrup 18.6.3)
    terms.erase(newlast, terms.end());

    std::vector<om_termname> result;
    std::vector<std::pair<om_termname, om_termpos> >::const_iterator i;
    for (i = terms.begin(); i != terms.end(); ++i) {
	result.push_back(i->first);
    }

    return OmTermIterator(new OmTermIterator::Internal(
			      new VectorTermList(result.begin(),
						 result.end())));
}







// Methods 

// Make an uninitialised query
OmQuery::Internal::Internal()
	: mutex(), op(OmQuery::Internal::OP_UNDEF), qlen(0)
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
    std::swap(isbool, other.isbool);
    subqs.swap(other.subqs);
    std::swap(qlen, other.qlen);
    std::swap(window, other.window);
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
	: mutex(),
	  op(copyme.op),
	  isbool(copyme.isbool),
	  subqs(),
	  qlen(copyme.qlen),
	  window(copyme.window),
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

// //////////////////////////////////////////
// // Methods for making new query objects

OmQuery::Internal::Internal(const om_termname & tname_,
		 om_termcount wqf_,
		 om_termpos term_pos_)
	: mutex(),
	  op(OmQuery::Internal::OP_LEAF),
	  isbool(false),
	  subqs(),
	  qlen(wqf_),
	  window(0),
	  tname(tname_),
	  term_pos(term_pos_),
	  wqf(wqf_)
{
    if(tname.size() == 0) {
	throw OmInvalidArgumentError("Termnames may not have zero length.");
    }
}

OmQuery::Internal::Internal(op_t op_)
	: mutex(),
	  op(op_),
	  isbool(false),
	  subqs(),
	  qlen(0),
	  window(0),
	  tname(),
	  term_pos(0),
	  wqf(0)
{
}

OmQuery::Internal::~Internal()
{
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
    if (op == OmQuery::Internal::OP_UNDEF) return;

    // Check that the number of subqueries is in acceptable limits for this op
    if (subqs.size() < get_min_subqs(op) ||
	subqs.size() > get_max_subqs(op)) {
	throw OmInvalidArgumentError("OmQuery: " + get_op_name(op) +
		" requires a minimum of " + om_tostring(get_min_subqs(op)) +
		" and a maximum of " + om_tostring(get_max_subqs(op)) +
		" sub queries, had " +
		om_tostring(subqs.size()) + ".");
    }

    // Check that the termname is not null in a leaf query
    if (is_leaf(op)) {
	if (tname.size() == 0)
	    throw OmInvalidArgumentError("OmQuery: term names cannot be empty.");
    } else {
	AssertEq(tname.size(), 0);
    }
}

void
OmQuery::Internal::validate_query() const
{
    DEBUGCALL(API, void, "OmQuery::Internal::validate_query", "");
    prevalidate_query();

    // Check that the window size is in acceptable limits
    if (window < get_min_window(op)) {
	throw OmInvalidArgumentError("OmQuery: " + get_op_name(op) +
		" requires a window size of at least 1, had " +
		om_tostring(window) + ".");
    }

    // Check that all subqueries are defined and valid.
    for (subquery_list::const_iterator i = subqs.begin();
	 i != subqs.end();
	 ++i) {
	if ((**i).op == OmQuery::Internal::OP_UNDEF)
	    throw OmInvalidArgumentError("OmQuery: subqueries must not be undefined.");
	(**i).validate_query();

	// FIXME: pure boolean subqueries are okay in some situations (eg,
	// part of an AND -> a FILTER)
	if ((**i).isbool)
	    throw OmInvalidArgumentError("OmQuery: subqueries must not be pure boolean.");
    }
}

void
OmQuery::Internal::simplify_query()
{
    DEBUGCALL(API, void, "OmQuery::Internal::simplify_query", "");

    // Special handling for FILTER with null on LHS
    if (op == OmQuery::OP_FILTER &&
	subqs[0]->op == OmQuery::Internal::OP_UNDEF &&
	subqs[1]->op != OmQuery::Internal::OP_UNDEF) {
	Assert(subqs.size() == 2);
	initialise_from_copy(*(subqs[1]));
	isbool = true;
    }

    // Special handling for AND_NOT or AND_MAYBE with null on LHS
    if ((op == OmQuery::OP_AND_NOT || op == OmQuery::OP_AND_MAYBE) &&
	subqs[0]->op == OmQuery::Internal::OP_UNDEF &&
	subqs[1]->op != OmQuery::Internal::OP_UNDEF) {
	Assert(subqs.size() == 2);
	throw OmInvalidArgumentError(get_op_name(op) +
				     " can't have an undefined LHS");
    }

    // if window size is 0, then use number of subqueries
    // This is cheap, so we might as well always set it.
    if (window == 0) window = subqs.size();

    // Remove nulls if we can
    if (subqs.size() > 0 && can_remove_nulls(op)) {
	remove_undef_subqs();
    }

    // Remove duplicates if we can.
    if (subqs.size() > 1 && can_reorder(op)) {
	collapse_subqs();
    }

    // Flatten out sub queries if this is a phrase (or near) operation.
    if(op == OmQuery::OP_NEAR || op == OmQuery::OP_PHRASE) {
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

/** Remove undefined subqueries.
 */
void
OmQuery::Internal::remove_undef_subqs()
{
    Assert(can_remove_nulls(op));

    subquery_list::iterator sq = subqs.begin();
    while (sq != subqs.end()) {
	Assert(*sq != 0);
	if ((*sq)->op == OmQuery::Internal::OP_UNDEF) {
	    sq = subqs.erase(sq);
	} else {
	    ++sq;
	}
    }
}

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

    subquery_list::iterator sq = subqs.begin();
    for (sq = subqs.begin(); sq != subqs.end(); sq++) {
	Assert((*sq)->op != OmQuery::Internal::OP_UNDEF);
	if (!is_leaf((*sq)->op)) break;
    }

    if (sq != subqs.end()) {
	if ((*sq)->op == OmQuery::OP_NEAR ||
	    (*sq)->op == OmQuery::OP_PHRASE) {
	    // FIXME: A PHRASE (B PHRASE C) -> (A PHRASE B) AND (B PHRASE C)?
	    throw OmUnimplementedError("Can't use NEAR/PHRASE with a subexpression containing NEAR or PHRASE");
	}

	// Offset to subquery we're flattening.
	subquery_list::size_type offset = sq - subqs.begin();
	AutoPtr<OmQuery::Internal> flattenme(subqs[offset]);
	subqs[offset] = 0;

	// New query to build up.
	OmQuery::Internal newq(flattenme->op);

	for (subquery_list::size_type j = 0;
	     j != flattenme->subqs.size();
	     j++) {
	    subqs[offset] = flattenme->subqs[j];
	    flattenme->subqs[j] = 0;
	    flatten_subqs();
	    newq.add_subquery(*this);
	    delete subqs[offset];
	    subqs[offset] = 0;
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

