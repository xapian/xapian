/* ompositionlistiterator.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003 Olly Betts
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
#include "xapian/positionlistiterator.h"
#include "positionlist.h"
#include "omdebug.h"

Xapian::PositionListIterator::PositionListIterator(Internal *internal_)
	: internal(internal_)
{
    if (internal.get()) {
	internal->next();
       	if (internal->at_end()) internal = 0;
    }
}

Xapian::PositionListIterator::PositionListIterator(const Xapian::PositionListIterator &o)
	: internal(o.internal)
{
}

Xapian::PositionListIterator::~PositionListIterator()
{
}

void
Xapian::PositionListIterator::operator=(Xapian::PositionListIterator &o)
{
    internal = o.internal;
}

om_termpos
Xapian::PositionListIterator::operator *() const
{
    DEBUGAPICALL(om_termpos, "Xapian::PositionListIterator::operator*", "");
    Assert(internal.get());
    Assert(!internal->at_end());
    RETURN(internal->get_position());
}

Xapian::PositionListIterator &
Xapian::PositionListIterator::operator++()
{
    DEBUGAPICALL(Xapian::PositionListIterator &, "Xapian::PositionListIterator::operator++", "");
    Assert(internal.get());
    Assert(!internal->at_end());
    internal->next();
    if (internal->at_end()) internal = 0;
    RETURN(*this);
}

void
Xapian::PositionListIterator::operator++(int)
{
    DEBUGAPICALL(void, "Xapian::PositionListIterator::operator++(int)", "");
    Assert(internal.get());
    Assert(!internal->at_end());
    internal->next();
    if (internal->at_end()) internal = 0;
}

// extra method, not required to be an input_iterator
void
Xapian::PositionListIterator::skip_to(om_termpos pos)
{
    DEBUGAPICALL(void, "Xapian::PositionListIterator::skip_to", pos);
    Assert(internal.get());
    Assert(!internal->at_end());
    internal->skip_to(pos);
    if (internal->at_end()) internal = 0;
}    

std::string
Xapian::PositionListIterator::get_description() const
{
    DEBUGCALL(INTRO, std::string, "Xapian::PositionListIterator::get_description", "");
    /// \todo display contents of the object
    RETURN("Xapian::PositionListIterator()");
}
