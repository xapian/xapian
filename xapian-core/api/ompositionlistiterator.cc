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
#include <xapian/positioniterator.h>
#include "positionlist.h"
#include "omdebug.h"

Xapian::PositionIterator::PositionIterator(Internal *internal_)
	: internal(internal_)
{
    if (internal.get()) {
	internal->next();
       	if (internal->at_end()) internal = 0;
    }
}

Xapian::PositionIterator::PositionIterator() : internal(0)
{
}

Xapian::PositionIterator::PositionIterator(const Xapian::PositionIterator &o)
	: internal(o.internal)
{
}

Xapian::PositionIterator::~PositionIterator()
{
}

void
Xapian::PositionIterator::operator=(const Xapian::PositionIterator &o)
{
    internal = o.internal;
}

Xapian::termpos
Xapian::PositionIterator::operator *() const
{
    DEBUGAPICALL(Xapian::termpos, "Xapian::PositionIterator::operator*", "");
    Assert(internal.get());
    Assert(!internal->at_end());
    RETURN(internal->get_position());
}

Xapian::PositionIterator &
Xapian::PositionIterator::operator++()
{
    DEBUGAPICALL(Xapian::PositionIterator &, "Xapian::PositionIterator::operator++", "");
    Assert(internal.get());
    Assert(!internal->at_end());
    internal->next();
    if (internal->at_end()) internal = 0;
    RETURN(*this);
}

void
Xapian::PositionIterator::operator++(int)
{
    DEBUGAPICALL(void, "Xapian::PositionIterator::operator++(int)", "");
    Assert(internal.get());
    Assert(!internal->at_end());
    internal->next();
    if (internal->at_end()) internal = 0;
}

// extra method, not required to be an input_iterator
void
Xapian::PositionIterator::skip_to(Xapian::termpos pos)
{
    DEBUGAPICALL(void, "Xapian::PositionIterator::skip_to", pos);
    Assert(internal.get());
    Assert(!internal->at_end());
    internal->skip_to(pos);
    if (internal->at_end()) internal = 0;
}    

std::string
Xapian::PositionIterator::get_description() const
{
    DEBUGCALL(INTRO, std::string, "Xapian::PositionIterator::get_description", "");
    /// \todo display contents of the object
    RETURN("Xapian::PositionIterator()");
}
