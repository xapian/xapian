/* omquery.cc: External interface for running queries
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

#include <om/omerror.h>
#include <om/omenquire.h>
#include <om/omoutput.h>

#include <vector>
#include <map>
#include <algorithm>
#include <math.h>

/////////////////////////
// Methods for OmQuery //
/////////////////////////

OmQuery::OmQuery(const om_termname & tname_,
		 om_termcount wqf_,
		 om_termpos term_pos_)
	: internal(0)
{
    DEBUGAPICALL("OmQuery::OmQuery",
		 tname_ << ", " << wqf_ << ", " << term_pos_);
    internal = new OmQueryInternal(tname_, wqf_, term_pos_);
}

OmQuery::OmQuery(OmQuery::op op_, const OmQuery &left, const OmQuery &right)
	: internal(0)
{
    DEBUGAPICALL("OmQuery::OmQuery",
		 op_ << ", " << left << ", " << right);
    internal = new OmQueryInternal(op_,
				   *(left.internal),
				   *(right.internal));
}

OmQuery::OmQuery(OmQuery::op op_,
		 const std::vector<OmQuery *>::const_iterator qbegin,
		 const std::vector<OmQuery *>::const_iterator qend,
		 om_termpos window)
	: internal(0)
{
    // FIXME: display the contents of the vector
    DEBUGAPICALL("OmQuery::OmQuery",
		 op_ << ", " << "std::vector<OmQuery *>, " << window);
    std::vector<OmQueryInternal *> temp;
    std::vector<OmQuery *>::const_iterator i;
    for (i = qbegin; i != qend; i++) {
	temp.push_back((*i)->internal);
    }
    internal = new OmQueryInternal(op_, temp.begin(), temp.end(), window);
}

OmQuery::OmQuery(OmQuery::op op_,
		 const std::vector<OmQuery>::const_iterator qbegin,
		 const std::vector<OmQuery>::const_iterator qend,
		 om_termpos window)
	: internal(0)
{
    // FIXME: display the contents of the vector
    DEBUGAPICALL("OmQuery::OmQuery",
		 op_ << ", " << "std::vector<OmQuery>, " << window);
    std::vector<OmQueryInternal *> temp;
    std::vector<OmQuery>::const_iterator i;
    for (i = qbegin; i != qend; i++) {
	temp.push_back(i->internal);
    }
    internal = new OmQueryInternal(op_, temp.begin(), temp.end(), window);
}


OmQuery::OmQuery(OmQuery::op op_,
		 const std::vector<om_termname>::const_iterator tbegin,
		 const std::vector<om_termname>::const_iterator tend,
		 om_termpos window)
	: internal(0)
{
    DEBUGAPICALL("OmQuery::OmQuery",
		 op_ << ", " << "std::vector<om_termname>, " << window);
    internal = new OmQueryInternal(op_, tbegin, tend, window);
}

// Copy constructor
OmQuery::OmQuery(const OmQuery & copyme)
	: internal(0)
{
    DEBUGAPICALL("OmQuery::OmQuery", copyme);
    internal = new OmQueryInternal(*(copyme.internal));
}

// Assignment
OmQuery &
OmQuery::operator=(const OmQuery & copyme)
{
    DEBUGAPICALL("OmQuery::operator=", copyme);
    OmQueryInternal * temp = new OmQueryInternal(*(copyme.internal));
    std::swap(temp, this->internal);
    delete temp;

    DEBUGAPIRETURN(*this);
    return *this;
}

// Default constructor
OmQuery::OmQuery()
	: internal(0)
{
    DEBUGAPICALL("OmQuery::OmQuery", "");
    internal = new OmQueryInternal();
}

// Destructor
OmQuery::~OmQuery()
{
    DEBUGAPICALL("OmQuery::~OmQuery", "");
    delete internal;
}

std::string
OmQuery::get_description() const
{
    DEBUGAPICALL("OmQuery::get_description", "");
    OmLockSentry locksentry(internal->mutex);
    std::string description = "OmQuery(" + internal->get_description() + ")";
    DEBUGAPIRETURN(description);
    return description;
}

bool OmQuery::is_defined() const
{
    DEBUGAPICALL("OmQuery::is_defined", "");
    OmLockSentry locksentry(internal->mutex);
    DEBUGAPIRETURN(internal->is_defined());
    return internal->is_defined();
}

bool OmQuery::is_bool() const
{
    DEBUGAPICALL("OmQuery::is_bool", "");
    OmLockSentry locksentry(internal->mutex);
    bool ret = internal->is_bool();
    DEBUGAPIRETURN(ret);
    return ret;
}

bool OmQuery::set_bool(bool isbool_)
{
    DEBUGAPICALL("OmQuery::set_bool", isbool_);
    OmLockSentry locksentry(internal->mutex);
    bool ret = internal->set_bool(isbool_);
    DEBUGAPIRETURN(ret);
    return ret;
}

om_termcount OmQuery::get_length() const
{
    DEBUGAPICALL("OmQuery::get_length", "");
    OmLockSentry locksentry(internal->mutex);
    om_termcount ret = internal->get_length();
    DEBUGAPIRETURN(ret);
    return ret;
}

om_termcount OmQuery::set_length(om_termcount qlen_)
{
    DEBUGAPICALL("OmQuery::set_length", qlen_);
    OmLockSentry locksentry(internal->mutex);
    om_termcount ret = internal->set_length(qlen_);
    DEBUGAPIRETURN(ret);
    return ret;
}

om_termname_list OmQuery::get_terms() const
{
    DEBUGAPICALL("OmQuery::get_terms", "");
    OmLockSentry locksentry(internal->mutex);
    DEBUGAPIRETURN(internal->get_terms());
    return internal->get_terms();
}

/////////////////////////////////
// Methods for OmQueryInternal //
/////////////////////////////////

/** serialising method, for network matches.
 *
 *  The format is designed to be relatively easy
 *  to parse, as well as encodable in one line of text.
 *
 *  A null query is represented as `%N'.
 *
 *  A single-term query becomes `%T<tname>,<wqf>,<termpos>',
 *  where:
 *  	<tname> is the encoded term name
 *  	<wqf> is the decimal within query frequency,
 *  	<termpos> is the decimal term position.
 *
 *  A term name is encoded as a string of hex digits, two per
 *  byte in the term.  The first digit is the high nibble, and
 *  the second is the low nibble.
 *
 *  A compound query becomes `%(<subqueries> <op>%)', where:
 *  	<subqueries> is the space-separated list of subqueries
 *  	<op> is one of: %and, %or, %filter, %andmaybe, %andnot, %xor
 */
std::string
OmQueryInternal::serialise() const
{
    if (!isdefined) return "%N";

    std::string result;

    std::string qlens = std::string("%L") + om_tostring(qlen);
    if (isbool) {
	result = "%B";
    }
    result += qlens;
    if (op == OmQuery::OP_LEAF) {
	result += "%T" + encode_tname(tname) +
		"," + om_tostring(wqf) +
		"," + om_tostring(term_pos);
    } else {
	result += "%(";
	for (subquery_list::const_iterator i=subqs.begin();
	     i != subqs.end();
	     ++i) {
	    result += (*i)->serialise() + " ";
	}
	switch (op) {
	    case OmQuery::OP_LEAF:
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

// Introspection
std::string
OmQueryInternal::get_description() const
{
    if(!isdefined) return "<NULL>";
    std::string opstr;
    switch(op) {
	case OmQuery::OP_LEAF:
	    return tname;
	    break;
	case OmQuery::OP_AND:
	    opstr = " AND ";
	    break;
	case OmQuery::OP_OR:
	    opstr = " OR ";
	    break;
	case OmQuery::OP_FILTER:
	    opstr = " FILTER ";
	    break;
	case OmQuery::OP_AND_MAYBE:
	    opstr = " AND_MAYBE ";
	    break;
	case OmQuery::OP_AND_NOT:
	    opstr = " AND_NOT ";
	    break;
	case OmQuery::OP_XOR:
	    opstr = " XOR ";
	    break;
	case OmQuery::OP_NEAR:
	    opstr = " NEAR " + om_tostring(window) + " ";
	    break;
	case OmQuery::OP_PHRASE:
	    opstr = " PHRASE " + om_tostring(window) + " ";
	    break;
    }
    std::string description;
    std::vector<const OmQueryInternal *>::const_iterator i;
    for (i = subqs.begin(); i != subqs.end(); i++) {
	if (!description.empty()) description += opstr;
	description += (**i).get_description();
    }
    return "(" + description + ")";
}

bool
OmQueryInternal::set_bool(bool isbool_)
{
    bool oldbool = isbool;
    isbool = isbool_;
    return oldbool;
}

om_termcount
OmQueryInternal::set_length(om_termcount qlen_)
{
    om_termcount oldqlen = qlen;
    qlen = qlen_;
    return oldqlen;
}

void
OmQueryInternal::accumulate_terms(
			std::vector<std::pair<om_termname, om_termpos> > &terms) const
{
    Assert(isdefined);

    if (op == OmQuery::OP_LEAF) {
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

om_termname_list
OmQueryInternal::get_terms() const
{
    om_termname_list result;

    std::vector<std::pair<om_termname, om_termpos> > terms;
    if (isdefined) {
        accumulate_terms(terms);
    }

    sort(terms.begin(), terms.end(), LessByTermpos());

    // remove adjacent duplicates, and return an iterator pointing
    // to just after the last unique element
    std::vector<std::pair<om_termname, om_termpos> >::iterator newlast =
	    	unique(terms.begin(), terms.end());
    // and remove the rest...  (See Stroustrup 18.6.3)
    terms.erase(newlast, terms.end());

    std::vector<std::pair<om_termname, om_termpos> >::const_iterator i;
    for (i=terms.begin(); i!= terms.end(); ++i) {
	result.push_back(i->first);
    }

    return result;
}

OmQueryInternal::OmQueryInternal()
	: mutex(), isdefined(false), qlen(0)
{}

void
OmQueryInternal::swap(OmQueryInternal &other)
{
    std::swap(isdefined, other.isdefined);
    std::swap(isbool, other.isbool);
    std::swap(op, other.op);
    subqs.swap(other.subqs);
    std::swap(qlen, other.qlen);
    std::swap(tname, other.tname);
    std::swap(term_pos, other.term_pos);
    std::swap(wqf, other.wqf);
    std::swap(max_weight, other.max_weight);
    std::swap(window, other.window);
}

void
OmQueryInternal::operator=(const OmQueryInternal &copyme)
{
    // The exception-safe assignment operator idiom:
    OmQueryInternal temp(copyme);
    this->swap(temp);
}

OmQueryInternal::OmQueryInternal(const OmQueryInternal &copyme)
	: mutex(), isdefined(copyme.isdefined),
	isbool(copyme.isbool), op(copyme.op), subqs(subquery_list()),
        qlen(copyme.qlen), window(copyme.window), tname(copyme.tname),
        term_pos(copyme.term_pos), wqf(copyme.wqf),
        max_weight(copyme.max_weight)
{
    try {
	for (subquery_list::const_iterator i = copyme.subqs.begin();
	     i != copyme.subqs.end();
	     ++i) {
	    subqs.push_back(new OmQueryInternal(**i));
	}
    } catch (...) {
	// Delete the allocated subqueries if we fail
	for (subquery_list::const_iterator del = subqs.begin();
	     del != subqs.end();
	     ++del) {
	    delete *del;
	}
	// and rethrow the exception
	throw;
    }
}

OmQueryInternal::OmQueryInternal(const om_termname & tname_,
		 om_termcount wqf_,
		 om_termpos term_pos_)
	: isdefined(true), isbool(false), op(OmQuery::OP_LEAF),
	qlen(wqf_), tname(tname_), term_pos(term_pos_), wqf(wqf_)
{
    if(tname.size() == 0) {
	throw OmInvalidArgumentError("Termnames may not have zero length.");
    }
}

OmQueryInternal::OmQueryInternal(OmQuery::op op_,
				 OmQueryInternal &left,
				 OmQueryInternal &right)
	: isdefined(true), isbool(false), op(op_),
	  qlen(left.qlen + right.qlen)
{
    if (op == OmQuery::OP_LEAF) {
    	throw OmInvalidArgumentError("Invalid query operation");
    }

    if (op == OmQuery::OP_NEAR || op == OmQuery::OP_PHRASE) {
	std::vector<OmQueryInternal *> temp;
	temp.push_back(&left);
	temp.push_back(&right);	
	initialise_from_vector(temp.begin(), temp.end());
	return;
    }

    // reject any attempt to make up a composite query when any sub-query
    // is a pure boolean query.  FIXME: ought to handle the different
    // operators specially.
    if ((left.isdefined && left.isbool) || (right.isdefined && right.isbool)) {
	throw OmInvalidArgumentError("Only the top-level query can be bool");
    }

    // Handle undefined sub-queries.
    // See documentation for table for result of operations when one of the
    // operands is undefined:
    //
    if(!left.isdefined || !right.isdefined) {
	switch (op) {
	    case OmQuery::OP_OR:
	    case OmQuery::OP_AND:
		if (left.isdefined) {
		    initialise_from_copy(left);
		} else {
		    if (right.isdefined) initialise_from_copy(right);
		    else isdefined = false;
		}
		break;
	    case OmQuery::OP_FILTER:
		if (left.isdefined) {
		    initialise_from_copy(left);
		} else {
		    if (right.isdefined) {
			// Pure boolean
			initialise_from_copy(right);
			isbool = true;
		    } else {
			isdefined = false;
		    }
		}
		break;
	    case OmQuery::OP_AND_MAYBE:
		if (left.isdefined) {
		    initialise_from_copy(left);
		} else {
		    if (right.isdefined) initialise_from_copy(right);
		    else isdefined = false;
		}
		break;
	    case OmQuery::OP_AND_NOT:
		if (left.isdefined) {
		    initialise_from_copy(left);
		} else {
		    if (right.isdefined) {
		        throw OmInvalidArgumentError("AND NOT can't have an undefined LHS");
		    } else isdefined = false;
		}
		break;
	    case OmQuery::OP_XOR:
		if (!left.isdefined && !right.isdefined) {
		    isdefined = true;
		} else {
		    throw OmInvalidArgumentError("XOR can't have one undefined argument");
		}
		break;
	    case OmQuery::OP_LEAF:
	    case OmQuery::OP_NEAR:
	    case OmQuery::OP_PHRASE:
		Assert(false); // Shouldn't have got this far
	}
    } else {
	DebugMsg(" (" << left.get_description() << " " << (int) op <<
		 " " << right.get_description() << ") => ");

	// If sub query has same op, which is OR or AND, add to list rather
	// than makeing sub-node.  Can then optimise the list at search time.
	if(op == OmQuery::OP_AND || op == OmQuery::OP_OR) {
	    if(left.op == op && right.op == op) {
		// Both queries have same operation as top
		initialise_from_copy(left);
		std::vector<const OmQueryInternal *>::const_iterator i;
		for (i = right.subqs.begin(); i != right.subqs.end(); i++) {
		    subqs.push_back(new OmQueryInternal(**i));
		}
	    } else if(left.op == op) {
		// Query2 has different operation (or is a leaf)
		initialise_from_copy(left);
		subqs.push_back(new OmQueryInternal(right));
		// the initialise_from_copy will have set our
		// qlen to be just left's
		qlen += right.qlen;
	    } else if(right.op == op) { // left has different operation
		// Query1 has different operation (or is a leaf)
		initialise_from_copy(right);
		subqs.push_back(new OmQueryInternal(left));
		// the initialise_from_copy will have set our
		// qlen to be just right's
		qlen += left.qlen;
	    } else {
		subqs.push_back(new OmQueryInternal(left));
		subqs.push_back(new OmQueryInternal(right));
	    }
	} else {
	    subqs.push_back(new OmQueryInternal(left));
	    subqs.push_back(new OmQueryInternal(right));
	}
	collapse_subqs();
	DEBUGLINE(API, get_description());
    }
}

OmQueryInternal::OmQueryInternal(OmQuery::op op_,
		 const std::vector<OmQueryInternal *>::const_iterator qbegin,
		 const std::vector<OmQueryInternal *>::const_iterator qend,
		 om_termpos window_)
	: isdefined(true), isbool(false), op(op_)
{
    initialise_from_vector(qbegin, qend, window_);
    collapse_subqs();
}

OmQueryInternal::OmQueryInternal(OmQuery::op op_,
		 const std::vector<om_termname>::const_iterator tbegin,
		 const std::vector<om_termname>::const_iterator tend,
		 om_termpos window_)
	: isdefined(true), isbool(false), op(op_)
{
    std::vector<OmQueryInternal *> subqueries;
    try {
	std::vector<om_termname>::const_iterator i;
	for (i = tbegin; i != tend; i++) {
	    subqueries.push_back(new OmQueryInternal(*i));
	}
	initialise_from_vector(subqueries.begin(), subqueries.end(), window_);
	collapse_subqs();
    } catch (...) {
	// this code would be in a finally clause if there were one...
	// could also go in a destructor.
	std::vector<OmQueryInternal *>::iterator i;
	for (i = subqueries.begin(); i != subqueries.end(); ++i) {
	    delete *i;
	}
	throw;
    };
    // same code as above.
    // FIXME: use a destructor instead.
    std::vector<OmQueryInternal *>::iterator i;
    for (i = subqueries.begin(); i != subqueries.end(); ++i) {
	delete *i;
    }
}

OmQueryInternal::~OmQueryInternal()
{
    std::vector<OmQueryInternal *>::const_iterator i;
    for (i = subqs.begin(); i != subqs.end(); i++) {
	delete *i;
    }
    subqs.clear();
}

// Copy an OmQueryInternal object into self
void
OmQueryInternal::initialise_from_copy(const OmQueryInternal &copyme)
{
    isdefined = copyme.isdefined;
    isbool = copyme.isbool;
    op = copyme.op;
    qlen = copyme.qlen;
    if(op == OmQuery::OP_LEAF) {
	tname = copyme.tname;
	term_pos = copyme.term_pos;
	wqf = copyme.wqf;
    } else {
	std::vector<const OmQueryInternal *>::const_iterator i;
	for (i = subqs.begin(); i != subqs.end(); i++) {
	    delete *i;
	}
	subqs.clear();
	for(i = copyme.subqs.begin(); i != copyme.subqs.end(); i++) {
	    subqs.push_back(new OmQueryInternal(**i));
	}
    }
}

void
OmQueryInternal::initialise_from_vector(
			const std::vector<OmQueryInternal *>::const_iterator qbegin,
			const std::vector<OmQueryInternal *>::const_iterator qend,
                        om_termpos window_)
{
    bool merge_ok = false; // set if merging with subqueries is valid
    bool distribute = false; // Should A OP (B AND C) -> (A OP B) AND (A OP C)?
    switch (op) {
	case OmQuery::OP_AND:
	case OmQuery::OP_OR:
	    if (window_ != 0)
		throw OmInvalidArgumentError("window parameter not valid for AND or OR");
	    merge_ok = true;
	    break;
	case OmQuery::OP_NEAR:
	case OmQuery::OP_PHRASE:
	    // if NEAR/PHRASE and window_ == 0 default to number of subqueries
	    if (window_ == 0) window_ = (qend - qbegin);
	    distribute = true;
	    break;
	case OmQuery::OP_AND_NOT:
	case OmQuery::OP_XOR:
	case OmQuery::OP_AND_MAYBE:
	case OmQuery::OP_FILTER:
	    if ((qend - qbegin) != 2) {
		throw OmInvalidArgumentError("AND_NOT, XOR, AND_MAYBE, FILTER must have exactly two subqueries.");
	    }
	    break;
	case OmQuery::OP_LEAF:
	    throw OmInvalidArgumentError("LEAF queries from vectors invalid");
    }
    window = window_;
    qlen = 0;

    subquery_list::const_iterator i;
    // reject any attempt to make up a composite query when any sub-query
    // is a pure boolean query.  FIXME: ought to handle the different
    // operators specially.
    for (i = qbegin; i != qend; ++i) {
	if ((*i)->isbool) {
	    throw OmInvalidArgumentError("Only the top-level query can be pure boolean");
	}
    }

    if (distribute) {
	for (i = qbegin; i != qend; i++) {
	    if ((*i)->isdefined && (*i)->op != OmQuery::OP_LEAF) break;
	}
	if (i != qend) {
	    OmQuery::op newop = (*i)->op;
	    if (newop == OmQuery::OP_NEAR || newop == OmQuery::OP_PHRASE) {
		// FIXME: A PHRASE (B PHRASE C) -> (A PHRASE B) AND (B PHRASE C)?
		throw OmUnimplementedError("Can't use NEAR/PHRASE with a subexpression containing NEAR or PHRASE");
	    }
	    subquery_list copy(qbegin, qend);
	    subquery_list newsubqs;
	    int offset = i - qbegin;	    
	    subquery_list::const_iterator j;	    
	    for (j = (*i)->subqs.begin(); j != (*i)->subqs.end(); j++) {
		copy[offset] = *j;
		newsubqs.push_back(new OmQueryInternal(op, copy.begin(), copy.end(), window));
	    }	    
	    op = newop;
	    for (j = subqs.begin(); j != subqs.end(); j++) {
		delete *j;
	    }
	    subqs = newsubqs;
	}
    }

    try {
	for (i = qbegin; i != qend; i++) {
	    if ((*i)->isdefined) {
		/* if the subqueries have the same operator, then we
		 * merge them in, rather than just adding the query.
		 * There's no need to recurse any deeper, since the
		 * sub-queries will all have gone through this process
		 * themselves already.
		 */
		if (merge_ok && (*i)->op == op) {
		    subquery_list::const_iterator j;
		    for (j = (*i)->subqs.begin(); j != (*i)->subqs.end(); ++j) {
			subqs.push_back(new OmQueryInternal(**j));
		    }
		} else {
		    // sub-sub query has different op, just add
		    // it in.
		    subqs.push_back(new OmQueryInternal(**i));
		}
		qlen += (*i)->qlen;
	    }
	}
    } catch (...) {
	for (subquery_list::iterator i=subqs.begin(); i != subqs.end(); ++i) {
	    delete *i;
	}
        throw;
    }

    if (subqs.empty()) {
	isdefined = false;
    } else if(subqs.size() == 1) {
	// Should just have copied into self
	const OmQueryInternal * copyme = *(subqs.begin());
	subqs.clear();
	initialise_from_copy(*copyme);
	delete copyme;
    }
}

struct Collapse_PosNameLess {
    bool operator()(const std::pair<om_termpos, om_termname> &left,
		    const std::pair<om_termpos, om_termname> &right) const {
	if (left.first != right.first) {
	    return left.first < right.first;
	} else {
	    return left.second < right.second;
	}
    }
};

void OmQueryInternal::collapse_subqs()
{
    // We must have more than one query item for collapsing to make sense
    // For the moment, we only collapse ORs and ANDs.
    if ((subqs.size() > 1) && ((op == OmQuery::OP_OR) || (op == OmQuery::OP_AND))) {
	typedef std::map<std::pair<om_termpos, om_termname>,
	            OmQueryInternal *,
		    Collapse_PosNameLess> subqtable;

	subqtable sqtab;
	subquery_list::iterator sq = subqs.begin();
	while (sq != subqs.end()) {
	    if ((*sq)->op == OmQuery::OP_LEAF) {
		subqtable::key_type key(std::make_pair((*sq)->term_pos,
						  (*sq)->tname));
		subqtable::iterator s = sqtab.find(key);
		if (s == sqtab.end()) {
		    sqtab[key] = *sq;
		    ++sq;
		} else {
		    s->second->wqf += (*sq)->wqf;
		    // rather than incrementing, delete the current
		    // element, as it has been merged into the other
		    // equivalent term.
		    delete (*sq);
	 	    sq = subqs.erase(sq);
		}
	    } else {
		++sq;
	    }
	}

	// We should never lose all the subqueries
	Assert(subqs.size() > 0);

	// If we have only one subquery, move it into ourself
	if (subqs.size() == 1) {
	    // take care to preserve the query length
	    om_doclength qlen = get_length();
	    OmQueryInternal *only_child = *subqs.begin();
	    subqs.clear();
	    initialise_from_copy(*only_child);
	    delete only_child;
	    set_length(qlen);
	}
    }
}
