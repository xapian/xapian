/* flint_positionlist.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005 Olly Betts
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
#include "omdebug.h"
#include "flint_positionlist.h"
#include "flint_utils.h"

using std::string;

static inline void
make_key(Xapian::docid did, const string & tname, string & key)
{
    key = pack_uint_preserving_sort(did) + tname;
}

void
FlintPositionList::read_data(const FlintTable * table,
			      Xapian::docid did,
			      const string & tname)
{
    DEBUGCALL(DB, void, "FlintPositionList::read_data",
	      table << ", " << did << ", " << tname);

    string key;
    make_key(did, tname, key);
    if (!table->get_exact_entry(key, data)) {
	// This isn't an error, since position list not be present simply
	// implies that there is no positional information available.
	data = "";
	pos = data.data();
	end = pos;
	is_at_end = false;
	have_started = false;
	current_pos = 0;
	number_of_entries = 0;
	return;
    }

    pos = data.data();
    end = pos + data.size();
    is_at_end = false;
    have_started = false;
    current_pos = 0;

    bool success = unpack_uint(&pos, end, &number_of_entries);
    if (! success) {
	if (pos == 0) {
	    // data ran out
	    throw Xapian::DatabaseCorruptError("Data ran out when reading position list length.");
	} else {
	    // overflow
	    throw Xapian::RangeError("Position list length too large.");
	}
    }
}

void
FlintPositionList::next_internal()
{
    DEBUGCALL(DB, void, "FlintPositionList::next_internal", "");
    if (pos == end) {
	is_at_end = true;
	return;
    }

    Xapian::termpos pos_increment;
    bool success = unpack_uint(&pos, end, &pos_increment);
    if (! success) {
	if (pos == 0) {
	    // data ran out
	    throw Xapian::DatabaseCorruptError("Data ran out when reading position list entry.");
	} else {
	    // overflow
	    throw Xapian::RangeError("Position list length too large.");
	}
    }
    Assert(pos != 0);
    current_pos += pos_increment + 1;
}

void
FlintPositionList::next()
{
    DEBUGCALL(DB, void, "FlintPositionList::next", "");
    Assert(!is_at_end);
    next_internal();
    have_started = true;
    DEBUGLINE(DB, string("FlintPositionList - moved to ") <<
	      (is_at_end ? string("end.") : std::string("position = ") +
	       om_tostring(current_pos) + "."));
}

void
FlintPositionList::skip_to(Xapian::termpos termpos)
{
    DEBUGCALL(DB, void, "FlintPositionList::skip_to", termpos);
    if (!have_started) {
	next_internal();
	have_started = true;
    }
    while (!is_at_end && current_pos < termpos) next_internal();
    DEBUGLINE(DB, string("FlintPositionList - skipped to ") <<
	      (is_at_end ? string("end.") : std::string("position = ") +
	       om_tostring(current_pos) + "."));
}

// Methods modifying position lists

void
FlintPositionListTable::set_positionlist(Xapian::docid did,
			const string & tname,
			Xapian::PositionIterator pos,
			const Xapian::PositionIterator &pos_end)
{
    DEBUGCALL(DB, void, "FlintPositionList::set_positionlist", did << ", " << tname << ", " << pos << ", " << pos_end);
    string key;

    make_key(did, tname, key);
    string tag;

    Xapian::termpos prevpos = 0;
    unsigned int size = 0;
    for ( ; pos != pos_end; ++pos) {
	tag += pack_uint(*pos - prevpos - 1);
	prevpos = *pos;
	size++;
    }
    tag = pack_uint(size) + tag;
    add(key, tag);
}

void
FlintPositionListTable::delete_positionlist(Xapian::docid did,
			const string & tname)
{
    DEBUGCALL(DB, void, "FlintPositionList::delete_positionlist", did << ", " << tname);
    string key;
    make_key(did, tname, key);
    del(key);
}
