/* flint_positionlist.cc: A position list in a flint database.
 *
 * Copyright (C) 2004,2005,2006 Olly Betts
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

const static unsigned char flstab[256] = {
    0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
};

// Highly optimised fls() implementation.
inline int my_fls(unsigned mask)
{
    int result = 0;
    if (mask > 0x10000u) {
	mask >>= 16;
	result = 16;
    }
    if (mask > 0x100u) {
	mask >>= 8;
	result += 8;
    }
    return result + flstab[mask];
}

class BitWriter {
    private:
	string buf;
	int n_bits;
	unsigned int acc;
    public:
	BitWriter() : n_bits(0), acc(0) { }
	BitWriter(const string & seed) : buf(seed), n_bits(0), acc(0) { }
	void encode(size_t value, size_t outof) {
	    Assert(value < outof);
	    size_t bits = my_fls(outof);
	    const size_t spare = (1 << bits) - outof;
	    if (spare) {
		const size_t mid_start = (outof - spare) / 2;
		if (value < mid_start) {
		    write_bits(value, bits);
		} else if (value >= mid_start + spare) {
		    write_bits((value - (mid_start + spare)) | (1 << (bits - 1)), bits);
		} else {
		    --bits;
		    write_bits(value, bits);
		}
	    } else {
		write_bits(value, bits);
	    }
	}
	void write_bits(int data, int count) {
	    if (count + n_bits > 32) {
		// We need to write more bits than there's empty room for in
		// the accumulator.  So we arrange to shift out 8 bits, then
		// adjust things so we're adding 8 fewer bits.
		Assert(count <= 32);
		acc |= (data << n_bits);
		buf += char(acc);
		acc >>= 8;
		data >>= 8;
		count -= 8;
	    }
	    acc |= (data << n_bits);
	    n_bits += count;
	    while (n_bits >= 8) {
		buf += char(acc);
		acc >>= 8;
		n_bits -= 8;
	    }
	}
	string & freeze() {
	    if (n_bits) {
		buf += char(acc);
		n_bits = 0;
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
    const size_t lowest = pos[j] + mid - j;
    wr.encode(pos[mid] - lowest, outof);
    encode_interpolative(wr, pos, j, mid);
    encode_interpolative(wr, pos, mid, k);
}

class BitReader {
    private:
	string buf;
	size_t idx;
	int n_bits;
	unsigned int acc;
    public:
	BitReader(const string &buf_) : buf(buf_), idx(0), n_bits(0), acc(0) { }
	Xapian::termpos decode(Xapian::termpos outof) {
	    size_t bits = my_fls(outof);
	    const size_t spare = (1 << bits) - outof;
	    const size_t mid_start = (outof - spare) / 2;
	    Xapian::termpos p;
	    if (spare) {
		p = read_bits(bits - 1);
		if (p < mid_start) {
		    if (read_bits(1)) p += mid_start + spare;
		}
	    } else {
		p = read_bits(bits);
	    }
	    Assert(p < outof);
	    return p;
	}
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
	    while (n_bits < count) {
		Assert(idx < buf.size());
		acc |= static_cast<unsigned char>(buf[idx++]) << n_bits;
		n_bits += 8;
	    }
	    result = acc & ((1u << count) - 1);
	    acc >>= count;
	    n_bits -= count;
	    return result;
	}
	// Check all the data has been read.  Because it'll be zero padded
	// to fill a byte, the best we can actually do is check that
	// there's less than a byte left and that all remaining bits are
	// zero.
	bool check_all_gone() const {
	    return (idx == buf.size() && n_bits < 7 && acc == 0);
	}
};

static void
decode_interpolative(BitReader & rd, vector<Xapian::termpos> & pos, int j, int k)
{
    if (j + 1 >= k) return;

    const size_t mid = (j + k) / 2;
    // Decode one out of (pos[k] - pos[j] + 1) values
    // (less some at either end because we must be able to fit
    // all the intervening pos in)
    const size_t outof = pos[k] - pos[j] + j - k + 1;
    pos[mid] = rd.decode(outof) + (pos[j] + mid - j);
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

    BitWriter wr(pack_uint(poscopy.back()));

    wr.encode(poscopy[0], poscopy.back());
    wr.encode(poscopy.size() - 2, poscopy.back() - poscopy[0]);
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
    Xapian::termpos pos_last;
    if (!unpack_uint(&pos, end, &pos_last)) {
	throw Xapian::DatabaseCorruptError("Position list data corrupt");
    }
    if (pos == end) {
	// Special case for single entry position list.
	positions.push_back(pos_last);
	current_pos = positions.begin();
	return;
    }
    BitReader rd(data);
    // Skip the header we just read.
    (void)rd.read_bits(8 * (pos - data.data()));
    Xapian::termpos pos_first = rd.decode(pos_last);
    Xapian::termpos pos_size = rd.decode(pos_last - pos_first) + 2;
    positions.resize(pos_size);
    positions[0] = pos_first;
    positions.back() = pos_last;
    decode_interpolative(rd, positions, 0, pos_size - 1);

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

    if (!have_started) {
	have_started = true;
    } else {
	Assert(!at_end());
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
