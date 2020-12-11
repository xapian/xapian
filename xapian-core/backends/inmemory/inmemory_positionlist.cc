/** @file
 * @brief PositionList from an InMemory DB or a Document object
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003,2009 Olly Betts
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

#include "inmemory_positionlist.h"

#include "debuglog.h"
#include "omassert.h"

InMemoryPositionList::InMemoryPositionList(const OmDocumentTerm::term_positions & positions_)
    : positions(positions_), mypos(positions.begin()),
      iterating_in_progress(false)
{
}

void
InMemoryPositionList::set_data(const OmDocumentTerm::term_positions & positions_)
{
    positions = positions_;
    mypos = positions.begin();
    iterating_in_progress = false;
}

Xapian::termcount
InMemoryPositionList::get_approx_size() const
{
    return positions.size();
}

Xapian::termpos
InMemoryPositionList::get_position() const
{
    Assert(iterating_in_progress);
    Assert(mypos != positions.end());
    return *mypos;
}

bool
InMemoryPositionList::next()
{
    if (iterating_in_progress) {
	Assert(mypos != positions.end());
	++mypos;
    } else {
	iterating_in_progress = true;
    }
    return (mypos != positions.end());
}

bool
InMemoryPositionList::skip_to(Xapian::termpos termpos)
{
    if (!iterating_in_progress) iterating_in_progress = true;
    while (mypos != positions.end() && *mypos < termpos) ++mypos;
    return (mypos != positions.end());
}
