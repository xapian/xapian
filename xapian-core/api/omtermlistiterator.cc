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
#include "omdebug.h"

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

const om_termname
OmTermListIterator::operator *() const
{
    DEBUGAPICALL(om_termname, "OmTermListIterator::operator*", "");
    RETURN(internal->termlist->get_termname());
}

om_termcount
OmTermListIterator::get_wdf() const
{
    DEBUGAPICALL(om_termcount, "OmTermListIterator::get_wdf", "");
    RETURN(internal->termlist->get_wdf());
}

om_doccount
OmTermListIterator::get_termfreq() const
{
    DEBUGAPICALL(om_doccount, "OmTermListIterator::get_termfreq", "");
    RETURN(internal->termlist->get_termfreq());
}

OmTermListIterator &
OmTermListIterator::operator++()
{
    DEBUGAPICALL(OmTermListIterator &, "OmTermListIterator::operator++", "");
    internal->termlist->next();
    RETURN(*this);
}

void
OmTermListIterator::operator++(int)
{
    DEBUGAPICALL(void, "OmTermListIterator::operator++(int)", "");
    internal->termlist->next();
}

// extra method, not required to be an input_iterator
void
OmTermListIterator::skip_to(const om_termname & tname)
{
    DEBUGAPICALL(void, "OmTermListIterator::skip_to", tname);
    while (!internal->termlist->at_end() &&
	   internal->termlist->get_termname() < tname)
	internal->termlist->next();
}

std::string
OmTermListIterator::get_description() const
{
    DEBUGAPICALL(std::string, "OmTermListIterator::get_description", "");
    /// \todo display contents of the object
    RETURN("OmTermListIterator()");
}

bool
operator==(const OmTermListIterator &a, const OmTermListIterator &b)
{
    if (a.internal == b.internal) return true;
    if (a.internal) {
	if (b.internal) return a.internal->termlist.get() == b.internal->termlist.get();
	return a.internal->termlist->at_end();
    }
    Assert(b.internal); // a.internal is NULL, so b.internal can't be
    return b.internal->termlist->at_end();
}
