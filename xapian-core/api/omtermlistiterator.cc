/* omtermlistiterator.cc
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

#include <config.h>
#include "om/omtermlistiterator.h"
#include "omtermlistiteratorinternal.h"
#include "termlist.h"
#include "positionlist.h"
#include "omdebug.h"

OmTermIterator::OmTermIterator()
	: internal(0)
{
}

OmTermIterator::OmTermIterator(Internal *internal_)
	: internal(internal_)
{
    if (internal && internal->at_end()) {
	delete internal;
	internal = 0;
    }
}

OmTermIterator::~OmTermIterator() {
    DEBUGAPICALL(void, "OmTermIterator::~OmTermIterator", "");
    delete internal;
}

OmTermIterator::OmTermIterator(const OmTermIterator &other)
    : internal(other.internal ? new Internal(*(other.internal)) : NULL)
{
    DEBUGAPICALL(void, "OmTermIterator::OmTermIterator", other);
}

void
OmTermIterator::operator=(const OmTermIterator &other)
{
    DEBUGAPICALL(void, "OmTermIterator::operator=", other);
    if (this == &other) {
	DEBUGLINE(API, "OmTermIterator assigned to itself");
	return;
    }

    Internal * newinternal = NULL;
    if (other.internal)
	newinternal = new Internal(*(other.internal));
    std::swap(internal, newinternal);
    delete newinternal;
}

string
OmTermIterator::operator *() const
{
    DEBUGAPICALL(string, "OmTermIterator::operator*", "");
    Assert(internal);
    Assert(!internal->at_end());
    RETURN(internal->get_termname());
}

om_termcount
OmTermIterator::get_wdf() const
{
    DEBUGAPICALL(om_termcount, "OmTermIterator::get_wdf", "");
    Assert(internal);
    Assert(!internal->at_end());
    RETURN(internal->get_wdf());
}

om_doccount
OmTermIterator::get_termfreq() const
{
    DEBUGAPICALL(om_doccount, "OmTermIterator::get_termfreq", "");
    Assert(internal);
    Assert(!internal->at_end());
    RETURN(internal->get_termfreq());
}

OmTermIterator &
OmTermIterator::operator++()
{
    DEBUGAPICALL(OmTermIterator &, "OmTermIterator::operator++", "");
    Assert(internal);
    Assert(!internal->at_end());
    internal->next();
    if (internal->at_end()) {
	delete internal;
	internal = 0;
    }
    RETURN(*this);
}

void
OmTermIterator::operator++(int)
{
    DEBUGAPICALL(void, "OmTermIterator::operator++(int)", "");
    Assert(internal);
    Assert(!internal->at_end());
    internal->next();
    if (internal->at_end()) {
	delete internal;
	internal = 0;
    }
}

// extra method, not required to be an input_iterator
void
OmTermIterator::skip_to(const string & tname)
{
    DEBUGAPICALL(void, "OmTermIterator::skip_to", tname);
    if(internal && !internal->at_end()) {
	internal->skip_to(tname);
	if (internal->at_end()) {
	    delete internal;
	    internal = 0;
	}
    }
}

Xapian::PositionListIterator
OmTermIterator::positionlist_begin()
{
    DEBUGAPICALL(Xapian::PositionListIterator, "OmTermIterator::positionlist_begin", "");
    Assert(internal);
    Assert(!internal->at_end());
    RETURN(internal->positionlist_begin());
}

Xapian::PositionListIterator
OmTermIterator::positionlist_end()
{
    DEBUGAPICALL(Xapian::PositionListIterator, "OmTermIterator::positionlist_end", "");
    Assert(internal);
    Assert(!internal->at_end());
    RETURN(Xapian::PositionListIterator(NULL));
}

std::string
OmTermIterator::get_description() const
{
    DEBUGCALL(INTRO, std::string, "OmTermIterator::get_description", "");
    /// \todo display contents of the object
    RETURN("OmTermIterator()");
}

bool
operator==(const OmTermIterator &a, const OmTermIterator &b)
{
    if (a.internal == b.internal) return true;
    if (a.internal == 0 || b.internal == 0) return false;
    return (*(a.internal) == *(b.internal));
}
