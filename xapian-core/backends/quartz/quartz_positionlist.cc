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
#include "quartz_table.h"

void
QuartzPositionList::read_data(const QuartzTable * table,
			      om_docid did,
			      const om_termname & tname)
{
    DEBUGCALL(DB, void, "QuartzPositionList::read_data",
	      table << ", " << did << ", " << tname);

    QuartzDbKey key;
    QuartzDbTag tag;
    make_key(did, tname, key);
    if (!table->get_exact_entry(key, tag)) {
	throw OmDocNotFoundError("Position list for term " + tname +
				 " in document " + om_tostring(did) +
				 " not present in database.");
    }

    // FIXME: Unwanted copy
    data = tag.value;

    pos = data.data();
    end = pos + data.size();
    is_at_end = false;
    have_started = false;

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
    if (pos == end) {
	is_at_end = true;
	return;
    }

    bool success = unpack_uint(&pos, end, &current_pos);
    if (! success) {
	if (pos == 0) {
	    // data ran out
	    throw OmDatabaseCorruptError("Data ran out when reading position list entry.");
	} else {
	    // overflow
	    throw OmRangeError("Position list length too large.");
	}
    }
    Assert(pos != 0);
}

void
QuartzPositionList::next()
{
    DEBUGCALL(DB, void, "QuartzPositionList::next", "");
    Assert(!is_at_end);
    next_internal();
    have_started = true;
    DEBUGLINE(DB, string("QuartzPositionList - moved to ") <<
	      (is_at_end ? string("end.") : string("position = ") +
	       om_tostring(current_pos) + "."));
}

void
QuartzPositionList::skip_to(om_termpos termpos)
{
    DEBUGCALL(DB, void, "QuartzPositionList::skip_to", termpos);
    if (!have_started) {
	next_internal();
	have_started = true;
    }
    while(!is_at_end && current_pos < termpos) next_internal();
    DEBUGLINE(DB, string("QuartzPositionList - skipped to ") <<
	      (is_at_end ? string("end.") : string("position = ") +
	       om_tostring(current_pos) + "."));
}

void
QuartzPositionList::make_key(om_docid did,
			     const om_termname & tname,
			     QuartzDbKey & key)
{
    key.value = pack_uint(did);
    key.value += pack_string(tname);
}

// Methods modifying position lists

void
QuartzPositionList::set_positionlist(QuartzBufferedTable * table,
			om_docid did,
			const om_termname & tname,
			const OmDocumentTerm::term_positions & positions)
{
    QuartzDbKey key;
    QuartzDbTag * tag;

    make_key(did, tname, key);
    tag = table->get_or_make_tag(key);

    tag->value = pack_uint(positions.size());

    OmDocumentTerm::term_positions::const_iterator i;
    for (i = positions.begin();
	 i != positions.end();
	 i++) {
	tag->value += pack_uint(*i);
    }
}

void
QuartzPositionList::delete_positionlist(QuartzBufferedTable * table,
			om_docid did,
			const om_termname & tname)
{
    QuartzDbKey key;
    make_key(did, tname, key);
    table->delete_tag(key);
}

