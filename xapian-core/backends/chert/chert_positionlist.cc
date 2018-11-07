/* chert_positionlist.cc: A position list in a chert database.
 *
 * Copyright (C) 2004,2005,2006,2008,2010,2013 Olly Betts
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

#include "chert_positionlist.h"

#include <xapian/types.h>

#include "bitstream.h"
#include "debuglog.h"
#include "pack.h"

#include <string>
#include <vector>

using namespace std;

void
ChertPositionListTable::set_positionlist(Xapian::docid did,
					 const string & tname,
					 Xapian::PositionIterator pos,
					 const Xapian::PositionIterator &pos_end,
					 bool check_for_update)
{
    LOGCALL_VOID(DB, "ChertPositionListTable::set_positionlist", did | tname | pos | pos_end | check_for_update);
    Assert(pos != pos_end);

    // FIXME: avoid the need for this copy!
    vector<Xapian::termpos> poscopy(pos, pos_end);

    string key = make_key(did, tname);

    string s;
    pack_uint(s, poscopy.back());

    if (poscopy.size() > 1) {
	BitWriter wr(s);
	wr.encode(poscopy[0], poscopy.back());
	wr.encode(poscopy.size() - 2, poscopy.back() - poscopy[0]);
	wr.encode_interpolative(poscopy, 0, poscopy.size() - 1);
	swap(s, wr.freeze());
    }

    if (check_for_update) {
	string old_tag;
	if (get_exact_entry(key, old_tag) && s == old_tag)
	    return;
    }
    add(key, s);
}

Xapian::termcount
ChertPositionListTable::positionlist_count(Xapian::docid did,
					   const string & term) const
{
    LOGCALL(DB, Xapian::termcount, "ChertPositionListTable::positionlist_count", did | term);

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
ChertPositionList::read_data(const ChertTable * table, Xapian::docid did,
			     const string & tname)
{
    LOGCALL(DB, bool, "ChertPositionList::read_data", table | did | tname);

    have_started = false;

    string data;
    if (!table->get_exact_entry(ChertPositionListTable::make_key(did, tname), data)) {
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

Xapian::termcount
ChertPositionList::get_approx_size() const
{
    LOGCALL(DB, Xapian::termcount, "ChertPositionList::get_approx_size", NO_ARGS);
    RETURN(size);
}

Xapian::termpos
ChertPositionList::get_position() const
{
    LOGCALL(DB, Xapian::termpos, "ChertPositionList::get_position", NO_ARGS);
    Assert(have_started);
    RETURN(current_pos);
}

bool
ChertPositionList::next()
{
    LOGCALL(DB, bool, "ChertPositionList::next", NO_ARGS);
    if (rare(!have_started)) {
	have_started = true;
	return current_pos <= last;
    }
    if (current_pos == last) {
	return false;
    }
    current_pos = rd.decode_interpolative_next();
    return true;
}

bool
ChertPositionList::skip_to(Xapian::termpos termpos)
{
    LOGCALL(DB, bool, "ChertPositionList::skip_to", termpos);
    have_started = true;
    if (termpos >= last) {
	if (termpos == last) {
	    current_pos = last;
	    return true;
	}
	return false;
    }
    while (current_pos < termpos) {
	if (current_pos == last) {
	    return false;
	}
	current_pos = rd.decode_interpolative_next();
    }
    return true;
}
