/* ompositionlistiterator.cc
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003,2004,2008 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>
#include <xapian/positioniterator.h>
#include "positionlist.h"
#include "debuglog.h"
#include "omassert.h"

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
    LOGCALL(API, Xapian::termpos, "Xapian::PositionIterator::operator*", NO_ARGS);
    Assert(internal.get());
    Assert(!internal->at_end());
    RETURN(internal->get_position());
}

Xapian::PositionIterator &
Xapian::PositionIterator::operator++()
{
    LOGCALL_VOID(API, "Xapian::PositionIterator::operator++", NO_ARGS);
    Assert(internal.get());
    Assert(!internal->at_end());
    internal->next();
    if (internal->at_end()) internal = 0;
    return *this;
}

// extra method, not required to be an input_iterator
void
Xapian::PositionIterator::skip_to(Xapian::termpos pos)
{
    LOGCALL_VOID(API, "Xapian::PositionIterator::skip_to", pos);
    if (!internal.get()) return;
    Assert(!internal->at_end());
    internal->skip_to(pos);
    if (internal->at_end()) internal = 0;
}    

std::string
Xapian::PositionIterator::get_description() const
{
    /// \todo display contents of the object
    return "Xapian::PositionIterator()";
}
