/* omtermlistiterator.cc
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

#include "om/omtermlistiterator.h"
#include "omtermlistiteratorinternal.h"
#include "termlist.h"
#include "inmemory_positionlist.h"
#include "ompositionlistiteratorinternal.h"
#include "omdebug.h"

OmTermListIterator::OmTermListIterator(Internal *internal_)
	: internal(internal_)
{
    if (internal && internal->at_end()) {
	delete internal;
	internal = 0;
    }
}

OmTermListIterator::~OmTermListIterator() {
    DEBUGAPICALL(void, "OmTermListIterator::~OmTermListIterator", "");
    delete internal;
}

OmTermListIterator::OmTermListIterator(const OmTermListIterator &other)
    : internal(NULL)
{
    DEBUGAPICALL(void, "OmTermListIterator::OmTermListIterator", other);
    if (other.internal) internal = new Internal(*(other.internal));
}

void
OmTermListIterator::operator=(const OmTermListIterator &other)
{
    DEBUGAPICALL(void, "OmTermListIterator::operator=", other);
    if (this == &other) {
	DEBUGLINE(API, "OmTermListIterator assigned to itself");
	return;
    }

    Internal * newinternal = NULL;
    if (other.internal)
	newinternal = new Internal(*(other.internal));
    std::swap(internal, newinternal);
    delete newinternal;
}

om_termname
OmTermListIterator::operator *() const
{
    DEBUGAPICALL(om_termname, "OmTermListIterator::operator*", "");
    Assert(internal);
    Assert(!internal->at_end());
    if (internal->using_termlist) 
	RETURN(internal->termlist->get_termname());
    else
	RETURN(internal->it->first);
}

om_termcount
OmTermListIterator::get_wdf() const
{
    DEBUGAPICALL(om_termcount, "OmTermListIterator::get_wdf", "");
    Assert(internal);
    Assert(!internal->at_end());
    if (internal->using_termlist)
	RETURN(internal->termlist->get_wdf());
    else
	RETURN(internal->it->second.wdf);
}

om_doccount
OmTermListIterator::get_termfreq() const
{
    DEBUGAPICALL(om_doccount, "OmTermListIterator::get_termfreq", "");
    Assert(internal);
    Assert(!internal->at_end());
    if (internal->using_termlist)
	RETURN(internal->termlist->get_termfreq());
    else
	RETURN(internal->it->second.termfreq);
}

OmTermListIterator &
OmTermListIterator::operator++()
{
    DEBUGAPICALL(OmTermListIterator &, "OmTermListIterator::operator++", "");
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
OmTermListIterator::operator++(int)
{
    DEBUGAPICALL(void, "OmTermListIterator::operator++(int)", "");
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
OmTermListIterator::skip_to(const om_termname & tname)
{
    DEBUGAPICALL(void, "OmTermListIterator::skip_to", tname);
    Assert(internal);
    Assert(!internal->at_end());
    internal->skip_to(tname);
    if (internal->at_end()) {
	delete internal;
	internal = 0;
    }
}

OmPositionListIterator
OmTermListIterator::positionlist_begin()
{
    DEBUGAPICALL(OmPositionListIterator, "OmTermListIterator::positionlist_begin", "");
    Assert(internal);
    Assert(!internal->at_end());
    if (internal->using_termlist) {
	RETURN(internal->database.positionlist_begin(internal->did,
						     internal->termlist->get_termname()));
    } else {
	AutoPtr<InMemoryPositionList> pl(new InMemoryPositionList());
	pl->set_data(internal->it->second.positions);
	RETURN(OmPositionListIterator(new OmPositionListIterator::Internal(
			AutoPtr<PositionList>(pl.release()))));
    }
}

OmPositionListIterator
OmTermListIterator::positionlist_end()
{
    DEBUGAPICALL(OmPositionListIterator, "OmTermListIterator::positionlist_end", "");
    Assert(internal);
    Assert(!internal->at_end());
    RETURN(OmPositionListIterator(NULL));
}

std::string
OmTermListIterator::get_description() const
{
    DEBUGCALL(INTRO, std::string, "OmTermListIterator::get_description", "");
    /// \todo display contents of the object
    RETURN("OmTermListIterator()");
}


bool
operator==(const OmTermListIterator &a, const OmTermListIterator &b)
{
    if (a.internal == b.internal) return true;
    if (a.internal == 0 || b.internal == 0) return false;
    return (*(a.internal) == *(b.internal));
}
