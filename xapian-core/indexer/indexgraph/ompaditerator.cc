/* ompaditerator.cc
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

#include "om/ompaditerator.h"
#include "ompaditeratorinternal.h"
#include "omdebug.h"

OmPadIterator::OmPadIterator()
	: internal(0)
{
}

OmPadIterator::OmPadIterator(Internal *internal_)
	: internal(internal_)
{
    if (internal && internal->it == internal->end) {
	delete internal;
	internal = 0;
    }
}

OmPadIterator::~OmPadIterator()
{
    DEBUGAPICALL(void, "OmPadIterator::~OmPadIterator", "");
    delete internal;
}

OmPadIterator::OmPadIterator(const OmPadIterator &other)
	: internal(NULL)
{
    DEBUGAPICALL(void, "OmPadIterator::OmPadIterator", other);
    if (other.internal) internal = new Internal(*(other.internal));
}

void
OmPadIterator::operator=(OmPadIterator &other)
{
    DEBUGAPICALL(void, "OmPadIterator::operator=", other);
    if (this == &other) {
	DEBUGLINE(API, "OmPadIterator assigned to itself");
	return;
    }

    Internal * newinternal = NULL;
    if (other.internal)
	newinternal = new Internal(*(other.internal));
    std::swap(internal, newinternal);
    delete newinternal;
}

std::string
OmPadIterator::operator *() const
{
    DEBUGAPICALL(std::string, "OmPadIterator::operator*", "");
    Assert(internal);
    Assert(!(internal->it == internal->end));
    RETURN(internal->it->name);
}

std::string
OmPadIterator::get_type() const
{
    DEBUGAPICALL(std::string, "OmPadIterator::get_type", "");
    Assert(internal);
    Assert(!(internal->it == internal->end));
    RETURN(internal->it->type);
}

OmIndexerMessageType
OmPadIterator::get_phys_type() const
{
    DEBUGAPICALL(OmIndexerMessageType, "OmPadIterator::get_phys_type", "");
    Assert(internal);
    Assert(!(internal->it == internal->end));
    RETURN(internal->it->phys_type);
}

OmPadIterator &
OmPadIterator::operator++()
{
    DEBUGAPICALL(OmPadIterator &, "OmPadIterator::operator++", "");
    Assert(internal);
    Assert(!(internal->it == internal->end));
    internal->it++;
    if (internal->it == internal->end) {
	delete internal;
	internal = 0;
    }
    RETURN(*this);
}

void
OmPadIterator::operator++(int)
{
    DEBUGAPICALL(void, "OmPadIterator::operator++(int)", "");
    Assert(internal);
    Assert(!(internal->it == internal->end));
    internal->it++;
    if (internal->it == internal->end) {
	delete internal;
	internal = 0;
    }
}

std::string
OmPadIterator::get_description() const
{
    DEBUGCALL(INTRO, std::string, "OmPadIterator::get_description", "");
    /// \todo display contents of the object
    RETURN("OmPadIterator()");
}

bool
operator==(const OmPadIterator &a, const OmPadIterator &b)
{
    if (a.internal == b.internal) return true;
    if (a.internal == 0 || b.internal == 0) return false;
    return (a.internal->nodedesc.get() == b.internal->nodedesc.get() &&
	    a.internal->it == b.internal->it);
}
