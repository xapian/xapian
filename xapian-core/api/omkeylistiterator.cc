/* omkeylistiterator.cc
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

#include "om/omkeylistiterator.h"
#include "omkeylistiteratorinternal.h"
#include "omdebug.h"

OmKeyListIterator::~OmKeyListIterator() {
    DEBUGAPICALL(void, "OmKeyListIterator::~OmKeyListIterator", "");
    delete internal;
}

OmKeyListIterator::OmKeyListIterator(const OmKeyListIterator &other)
    : internal(NULL)
{
    DEBUGAPICALL(void, "OmKeyListIterator::OmKeyListIterator", other);
    if (other.internal) internal = new Internal(*(other.internal));
}

void
OmKeyListIterator::operator=(const OmKeyListIterator &other)
{
    DEBUGAPICALL(void, "OmKeyListIterator::operator=", other);
    if (this == &other) {
	DEBUGLINE(API, "OmKeyListIterator assigned to itself");
	return;
    }

    Internal * newinternal = NULL;
    if (other.internal)
	newinternal = new Internal(*(other.internal));
    std::swap(internal, newinternal);
    delete newinternal;
}

const OmKey &
OmKeyListIterator::operator *() const
{
    DEBUGAPICALL(const OmKey &, "OmKeyListIterator::operator*", "");
    Assert(internal);
    RETURN(internal->it->second);
}

const OmKey *
OmKeyListIterator::operator ->() const
{
    DEBUGAPICALL(const OmKey *, "OmKeyListIterator::operator->", "");
    Assert(internal);
    RETURN(&(internal->it->second));
}

OmKeyListIterator &
OmKeyListIterator::operator++()
{
    DEBUGAPICALL(OmKeyListIterator &, "OmKeyListIterator::operator++", "");
    Assert(internal);
    internal->it++;
    RETURN(*this);
}

void
OmKeyListIterator::operator++(int)
{
    DEBUGAPICALL(void, "OmKeyListIterator::operator++", "int");
    Assert(internal);
    internal->it++;
}

om_keyno
OmKeyListIterator::get_keyno() const
{
    DEBUGAPICALL(om_keyno, "OmKeyListIterator::get_keyno", "");
    Assert(internal);
    RETURN(internal->it->first);
}

std::string
OmKeyListIterator::get_description() const
{
    DEBUGCALL(INTRO, std::string, "OmKeyListIterator::get_description", "");
    /// \todo display contents of the object
    RETURN("OmKeyListIterator()");
}

bool
operator==(const OmKeyListIterator &a, const OmKeyListIterator &b)
{
    Assert(a.internal);
    Assert(b.internal);
    return a.internal->it == b.internal->it;    
}
