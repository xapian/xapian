/* omnodeinstanceiterator.cc
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

#include "om/omnodeinstanceiterator.h"
#include "omnodeinstanceiteratorinternal.h"
#include "omdebug.h"

OmNodeInstanceIterator::OmNodeInstanceIterator(Internal *internal_)
	: internal(internal_)
{
    if (internal && internal->it == internal->end) {
	delete internal;
	internal = 0;
    }
}

OmNodeInstanceIterator::~OmNodeInstanceIterator()
{
    DEBUGAPICALL(void, "OmNodeInstanceIterator::~OmNodeInstanceIterator", "");
    delete internal;
}

OmNodeInstanceIterator::OmNodeInstanceIterator(const OmNodeInstanceIterator &other)
	: internal(NULL)
{
    DEBUGAPICALL(void, "OmNodeInstanceIterator::OmNodeInstanceIterator", other);
    if (other.internal) internal = new Internal(*(other.internal));
}

void
OmNodeInstanceIterator::operator=(const OmNodeInstanceIterator &other)
{
    DEBUGAPICALL(void, "OmNodeInstanceIterator::operator=", other);
    if (this == &other) {
	DEBUGLINE(API, "OmNodeInstanceIterator assigned to itself");
	return;
    }

    Internal * newinternal = NULL;
    if (other.internal)
	newinternal = new Internal(*(other.internal));
    std::swap(internal, newinternal);
    delete newinternal;
}

std::string
OmNodeInstanceIterator::operator *() const
{
    DEBUGAPICALL(std::string, "OmNodeInstanceIterator::operator*", "");
    Assert(internal);
    Assert(!(internal->it == internal->end));
    RETURN(internal->it->id);
}

#if 0
std::string
OmNodeInstanceIterator::get_type() const
{
    DEBUGAPICALL(std::string, "OmNodeInstanceIterator::get_type", "");
    Assert(internal);
    Assert(!(internal->it == internal->end));
    RETURN(internal->it->type);
}

OmIndexerMessageType
OmNodeInstanceIterator::get_phys_type() const
{
    DEBUGAPICALL(OmIndexerMessageType, "OmNodeInstanceIterator::get_phys_type", "");
    Assert(internal);
    Assert(!(internal->it == internal->end));
    RETURN(internal->it->phys_type);
}
#endif

OmNodeInstanceIterator &
OmNodeInstanceIterator::operator++()
{
    DEBUGAPICALL(OmNodeInstanceIterator &, "OmNodeInstanceIterator::operator++", "");
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
OmNodeInstanceIterator::operator++(int)
{
    DEBUGAPICALL(void, "OmNodeInstanceIterator::operator++(int)", "");
    Assert(internal);
    Assert(!(internal->it == internal->end));
    internal->it++;
    if (internal->it == internal->end) {
	delete internal;
	internal = 0;
    }
}

std::string
OmNodeInstanceIterator::get_description() const
{
    DEBUGCALL(INTRO, std::string, "OmNodeInstanceIterator::get_description", "");
    /// \todo display contents of the object
    RETURN("OmNodeInstanceIterator()");
}

bool
operator==(const OmNodeInstanceIterator &a, const OmNodeInstanceIterator &b)
{
    if (a.internal == b.internal) return true;
    if (a.internal == 0 || b.internal == 0) return false;
    return (a.internal->indexerdesc.get() == b.internal->indexerdesc.get() &&
	    a.internal->it == b.internal->it);
}
