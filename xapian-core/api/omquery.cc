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

#include "om/omerror.h"
#include "om/omenquire.h"
#include "om/omoutput.h"

#include "om/omtermlistiterator.h"
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
    Assert(subq.internal != 0);
    internal->add_subquery(*(subq.internal));
}

/// Add a subquery by pointer
void
OmQuery::add_subquery(const OmQuery * subq)
{
    DEBUGAPICALL(void, "OmQuery::add_subquery", subq);
    if (subq == 0) {
	throw OmInvalidArgumentError("Pointer to subquery may not be null");
    }
    Assert(subq->internal != 0);
    internal->add_subquery(*(subq->internal));
}

/// Add a subquery which is a single term
void
OmQuery::add_subquery(const om_termname & tname)
{
    DEBUGAPICALL(void, "OmQuery::add_subquery", tname);
    OmQuery::Internal temp(tname);
    internal->add_subquery(temp);
}

/// Setup the internals for the query, with the appropriate operator.
void
OmQuery::start_construction(OmQuery::op op_)
{
    DEBUGAPICALL(void, "OmQuery::start_construction", op_);
    internal = new OmQuery::Internal(op_);
}

/// Check that query has an appropriate number of arguments, etc,
void
OmQuery::end_construction()
{
    DEBUGAPICALL(void, "OmQuery::end_construction", "");
    internal->end_construction();
}

/// Abort construction of the query: delete internal.
void
OmQuery::abort_construction()
{
    DEBUGAPICALL(void, "OmQuery::abort_construction", "");
    delete internal;
    internal = 0;
}

OmQuery::OmQuery(const om_termname & tname_,
		 om_termcount wqf_,
		 om_termpos term_pos_)
	: internal(0)
{
    DEBUGAPICALL(void, "OmQuery::OmQuery",
		 tname_ << ", " << wqf_ << ", " << term_pos_);
    internal = new OmQuery::Internal(tname_, wqf_, term_pos_);
}

OmQuery::OmQuery(OmQuery::op op_, const OmQuery &left, const OmQuery &right)
	: internal(0)
{
    DEBUGAPICALL(void, "OmQuery::OmQuery",
		 op_ << ", " << left << ", " << right);
    try {
	start_construction(op_);
	internal->add_subquery(*(left.internal));
	internal->add_subquery(*(right.internal));
	end_construction();
    } catch (...) {
	abort_construction();
	throw;
    }
}

// Copy constructor
OmQuery::OmQuery(const OmQuery & copyme)
	: internal(0)
{
    DEBUGAPICALL(void, "OmQuery::OmQuery", copyme);
    internal = new OmQuery::Internal(*(copyme.internal));
}

// Assignment
OmQuery &
OmQuery::operator=(const OmQuery & copyme)
{
    DEBUGAPICALL(OmQuery &, "OmQuery::operator=", copyme);
    OmQuery::Internal * temp = new OmQuery::Internal(*(copyme.internal));
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
    OmLockSentry locksentry(internal->mutex);
    RETURN("OmQuery(" + internal->get_description() + ")");
}

bool OmQuery::is_defined() const
{
    DEBUGAPICALL(bool, "OmQuery::is_defined", "");
    OmLockSentry locksentry(internal->mutex);
    RETURN(internal->is_defined());
}

bool OmQuery::is_bool() const
{
    DEBUGAPICALL(bool, "OmQuery::is_bool", "");
    OmLockSentry locksentry(internal->mutex);
    RETURN(internal->is_bool());
}

bool OmQuery::set_bool(bool isbool_)
{
    DEBUGAPICALL(bool, "OmQuery::set_bool", isbool_);
    OmLockSentry locksentry(internal->mutex);
    RETURN(internal->set_bool(isbool_));
}

void OmQuery::set_window(om_termpos window)
{
    DEBUGAPICALL(void, "OmQuery::set_window", window);
    OmLockSentry locksentry(internal->mutex);
    internal->set_window(window);
}

om_termcount OmQuery::get_length() const
{
    DEBUGAPICALL(om_termcount, "OmQuery::get_length", "");
    OmLockSentry locksentry(internal->mutex);
    RETURN(internal->get_length());
}

om_termcount OmQuery::set_length(om_termcount qlen_)
{
    DEBUGAPICALL(om_termcount, "OmQuery::set_length", qlen_);
    OmLockSentry locksentry(internal->mutex);
    RETURN(internal->set_length(qlen_));
}

OmTermIterator OmQuery::get_terms_begin() const
{
    DEBUGAPICALL(OmTermIterator, "OmQuery::get_terms_begin", "");
    OmLockSentry locksentry(internal->mutex);
    RETURN(internal->get_terms());
}

OmTermIterator OmQuery::get_terms_end() const
{
    DEBUGAPICALL(OmTermIterator, "OmQuery::get_terms_end", "");
    RETURN(OmTermIterator(NULL));
}

