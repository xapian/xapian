/* omquery.cc: External interface for running queries
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2003,2004,2005 Olly Betts
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

#include <vector>
#include <map>
#include <algorithm>
#include <math.h>

/// Add a subquery by reference
void
Xapian::Query::add_subquery(const Xapian::Query & subq)
{
    DEBUGAPICALL(void, "Xapian::Query::add_subquery", subq);
    Assert(internal.get());
    if (!subq.internal.get())
	throw Xapian::InvalidArgumentError("Can't compose a query from undefined queries");
    internal->add_subquery(*(subq.internal));
}

/// Add a subquery by pointer
void
Xapian::Query::add_subquery(const Xapian::Query * subq)
{
    DEBUGAPICALL(void, "Xapian::Query::add_subquery", subq);
    if (subq == 0) {
	throw Xapian::InvalidArgumentError("Pointer to subquery may not be null");
    }
    Assert(internal.get());
    if (!subq->internal.get())
	throw Xapian::InvalidArgumentError("Can't compose a query from undefined queries");
    internal->add_subquery(*(subq->internal));
}

/// Add a subquery which is a single term
void
Xapian::Query::add_subquery(const string & tname)
{
    DEBUGAPICALL(void, "Xapian::Query::add_subquery", tname);
    Assert(internal.get());
    internal->add_subquery(tname);
}

/// Setup the internals for the query, with the appropriate operator.
void
Xapian::Query::start_construction(Xapian::Query::op op_, Xapian::termpos window)
{
    DEBUGAPICALL(void, "Xapian::Query::start_construction", op_);
    Assert(!internal.get());
    internal = new Xapian::Query::Internal(op_, window);
}

/// Check that query has an appropriate number of arguments, etc,
void
Xapian::Query::end_construction()
{
    DEBUGAPICALL(void, "Xapian::Query::end_construction", "");
    Assert(internal.get());
    internal = internal->end_construction();
}

/// Abort construction of the query: delete internal.
void
Xapian::Query::abort_construction()
{
    DEBUGAPICALL(void, "Xapian::Query::abort_construction", "");
    Assert(internal.get());
    internal = 0;
}

Xapian::Query::Query(const string & tname_, Xapian::termcount wqf_, Xapian::termpos pos_)
	: internal(new Xapian::Query::Internal(tname_, wqf_, pos_))
{
    DEBUGAPICALL(void, "Xapian::Query::Query",
		 tname_ << ", " << wqf_ << ", " << pos_);
}

Xapian::Query::Query(Xapian::Query::op op_, const Xapian::Query &left, const Xapian::Query &right)
	: internal(new Xapian::Query::Internal(op_, 0u))
{
    DEBUGAPICALL(void, "Xapian::Query::Query",
		 op_ << ", " << left << ", " << right);
    try {
	add_subquery(left);
	add_subquery(right);
	end_construction();
    } catch (...) {
	abort_construction();
	throw;
    }
}

Xapian::Query::Query(Xapian::Query::op op_, Xapian::Query q) : internal(0)
{
    try {
	start_construction(op_, 0);
	add_subquery(q);
	end_construction();
    } catch (...) {
	abort_construction();
	throw;
    }
}

// Copy constructor
Xapian::Query::Query(const Xapian::Query & copyme)
	: internal(copyme.internal)
{
    DEBUGAPICALL(void, "Xapian::Query::Query", copyme);
}

// Assignment
Xapian::Query &
Xapian::Query::operator=(const Xapian::Query & copyme)
{
    DEBUGAPICALL(Xapian::Query &, "Xapian::Query::operator=", copyme);
    internal = copyme.internal;
    RETURN(*this);
}

// Default constructor
Xapian::Query::Query() : internal(0)
{
    DEBUGAPICALL(void, "Xapian::Query::Query", "");
}

// Destructor
Xapian::Query::~Query()
{
    DEBUGAPICALL(void, "Xapian::Query::~Query", "");
}

std::string
Xapian::Query::get_description() const
{
    DEBUGCALL(INTRO, std::string, "Xapian::Query::get_description", "");
    std::string res("Xapian::Query(");
    if (internal.get()) res += internal->get_description();
    res += ")";
    RETURN(res);
}

void Xapian::Query::set_elite_set_size(Xapian::termcount size)
{
    DEBUGAPICALL(void, "Xapian::Query::set_elite_set_size", size);
    Assert(internal.get());
    internal->set_elite_set_size(size);
}

Xapian::termcount Xapian::Query::get_length() const
{
    DEBUGAPICALL(Xapian::termcount, "Xapian::Query::get_length", "");
    RETURN(internal.get() ? internal->get_length() : 0);
}

Xapian::termcount Xapian::Query::set_length(Xapian::termcount qlen)
{
    DEBUGAPICALL(Xapian::termcount, "Xapian::Query::set_length", qlen);
    Assert(internal.get());
    RETURN(internal->set_length(qlen));
}

Xapian::TermIterator Xapian::Query::get_terms_begin() const
{
    DEBUGAPICALL(Xapian::TermIterator, "Xapian::Query::get_terms_begin", "");
    Assert(internal.get());
    RETURN(internal->get_terms());
}

Xapian::TermIterator Xapian::Query::get_terms_end() const
{
    DEBUGAPICALL(Xapian::TermIterator, "Xapian::Query::get_terms_end", "");
    RETURN(Xapian::TermIterator(NULL));
}
	
bool
Xapian::Query::empty() const
{
    DEBUGAPICALL(void, "Xapian::Query::empty", "");
    return internal.get() == 0;
}

Xapian::Query::Query(Query::op op,
		     const std::string & left, const std::string & right)
    : internal(0)
{
    try {
	start_construction(op, 0);
	add_subquery(left);
	add_subquery(right);
	end_construction();
    } catch (...) {
	abort_construction();
	throw;
    }
}
