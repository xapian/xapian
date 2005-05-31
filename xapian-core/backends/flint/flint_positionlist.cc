/* flint_positionlist.cc: A position list in a flint database.
 *
 * Copyright (C) 2004,2005 Olly Betts
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
 */

#include <config.h>

#include <xapian/types.h>

#include "flint_positionlist.h"
#include "flint_utils.h"
#include "omdebug.h"

#include <vector>
#include <string>
#include <cmath>

using namespace std;

class BitWriter {
    private:
	string buf;
	int bits;
	unsigned int acc;
    public:
	BitWriter() : bits(0), acc(0) { }
	void write_bits(int data, int count) {
	    if (count + bits > 32) {
		// We need to write more bits than there's empty room for in
		// the accumulator.  So we arrange to shift out 8 bits, then
		// adjust things so we're adding 8 fewer bits.
		Assert(count <= 32);
		acc |= (data << bits);
		buf += char(acc);
		acc >>= 8;
		data >>= 8;
		count -= 8;
	    }
	    acc |= (data << bits);
	    bits += count;
	    while (bits >= 8) {
		buf += char(acc);
		acc >>= 8;
		bits -= 8;
	    }
	}
	string & freeze() {
	    if (bits) {
		buf += char(acc);
		bits = 0;
		acc = 0;
	    }
	    return buf;
	}
};

static void
encode_interpolative(BitWriter & wr, const vector<Xapian::termpos> &pos, int j, int k)
{
    if (j + 1 >= k) return;
    
    const size_t mid = (j + k) / 2;
    // Encode one out of (pos[k] - pos[j] + 1) values
    // (less some at either end because we must be able to fit
    // all the intervening pos in)
    const size_t outof = pos[k] - pos[j] + j - k + 1;
    // FIXME: look at replacing this with "fls()" (where available) or
    // some integer code.
    size_t bits = (size_t)ceil(log(double(outof)) / log(2.0));
    const size_t lowest = pos[j] + mid - j;
    const size_t spare = (1 << bits) - outof;
    const size_t mid_start = lowest + (outof - spare) / 2;
    Assert(pos[mid] >= lowest);
    Assert(pos[mid] < lowest + outof);
    if (spare) {
	if (pos[mid] < mid_start) {
	    wr.write_bits(pos[mid] - lowest, bits);
	} else if (pos[mid] >= mid_start + spare) {
	    wr.write_bits((pos[mid] - (mid_start + spare)) | (1 << (bits - 1)), bits);
	} else {
	    --bits;
	    wr.write_bits(pos[mid] - lowest, bits);
	}
    } else {
	wr.write_bits(pos[mid] - lowest, bits);
    }
    encode_interpolative(wr, pos, j, mid);
    encode_interpolative(wr, pos, mid, k);
}

class BitReader {
    private:
	string buf;
	size_t idx;
	int bits;
	unsigned int acc;
    public:
	BitReader(const string &buf_) : buf(buf_), idx(0), bits(0), acc(0) { }
	unsigned int read_bits(int count) {
	    unsigned int result;
	    if (count > 25) {
		// If we need more than 25 bits, read in two goes to ensure
		// that we don't overflow acc.  This is a little more
		// conservative than it needs to be, but such large values will
		// inevitably be rare (because you can't fit very many of them
		// into 2^32!)
		Assert(count <= 32);
		result = read_bits(16);
		return result | (read_bits(count - 16) << 16);
	    }
	    while (bits < count) {
		Assert(idx < buf.size());
		acc |= static_cast<unsigned char>(buf[idx++]) << bits;
		bits += 8;
	    }
	    result = acc & ((1u << count) - 1);
	    acc >>= count;
	    bits -= count;
	    return result;
	}
	// Check all the data has been read.  Because it'll be zero padded
	// to fill a byte, the best we can actually do is check that
	// there's less than a byte left and that all remaining bits are
	// zero.
	bool check_all_gone() const {
	    return (idx == buf.size() && bits < 7 && acc == 0);
	}
};

static void
decode_interpolative(BitReader & rd, vector<Xapian::termpos> & pos, int j, int k)
{
    if (j + 1 >= k) return;
    
    const size_t mid = (j + k) / 2;
    // Encode one out of (pos[k] - pos[j] + 1) values
    // (less some at either end because we must be able to fit
    // all the intervening pos in)
    const size_t outof = pos[k] - pos[j] + j - k + 1;
    size_t bits = (size_t)ceil(log(double(outof)) / log(2.0));
    const size_t lowest = pos[j] + mid - j;
    const size_t spare = (1 << bits) - outof;
    const size_t mid_start = lowest + (outof - spare) / 2;
    if (spare) {
	Xapian::termpos p = rd.read_bits(bits - 1);
	p += lowest;
	if (p < mid_start) {
	    if (rd.read_bits(1)) p += mid_start - lowest + spare;
	    pos[mid] = p;
	} else {
	    pos[mid] = p;
	}
    } else {
	pos[mid] = lowest + rd.read_bits(bits);
    }
    Assert(pos[mid] >= lowest);
    Assert(pos[mid] < lowest + outof);
    decode_interpolative(rd, pos, j, mid);
    decode_interpolative(rd, pos, mid, k);
}

void
FlintPositionListTable::set_positionlist(Xapian::docid did,
					 const string & tname,
					 Xapian::PositionIterator pos,
					 const Xapian::PositionIterator &pos_end)
{
    DEBUGCALL(DB, void, "FlintPositionList::set_positionlist",
	      did << ", " << tname << ", " << pos << ", " << pos_end);
    Assert(pos != pos_end);

    // FIXME: avoid the need for this copy!
    vector<Xapian::termpos> poscopy(pos, pos_end);

    string key = make_key(did, tname);

    if (poscopy.size() == 1) {
	// Special case for single entry position list.
	add(key, pack_uint(poscopy[0]));
	return;
    }

    BitWriter wr;
    string list_size = pack_uint(poscopy.size());
    for (size_t i = 0; i < list_size.size(); ++i) {
	wr.write_bits(list_size[i], 8);
    }
    encode_interpolative(wr, poscopy, 0, poscopy.size() - 1);
    add(key, wr.freeze());
}

///////////////////////////////////////////////////////////////////////////

void
FlintPositionList::read_data(const FlintTable * table, Xapian::docid did,
			     const string & tname)
{
    DEBUGCALL(DB, void, "FlintPositionList::read_data",
	      table << ", " << did << ", " << tname);

    have_started = false;
    positions.clear();

    string data;
    if (!table->get_exact_entry(pack_uint_preserving_sort(did) + tname, data)) {
	// There's no positional information for this term.
	current_pos = positions.begin();
	return;
    }

    const char * pos = data.data();
    const char * end = pos + data.size();
    Xapian::termpos positions_size;
    if (!unpack_uint(&pos, end, &positions_size)) {
	throw Xapian::DatabaseCorruptError("Position list data corrupt");
    }
    if (pos == end) {
	// Special case for single entry position list.
	positions.push_back(positions_size);
	current_pos = positions.begin();
	return;
    }
    positions.resize(positions_size);
    if (!unpack_uint(&pos, end, &positions[0])) {
	throw Xapian::DatabaseCorruptError("Position list data corrupt");
    }
    if (!unpack_uint(&pos, end, &positions.back())) {
	throw Xapian::DatabaseCorruptError("Position list data corrupt");
    }
    BitReader rd(data);
    // Skip the header we just read.
    (void)rd.read_bits(8 * (pos - data.data()));
    decode_interpolative(rd, positions, 0, positions_size - 1);

    current_pos = positions.begin();
}

Xapian::termcount
FlintPositionList::get_size() const
{
    DEBUGCALL(DB, Xapian::termcount, "FlintPositionList::get_size", "");
    RETURN(positions.size());
}

Xapian::termpos
FlintPositionList::get_position() const
{
    DEBUGCALL(DB, Xapian::termpos, "FlintPositionList::get_position", "");
    Assert(have_started);
    RETURN(*current_pos);
}

void
FlintPositionList::next()
{
    DEBUGCALL(DB, void, "FlintPositionList::next", "");
    Assert(!at_end());

    if (!have_started) {
	have_started = true;
    } else {
	++current_pos;
    }
}

void
FlintPositionList::skip_to(Xapian::termpos termpos)
{
    DEBUGCALL(DB, void, "FlintPositionList::skip_to", termpos);
    if (!have_started) {
	have_started = true;
    }
    while (!at_end() && *current_pos < termpos) ++current_pos;
}

bool
FlintPositionList::at_end() const
{
    DEBUGCALL(DB, bool, "FlintPositionList::at_end", "");
    RETURN(current_pos == positions.end());
}
