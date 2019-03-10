/** @file glass_positionlist.cc
 * @brief A position list in a glass database.
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

#include "glass_positionlist.h"

#include <xapian/types.h>

#include "bitstream.h"
#include "debuglog.h"
#include "pack.h"

#include <string>

using namespace std;

void
GlassPositionListTable::pack(string & s,
			     const Xapian::VecCOW<Xapian::termpos> & vec) const
{
    LOGCALL_VOID(DB, "GlassPositionListTable::pack", s | vec);
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
GlassPositionListTable::positionlist_count(const string& data) const
{
    LOGCALL(DB, Xapian::termcount, "GlassPositionListTable::positionlist_count", data);

    Assert(!data.empty());

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
    BitReader rd(pos, end);
    Xapian::termpos pos_first = rd.decode(pos_last);
    Xapian::termpos pos_size = rd.decode(pos_last - pos_first) + 2;
    RETURN(pos_size);
}

Xapian::termcount
GlassPositionListTable::positionlist_count(Xapian::docid did,
					   const string & term) const
{
    LOGCALL(DB, Xapian::termcount, "GlassPositionListTable::positionlist_count", did | term);

    string data;
    if (!get_exact_entry(make_key(did, term), data)) {
	RETURN(0);
    }

    RETURN(positionlist_count(data));
}

///////////////////////////////////////////////////////////////////////////

void
GlassBasePositionList::set_data(const string& data)
{
    LOGCALL_VOID(DB, "GlassBasePositionList::set_data", data);

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
GlassBasePositionList::get_approx_size() const
{
    LOGCALL(DB, Xapian::termcount, "GlassBasePositionList::get_approx_size", NO_ARGS);
    RETURN(size);
}

Xapian::termpos
GlassBasePositionList::back() const
{
    LOGCALL(DB, Xapian::termpos, "GlassBasePositionList::back", NO_ARGS);
    RETURN(last);
}

Xapian::termpos
GlassBasePositionList::get_position() const
{
    LOGCALL(DB, Xapian::termpos, "GlassBasePositionList::get_position", NO_ARGS);
    Assert(have_started);
    RETURN(current_pos);
}

bool
GlassBasePositionList::next()
{
    LOGCALL(DB, bool, "GlassBasePositionList::next", NO_ARGS);
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
GlassBasePositionList::skip_to(Xapian::termpos termpos)
{
    LOGCALL(DB, bool, "GlassBasePositionList::skip_to", termpos);
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

GlassPositionList::GlassPositionList(string&& data)
{
    LOGCALL_CTOR(DB, "GlassPositionList", data);

    pos_data = std::move(data);

    set_data(pos_data);
}

GlassPositionList::GlassPositionList(const GlassTable* table,
				     Xapian::docid did,
				     const string& term)
{
    LOGCALL_CTOR(DB, "GlassPositionList", table | did | term);

    if (!table->get_exact_entry(GlassPositionListTable::make_key(did, term),
				pos_data)) {
	pos_data.clear();
    }

    set_data(pos_data);
}

void
GlassRePositionList::assign_data(string&& data)
{
    LOGCALL_VOID(DB, "GlassRePositionList::assign_data", data);

    // We need to ensure the data stays valid while in use, so abuse the cursor
    // current_tag member as somewhere to store it.
    cursor.to_end();
    cursor.current_tag = std::move(data);

    set_data(cursor.current_tag);
}

void
GlassRePositionList::read_data(Xapian::docid did,
			       const string& term)
{
    LOGCALL_VOID(DB, "GlassRePositionList::read_data", did | term);

    if (!cursor.find_exact(GlassPositionListTable::make_key(did, term))) {
	cursor.current_tag.clear();
    }

    set_data(cursor.current_tag);
}
