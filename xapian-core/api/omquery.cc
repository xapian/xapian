/* omquery.cc: External interface for running queries
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
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
#include <map>
#include <algorithm>
#include <math.h>

/////////////////////////
// Methods for OmQuery //
/////////////////////////

/// Add a subquery by reference
void
OmQuery::add_subquery(const OmQuery & subq)
{
    DEBUGAPICALL(void, "OmQuery::add_subquery", subq);
    Assert(internal);
    Assert(subq.internal);
    internal->add_subquery(*(subq.internal));
}

/// Add a subquery by pointer
void
OmQuery::add_subquery(const OmQuery * subq)
{
    DEBUGAPICALL(void, "OmQuery::add_subquery", subq);
    if (subq == 0) {
	throw Xapian::InvalidArgumentError("Pointer to subquery may not be null");
    }
    Assert(internal);
    Assert(subq->internal);
    internal->add_subquery(*(subq->internal));
}

/// Add a subquery which is a single term
void
OmQuery::add_subquery(const string & tname)
{
    DEBUGAPICALL(void, "OmQuery::add_subquery", tname);
    OmQuery::Internal temp(tname);
    Assert(internal);
    internal->add_subquery(temp);
}

/// Setup the internals for the query, with the appropriate operator.
void
OmQuery::start_construction(OmQuery::op op_)
{
    DEBUGAPICALL(void, "OmQuery::start_construction", op_);
    Assert(!internal);
    internal = new OmQuery::Internal(op_);
}

/// Check that query has an appropriate number of arguments, etc,
void
OmQuery::end_construction()
{
    DEBUGAPICALL(void, "OmQuery::end_construction", "");
    Assert(internal);
    internal->end_construction();
}

/// Abort construction of the query: delete internal.
void
OmQuery::abort_construction()
{
    DEBUGAPICALL(void, "OmQuery::abort_construction", "");
    Assert(internal);
    delete internal;
    internal = 0;
}

OmQuery::OmQuery(const string & tname_,
		 om_termcount wqf_,
		 om_termpos term_pos_)
	: internal(new OmQuery::Internal(tname_, wqf_, term_pos_))
{
    DEBUGAPICALL(void, "OmQuery::OmQuery",
		 tname_ << ", " << wqf_ << ", " << term_pos_);
}

OmQuery::OmQuery(OmQuery::op op_, const OmQuery &left, const OmQuery &right)
	: internal(new OmQuery::Internal(op_))
{
    DEBUGAPICALL(void, "OmQuery::OmQuery",
		 op_ << ", " << left << ", " << right);
    internal->add_subquery(*(left.internal));
    internal->add_subquery(*(right.internal));
    end_construction();
}

// Copy constructor
OmQuery::OmQuery(const OmQuery & copyme)
	: internal(0)
{
    DEBUGAPICALL(void, "OmQuery::OmQuery", copyme);
    Assert(copyme.internal);
    internal = new OmQuery::Internal(*(copyme.internal));
}

// Assignment
OmQuery &
OmQuery::operator=(const OmQuery & copyme)
{
    DEBUGAPICALL(OmQuery &, "OmQuery::operator=", copyme);
    Assert(copyme.internal);
    OmQuery::Internal *temp = new OmQuery::Internal(*(copyme.internal));
    std::swap(temp, this->internal);
    delete temp;

    RETURN(*this);
}

// Default constructor
OmQuery::OmQuery()
	: internal(0)
{
    DEBUGAPICALL(void, "OmQuery::OmQuery", "");
    internal = new OmQuery::Internal();
}

// Destructor
OmQuery::~OmQuery()
{
    DEBUGAPICALL(void, "OmQuery::~OmQuery", "");
    delete internal;
}

std::string
OmQuery::get_description() const
{
    DEBUGCALL(INTRO, std::string, "OmQuery::get_description", "");
    RETURN("OmQuery(" + internal->get_description() + ')');
}

void OmQuery::set_window(om_termpos window)
{
    DEBUGAPICALL(void, "OmQuery::set_window", window);
    Assert(internal);
    internal->set_window(window);
}

void OmQuery::set_cutoff(om_weight cutoff)
{
    DEBUGAPICALL(void, "OmQuery::set_cutoff", cutoff);
    Assert(internal);
    internal->set_cutoff(cutoff);
}

void OmQuery::set_elite_set_size(om_termcount size)
{
    DEBUGAPICALL(void, "OmQuery::set_elite_set_size", size);
    Assert(internal);
    internal->set_elite_set_size(size);
}

om_termcount OmQuery::get_length() const
{
    DEBUGAPICALL(om_termcount, "OmQuery::get_length", "");
    Assert(internal);
    RETURN(internal->get_length());
}

om_termcount OmQuery::set_length(om_termcount qlen)
{
    DEBUGAPICALL(om_termcount, "OmQuery::set_length", qlen);
    Assert(internal);
    RETURN(internal->set_length(qlen));
}

Xapian::TermIterator OmQuery::get_terms_begin() const
{
    DEBUGAPICALL(Xapian::TermIterator, "OmQuery::get_terms_begin", "");
    Assert(internal);
    RETURN(internal->get_terms());
}

Xapian::TermIterator OmQuery::get_terms_end() const
{
    DEBUGAPICALL(Xapian::TermIterator, "OmQuery::get_terms_end", "");
    RETURN(Xapian::TermIterator(NULL));
}
	
bool
OmQuery::is_empty() const
{
    DEBUGAPICALL(void, "OmQuery::is_empty", "");
    Assert(internal);
    return internal->is_empty();
}
