/* quartz_positionlist.cc
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
#include "quartz_positionlist.h"
#include "quartz_utils.h"

void
QuartzPositionList::set_data(const std::string & data_)
{
    data = data_;
    pos = data.data();
    end = pos + data.size();
    is_at_end = false;

    bool success = unpack_uint(&pos, end, &number_of_entries);
    if (! success) {
	if (pos == 0) {
	    // data ran out
	    throw OmDatabaseCorruptError("Data ran out when reading position list length.");
	} else {
	    // overflow
	    throw OmRangeError("Position list length too large.");
	}
    }
}

void
QuartzPositionList::next_internal()
{
    if (pos == data.end()) {
	is_at_end = true;
	return;
    }

    const char * tmppos = pos;
    bool success = unpack_uint(&tmppos, end, &current_pos);
    if (! success) {
	if (tmppos == 0) {
	    // data ran out
	    throw OmDatabaseCorruptError("Data ran out when reading position list entry.");
	} else {
	    // overflow
	    throw OmRangeError("Position list length too large.");
	}
    }
    if (pos == data.end()) is_at_end = true;
}

void
QuartzPositionList::next()
{
    Assert(!is_at_end);
    next_internal();
}

void
QuartzPositionList::skip_to(om_termpos termpos)
{
    if (pos == data.data()) next_internal();
    while(!is_at_end && current_pos < termpos) next_internal();
}

