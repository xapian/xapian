/* omvalueiterator.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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

#include "om/omvalueiterator.h"
#include "omvalueiteratorinternal.h"
#include "omdebug.h"

OmValueIterator::OmValueIterator()
	: internal(0)
{
}

OmValueIterator::OmValueIterator(Internal *internal_)
	: internal(internal_)
{
}

OmValueIterator::~OmValueIterator() {
    DEBUGAPICALL(void, "OmValueIterator::~OmValueIterator", "");
    delete internal;
}

OmValueIterator::OmValueIterator(const OmValueIterator &other)
    : internal(NULL)
{
    DEBUGAPICALL(void, "OmValueIterator::OmValueIterator", other);
    if (other.internal) internal = new Internal(*(other.internal));
}

void
OmValueIterator::operator=(const OmValueIterator &other)
{
    DEBUGAPICALL(void, "OmValueIterator::operator=", other);
    if (this == &other) {
	DEBUGLINE(API, "OmValueIterator assigned to itself");
	return;
    }

    Internal * newinternal = NULL;
    if (other.internal)
	newinternal = new Internal(*(other.internal));
    std::swap(internal, newinternal);
    delete newinternal;
}

const OmValue &
OmValueIterator::operator *() const
{
    DEBUGAPICALL(const OmValue &, "OmValueIterator::operator*", "");
    Assert(internal);
    RETURN(internal->it->second);
}

const OmValue *
OmValueIterator::operator ->() const
{
    DEBUGAPICALL(const OmValue *, "OmValueIterator::operator->", "");
    Assert(internal);
    RETURN(&(internal->it->second));
}

OmValueIterator &
OmValueIterator::operator++()
{
    DEBUGAPICALL(OmValueIterator &, "OmValueIterator::operator++", "");
    Assert(internal);
    internal->it++;
    RETURN(*this);
}

void
OmValueIterator::operator++(int)
{
    DEBUGAPICALL(void, "OmValueIterator::operator++", "int");
    Assert(internal);
    internal->it++;
}

om_valueno
OmValueIterator::get_valueno() const
{
    DEBUGAPICALL(om_valueno, "OmValueIterator::get_valueno", "");
    Assert(internal);
    RETURN(internal->it->first);
}

std::string
OmValueIterator::get_description() const
{
    DEBUGCALL(INTRO, std::string, "OmValueIterator::get_description", "");
    /// \todo display contents of the object
    RETURN("OmValueIterator()");
}

bool
operator==(const OmValueIterator &a, const OmValueIterator &b)
{
    Assert(a.internal);
    Assert(b.internal);
    return a.internal->it == b.internal->it;    
}
