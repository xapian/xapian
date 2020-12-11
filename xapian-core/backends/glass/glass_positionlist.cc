/** @file
 * @brief A position list in a glass database.
 */
/* Copyright (C) 2004,2005,2006,2008,2009,2010,2013,2019 Olly Betts
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
#include <vector>

using namespace std;

void
GlassPositionListTable::pack(string & s,
			     const vector<Xapian::termpos> & vec) const
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
    BitReader rd(data, pos - data.data());
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

bool
GlassPositionList::read_data(const string & data)
{
    LOGCALL(DB, bool, "GlassPositionList::read_data", data);

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
GlassPositionList::read_data(const GlassTable * table, Xapian::docid did,
			     const string & tname)
{
    LOGCALL(DB, bool, "GlassPositionList::read_data", table | did | tname);
    if (!cursor.get()) {
	cursor.reset(table->cursor_get());
    }
    if (cursor.get() &&
	cursor->find_exact(GlassPositionListTable::make_key(did, tname))) {
	RETURN(read_data(cursor->current_tag));
    }
    RETURN(read_data(string()));
}

Xapian::termcount
GlassPositionList::get_approx_size() const
{
    LOGCALL(DB, Xapian::termcount, "GlassPositionList::get_approx_size", NO_ARGS);
    RETURN(size);
}

Xapian::termpos
GlassPositionList::get_position() const
{
    LOGCALL(DB, Xapian::termpos, "GlassPositionList::get_position", NO_ARGS);
    Assert(have_started);
    RETURN(current_pos);
}

bool
GlassPositionList::next()
{
    LOGCALL(DB, bool, "GlassPositionList::next", NO_ARGS);
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
GlassPositionList::skip_to(Xapian::termpos termpos)
{
    LOGCALL(DB, bool, "GlassPositionList::skip_to", termpos);
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
