/* inmemory_positionlist.cc
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

#include "omdebug.h"
#include "inmemory_positionlist.h"

void
InMemoryPositionList::set_data(const std::vector<om_termpos> & positions_)
{
    positions = positions_;
    mypos = positions.begin();
    iterating_in_progress = false;
}

om_termcount
InMemoryPositionList::get_size() const
{
    return positions.size();
}

om_termpos
InMemoryPositionList::get_position() const
{
    Assert(iterating_in_progress);
    Assert(!at_end());
    return *mypos;
}

void
InMemoryPositionList::next()
{
    if(iterating_in_progress) {
	Assert(!at_end());
	mypos++;
    } else {
	iterating_in_progress = true;
    }
}

void
InMemoryPositionList::skip_to(om_termpos termpos)
{
    if (!iterating_in_progress) iterating_in_progress = true;
    while (!at_end() && *mypos < termpos) mypos++;
}

bool
InMemoryPositionList::at_end() const
{
    return(mypos == positions.end());
}
