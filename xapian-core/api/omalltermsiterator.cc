/* omalltermsiterator.cc
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

#include "om/omalltermsiterator.h"
#include "omalltermsiteratorinternal.h"
#include "alltermslist.h"
#include "omdebug.h"
#include "autoptr.h"
#include "omdatabaseinternal.h"
#include "omdatabaseinterface.h"

OmAllTermsIterator::OmAllTermsIterator(Internal *internal_)
	: internal(internal_)
{
    if (internal && internal->at_end()) {
	delete internal;
	internal = 0;
    }
}

OmAllTermsIterator::~OmAllTermsIterator() {
    DEBUGAPICALL(void, "OmAllTermsIterator::~OmAllTermsIterator", "");
    delete internal;
}

OmAllTermsIterator::OmAllTermsIterator(const OmAllTermsIterator &other)
    : internal(NULL)
{
    DEBUGAPICALL(void, "OmAllTermsIterator::OmAllTermsIterator", other);
    if (other.internal) {

	OmDatabase::Internal *dbinternal = OmDatabase::InternalInterface::get(other.internal->database);

	AutoPtr<Internal> newinternal(new Internal(
			dbinternal->open_allterms(other.internal->database),
			other.internal->database));
	newinternal->skip_to(other.internal->alltermslist->get_termname());
	internal = newinternal.release();
    }
}

void
OmAllTermsIterator::operator=(const OmAllTermsIterator &other)
{
    DEBUGAPICALL(void, "OmAllTermsIterator::operator=", other);
    if (this == &other) {
	DEBUGLINE(API, "OmAllTermsIterator assigned to itself");
	return;
    }

    OmAllTermsIterator newiterator(other);
    std::swap(internal, newiterator.internal);
}

om_termname
OmAllTermsIterator::operator *() const
{
    DEBUGAPICALL(om_termname, "OmAllTermsIterator::operator*", "");
    Assert(internal);
    Assert(!internal->at_end());
    RETURN(internal->alltermslist->get_termname());
}

om_doccount
OmAllTermsIterator::get_termfreq() const
{
    DEBUGAPICALL(om_doccount, "OmAllTermsIterator::get_termfreq", "");
    Assert(internal);
    Assert(!internal->at_end());
    RETURN(internal->alltermslist->get_termfreq());
}

OmAllTermsIterator &
OmAllTermsIterator::operator++()
{
    DEBUGAPICALL(OmAllTermsIterator &, "OmAllTermsIterator::operator++", "");
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
OmAllTermsIterator::operator++(int)
{
    DEBUGAPICALL(void, "OmAllTermsIterator::operator++(int)", "");
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
OmAllTermsIterator::skip_to(const om_termname & tname)
{
    DEBUGAPICALL(void, "OmAllTermsIterator::skip_to", tname);
    Assert(internal);
    Assert(!internal->at_end());
    internal->skip_to(tname);
    if (internal->at_end()) {
	delete internal;
	internal = 0;
    }
}

std::string
OmAllTermsIterator::get_description() const
{
    DEBUGCALL(INTRO, std::string, "OmAllTermsIterator::get_description", "");
    /// \todo display contents of the object
    RETURN("OmAllTermsIterator()");
}


bool
operator==(const OmAllTermsIterator &a, const OmAllTermsIterator &b)
{
    if (a.internal == b.internal) return true;
    if (a.internal == 0 || b.internal == 0) return false;
    return (*(a.internal) == *(b.internal));
}
