/* brass_positionlist.cc: A position list in a brass database.
 *
 * Copyright (C) 2004,2005,2006,2008,2009,2010,2013 Olly Betts
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

#include "brass_positionlist.h"

#include <xapian/types.h>

#include "bitstream.h"
#include "debuglog.h"
#include "pack.h"

#include <string>
#include <vector>

using namespace std;

void
BrassPositionListTable::pack(string & s,
			     Xapian::PositionIterator pos,
			     const Xapian::PositionIterator &pos_end)
{
    LOGCALL_VOID(DB, "BrassPositionListTable::pack", s | pos | pos_end);
    Assert(pos != pos_end);

    // FIXME: avoid the need for this copy!
    vector<Xapian::termpos> poscopy(pos, pos_end);

    pack_uint(s, poscopy.back());

    if (poscopy.size() > 1) {
	BitWriter wr(s);
	wr.encode(poscopy[0], poscopy.back());
	wr.encode(poscopy.size() - 2, poscopy.back() - poscopy[0]);
	wr.encode_interpolative(poscopy, 0, poscopy.size() - 1);
	swap(s, wr.freeze());
    }
}

Xapian::termcount
BrassPositionListTable::positionlist_count(Xapian::docid did,
					   const string & term) const
{
    LOGCALL(DB, Xapian::termcount, "BrassPositionListTable::positionlist_count", did | term);

    string data;
    if (!get_exact_entry(make_key(did, term), data)) {
	RETURN(0);
    }

    const char * pos = data.data();
    const char * end = pos + data.size();
    Xapian::termpos pos_last;
    if (!unpack_uint(&pos, end, &pos_last)) {
	throw Xapian::DatabaseCorruptError("Position list data corrupt");
    }
    if (pos == end) {
	// Special case for single entry position list.
	RETURN(1);
    }

    // Skip the header we just read.
    BitReader rd(data, pos - data.data());
    Xapian::termpos pos_first = rd.decode(pos_last);
    Xapian::termpos pos_size = rd.decode(pos_last - pos_first) + 2;
    RETURN(pos_size);
}

///////////////////////////////////////////////////////////////////////////

bool
BrassPositionList::read_data(const string & data)
{
    LOGCALL(DB, bool, "BrassPositionList::read_data", data);

    have_started = false;

    if (data.empty()) {
	// There's no positional information for this term.
	size = 0;
	last = 0;
	current_pos = 1;
	RETURN(false);
    }

    const char * pos = data.data();
    const char * end = pos + data.size();
    Xapian::termpos pos_last;
    if (!unpack_uint(&pos, end, &pos_last)) {
	throw Xapian::DatabaseCorruptError("Position list data corrupt");
    }
    if (pos == end) {
	// Special case for single entry position list.
	size = 1;
	current_pos = last = pos_last;
	RETURN(true);
    }
    // Skip the header we just read.
    rd.init(data, pos - data.data());
    Xapian::termpos pos_first = rd.decode(pos_last);
    Xapian::termpos pos_size = rd.decode(pos_last - pos_first) + 2;
    rd.decode_interpolative(0, pos_size - 1, pos_first, pos_last);
    size = pos_size;
    last = pos_last;
    current_pos = pos_first;
    RETURN(true);
}

bool
BrassPositionList::read_data(const BrassTable * table, Xapian::docid did,
			     const string & tname)
{
    LOGCALL(DB, bool, "BrassPositionList::read_data", table | did | tname);
    string data;
    (void)table->get_exact_entry(BrassPositionListTable::make_key(did, tname), data);
    return read_data(data);
}

Xapian::termcount
BrassPositionList::get_size() const
{
    LOGCALL(DB, Xapian::termcount, "BrassPositionList::get_size", NO_ARGS);
    RETURN(size);
}

Xapian::termpos
BrassPositionList::get_position() const
{
    LOGCALL(DB, Xapian::termpos, "BrassPositionList::get_position", NO_ARGS);
    Assert(have_started);
    RETURN(current_pos);
}

void
BrassPositionList::next()
{
    LOGCALL_VOID(DB, "BrassPositionList::next", NO_ARGS);
    if (rare(!have_started)) {
	have_started = true;
	return;
    }
    if (current_pos == last) {
	last = 0;
	current_pos = 1;
	return;
    }
    current_pos = rd.decode_interpolative_next();
}

void
BrassPositionList::skip_to(Xapian::termpos termpos)
{
    LOGCALL_VOID(DB, "BrassPositionList::skip_to", termpos);
    have_started = true;
    if (termpos >= last) {
	if (termpos == last) {
	    current_pos = last;
	    return;
	}
	last = 0;
	current_pos = 1;
	return;
    }
    while (current_pos < termpos) {
	if (current_pos == last) {
	    last = 0;
	    current_pos = 1;
	    return;
	}
	current_pos = rd.decode_interpolative_next();
    }
}

bool
BrassPositionList::at_end() const
{
    LOGCALL(DB, bool, "BrassPositionList::at_end", NO_ARGS);
    Assert(have_started);
    RETURN(current_pos > last);
}
