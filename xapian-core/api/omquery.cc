/* omquery.cc: External interface for running queries
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2003,2004,2005,2006,2007,2008,2009 Olly Betts
 * Copyright 2006,2007,2008,2009 Lemur Consulting Ltd
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

#include "omqueryinternal.h"

#include "debuglog.h"
#include "omassert.h"
#include "utils.h"

#include "xapian/error.h"
#include "xapian/postingsource.h"
#include "xapian/registry.h"
#include "xapian/termiterator.h"

#include <cmath>

namespace Xapian {

/// Add a subquery by reference
void
Query::add_subquery(const Query & subq)
{
    LOGCALL_VOID(API, "Xapian::Query::add_subquery", subq);
    Assert(internal.get());
    internal->add_subquery(subq.internal.get());
}

/// Add a subquery by pointer
void
Query::add_subquery(const Query * subq)
{
    LOGCALL_VOID(API, "Xapian::Query::add_subquery", subq);
    if (subq == 0) {
	throw InvalidArgumentError("Pointer to subquery may not be null");
    }
    Assert(internal.get());
    internal->add_subquery(subq->internal.get());
}

/// Add a subquery which is a single term
void
Query::add_subquery(const string & tname)
{
    LOGCALL_VOID(API, "Xapian::Query::add_subquery", tname);
    Assert(internal.get());
    Query::Internal subqint(tname);
    internal->add_subquery(&subqint);
}

/// Setup the internals for the query, with the appropriate operator.
void
Query::start_construction(Query::op op_, termcount parameter)
{
    LOGCALL_VOID(API, "Xapian::Query::start_construction", op_);
    Assert(!internal.get());
    internal = new Query::Internal(op_, parameter);
}

/// Check that query has an appropriate number of arguments, etc,
void
Query::end_construction()
{
    LOGCALL_VOID(API, "Xapian::Query::end_construction", NO_ARGS);
    Assert(internal.get());
    internal = internal->end_construction();
}

/// Abort construction of the query: delete internal.
void
Query::abort_construction()
{
    LOGCALL_VOID(API, "Xapian::Query::abort_construction", NO_ARGS);
    Assert(internal.get());
    internal = 0;
}

Query::Query(const string & tname_, termcount wqf_, termpos pos_)
	: internal(new Query::Internal(tname_, wqf_, pos_))
{
    LOGCALL_VOID(API, "Xapian::Query::Query", tname_ | wqf_ | pos_);
}

Query::Query(Query::op op_, const Query &left, const Query &right)
	: internal(new Query::Internal(op_, 0u))
{
    LOGCALL_VOID(API, "Xapian::Query::Query", op_ | left | right);
    try {
	add_subquery(left);
	add_subquery(right);
	end_construction();
    } catch (...) {
	abort_construction();
	throw;
    }
}

Query::Query(Query::op op_, Xapian::Query q, double parameter)
{
    LOGCALL_VOID(API, "Xapian::Query::Query", op_ | q | parameter);
    if (op_ == OP_SCALE_WEIGHT) {
	if (!q.internal.get() ||
	    q.internal->op == OP_VALUE_RANGE ||
	    q.internal->op == OP_VALUE_GE ||
	    q.internal->op == OP_VALUE_LE) {
	    // Applying OP_SCALE_WEIGHT to Xapian::Query or OP_VALUE_*
	    // has no effect as they're all pure-boolean.
	    internal = q.internal;
	    return;
	}
    }
    try {
	start_construction(op_, 0);
	internal->set_dbl_parameter(parameter);
	add_subquery(q);
	end_construction();
    } catch (...) {
	abort_construction();
	throw;
    }
}

Query::Query(Query::op op_, Xapian::valueno slot,
	     const string &begin, const string &end)
    : internal(new Query::Internal(op_, slot, begin, end))
{
    LOGCALL_VOID(API, "Xapian::Query::Query", op_ | slot | begin | end);
}

Query::Query(Query::op op_, Xapian::valueno slot, const std::string &value)
    : internal(new Query::Internal(op_, slot, value))
{
    LOGCALL_VOID(API, "Xapian::Query::Query", op_ | slot | value);
}

Query::Query(PostingSource * external_source)
	: internal(NULL)
{
    LOGCALL_VOID(API, "Xapian::Query::Query", external_source);
    if (!external_source)
	throw Xapian::InvalidArgumentError("The external_source parameter can not be NULL");
    PostingSource * clone = external_source->clone();
    if (clone) {
	internal = new Query::Internal(clone, true);
    } else {
	internal = new Query::Internal(external_source, false);
    }
}

// Copy constructor
Query::Query(const Query & copyme)
	: internal(copyme.internal)
{
    LOGCALL_VOID(API, "Xapian::Query::Query", copyme);
}

// Assignment
Query &
Query::operator=(const Query & copyme)
{
    LOGCALL(API, Xapian::Query &, "Xapian::Query::operator=", copyme);
    internal = copyme.internal;
    RETURN(*this);
}

// Default constructor
Query::Query() : internal(0)
{
    LOGCALL_VOID(API, "Xapian::Query::Query", NO_ARGS);
}

// Destructor
Query::~Query()
{
    LOGCALL_VOID(API, "Xapian::Query::~Query", NO_ARGS);
}

std::string
Query::serialise() const
{
    LOGCALL(API, std::string, "Xapian::Query::serialise", NO_ARGS);
    if (!internal.get()) return std::string();
    return internal->serialise();
}

Query
Query::unserialise(const std::string &s)
{
    LOGCALL_STATIC(API, Xapian::Query, "Xapian::Query::unserialise", s);
    Query result;
    if (!s.empty()) {
	result.internal = Xapian::Query::Internal::unserialise(s, Registry());
    }
    RETURN(result);
}

Query
Query::unserialise(const std::string & s, const Registry & reg)
{
    LOGCALL_STATIC(API, Xapian::Query, "Xapian::Query::unserialise", s | reg);
    Query result;
    if (!s.empty()) {
	result.internal = Xapian::Query::Internal::unserialise(s, reg);
    }
    RETURN(result);
}

std::string
Query::get_description() const
{
    std::string res("Xapian::Query(");
    if (internal.get()) res += internal->get_description();
    res += ")";
    return res;
}

termcount Query::get_length() const
{
    LOGCALL(API, Xapian::termcount, "Xapian::Query::get_length", NO_ARGS);
    RETURN(internal.get() ? internal->get_length() : 0);
}

TermIterator Query::get_terms_begin() const
{
    LOGCALL(API, Xapian::TermIterator, "Xapian::Query::get_terms_begin", NO_ARGS);
    if (!internal.get()) RETURN(TermIterator());
    RETURN(internal->get_terms());
}

bool
Query::empty() const
{
    LOGCALL_VOID(API, "Xapian::Query::empty", NO_ARGS);
    return internal.get() == 0;
}

Query::Query(Query::op op_, const std::string & left, const std::string & right)
    : internal(0)
{
    try {
	start_construction(op_, 0);
	add_subquery(left);
	add_subquery(right);
	end_construction();
    } catch (...) {
	abort_construction();
	throw;
    }
}

/* Define static members. */
const Xapian::Query Xapian::Query::MatchAll = Xapian::Query(string());
const Xapian::Query Xapian::Query::MatchNothing = Xapian::Query();

}
