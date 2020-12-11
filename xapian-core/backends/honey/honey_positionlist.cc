/** @file
 * @brief A position list in a honey database.
 */
/* Copyright (C) 2004,2005,2006,2008,2009,2010,2013,2017,2019 Olly Betts
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

#include "honey_positionlist.h"

#include <xapian/types.h>

#include "bitstream.h"
#include "debuglog.h"
#include "honey_cursor.h"
#include "pack.h"

#include <string>

using namespace std;

void
HoneyPositionTable::pack(string& s,
			 const Xapian::VecCOW<Xapian::termpos>& vec) const
{
    LOGCALL_VOID(DB, "HoneyPositionTable::pack", s | vec);
    Assert(!vec.empty());

    pack_uint(s, vec.back());

    if (vec.size() > 1) {
	BitWriter wr(s);
	wr.encode(vec[0], vec.back());
	wr.encode(vec.size() - 2, vec.back() - vec[0]);
	wr.encode_interpolative(vec, 0, vec.size() - 1);
	swap(s, wr.freeze());
    }
}

Xapian::termcount
HoneyPositionTable::positionlist_count(Xapian::docid did,
				       const string& term) const
{
    LOGCALL(DB, Xapian::termcount, "HoneyPositionTable::positionlist_count", did | term);

    string data;
    if (!get_exact_entry(make_key(did, term), data)) {
	RETURN(0);
    }

    const char* pos = data.data();
    const char* end = pos + data.size();
    Xapian::termpos pos_last;
    if (!unpack_uint(&pos, end, &pos_last)) {
	throw Xapian::DatabaseCorruptError("Position list data corrupt");
    }
    if (pos == end) {
	// Special case for single entry position list.
	RETURN(1);
    }

    // Skip the header we just read.
    BitReader rd(pos, end);
    Xapian::termpos pos_first = rd.decode(pos_last);
    Xapian::termpos pos_size = rd.decode(pos_last - pos_first) + 2;
    RETURN(pos_size);
}

///////////////////////////////////////////////////////////////////////////

void
HoneyBasePositionList::set_data(const string& data)
{
    LOGCALL_VOID(DB, "HoneyBasePositionList::set_data", data);

    have_started = false;

    if (data.empty()) {
	// There's no positional information for this term.
	size = 0;
	last = 0;
	current_pos = 1;
	return;
    }

    const char* pos = data.data();
    const char* end = pos + data.size();
    Xapian::termpos pos_last;
    if (!unpack_uint(&pos, end, &pos_last)) {
	throw Xapian::DatabaseCorruptError("Position list data corrupt");
    }

    if (pos == end) {
	// Special case for single entry position list.
	size = 1;
	current_pos = last = pos_last;
	return;
    }

    rd.init(pos, end);
    Xapian::termpos pos_first = rd.decode(pos_last);
    Xapian::termpos pos_size = rd.decode(pos_last - pos_first) + 2;
    rd.decode_interpolative(0, pos_size - 1, pos_first, pos_last);
    size = pos_size;
    last = pos_last;
    current_pos = pos_first;
}

Xapian::termcount
HoneyBasePositionList::get_approx_size() const
{
    LOGCALL(DB, Xapian::termcount, "HoneyBasePositionList::get_approx_size", NO_ARGS);
    RETURN(size);
}

Xapian::termpos
HoneyBasePositionList::back() const
{
    LOGCALL(DB, Xapian::termpos, "HoneyBasePositionList::back", NO_ARGS);
    RETURN(last);
}

Xapian::termpos
HoneyBasePositionList::get_position() const
{
    LOGCALL(DB, Xapian::termpos, "HoneyBasePositionList::get_position", NO_ARGS);
    Assert(have_started);
    RETURN(current_pos);
}

bool
HoneyBasePositionList::next()
{
    LOGCALL(DB, bool, "HoneyBasePositionList::next", NO_ARGS);
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
HoneyBasePositionList::skip_to(Xapian::termpos termpos)
{
    LOGCALL(DB, bool, "HoneyBasePositionList::skip_to", termpos);
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

HoneyPositionList::HoneyPositionList(string&& data)
{
    LOGCALL_CTOR(DB, "HoneyPositionList", data);

    pos_data = std::move(data);

    set_data(pos_data);
}

HoneyPositionList::HoneyPositionList(const HoneyTable& table,
				     Xapian::docid did,
				     const string& term)
{
    LOGCALL_CTOR(DB, "HoneyPositionList", table | did | term);

    if (!table.get_exact_entry(HoneyPositionTable::make_key(did, term),
			       pos_data)) {
	pos_data.clear();
    }

    set_data(pos_data);
}

void
HoneyRePositionList::assign_data(string&& data)
{
    LOGCALL_VOID(DB, "HoneyRePositionList::assign_data", data);

    // We need to ensure the data stays valid while in use, so abuse the cursor
    // current_tag member as somewhere to store it.
    cursor.to_end();
    cursor.current_tag = std::move(data);

    set_data(cursor.current_tag);
}

void
HoneyRePositionList::read_data(Xapian::docid did,
			       const string& term)
{
    LOGCALL_VOID(DB, "HoneyRePositionList::read_data", did | term);

    if (!cursor.find_exact(HoneyPositionTable::make_key(did, term))) {
	cursor.current_tag.clear();
    } else {
	cursor.read_tag();
    }

    set_data(cursor.current_tag);
}
