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

OmTermListIterator::~OmTermListIterator() { }

const om_termname
OmTermListIterator::operator *() {
    return internal->termlist->get_termname();
}

OmTermListIterator &
OmTermListIterator::operator++() { 
    internal->termlist->next();
    return *this;
}

OmTermListIterator
OmTermListIterator::operator++(int) {
    internal->termlist->next();
    return *this;
}

// extra method, not required to be an input_iterator
OmTermListIterator
OmTermListIterator::skip_to(const om_termname & tname) {
    while (!internal->termlist->at_end() &&
	   internal->termlist->get_termname() < tname)
	internal->termlist->next();
    return *this;
}

std::string
OmTermListIterator::get_description() const
{
    DEBUGAPICALL("OmTermListIterator::get_description", "");
    /// \todo display contents of the object
    std::string description = "OmTermListIterator()";
    DEBUGAPIRETURN(description);
    return description;
}

bool
operator==(const OmTermListIterator &a, const OmTermListIterator &b)
{
    if (a.internal == b.internal) return true;
    if (a.internal) {
	return !b.internal && a.internal->termlist->at_end();
    }
    Assert(b.internal); // a.internal is NULL, so b.internal can't be
    return b.internal->termlist->at_end();
}
