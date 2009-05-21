/** @file bitstream.cc
 * @brief Classes to encode/decode a bitstream.
 */
/* Copyright (C) 2004,2005,2006,2008 Olly Betts
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

#include "bitstream.h"

#include <xapian/types.h>

#include "omassert.h"

#include <cmath>
#include <vector>

using namespace std;

static const unsigned char flstab[256] = {
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
    if (mask >= 0x10000u) {
	mask >>= 16;
	result = 16;
    }
    if (mask >= 0x100u) {
	mask >>= 8;
	result += 8;
    }
    return result + flstab[mask];
}

namespace Xapian {

void
BitWriter::encode(size_t value, size_t outof)
{
    Assert(value < outof);
    size_t bits = my_fls(outof - 1);
    const size_t spare = (1 << bits) - outof;
    if (spare) {
	const size_t mid_start = (outof - spare) / 2;
	if (value >= mid_start + spare) {
	    value = (value - (mid_start + spare)) | (1 << (bits - 1));
	} else if (value >= mid_start) {
	    --bits;
	}
    }

    if (bits + n_bits > 32) {
	// We need to write more bits than there's empty room for in
	// the accumulator.  So we arrange to shift out 8 bits, then
	// adjust things so we're adding 8 fewer bits.
	Assert(bits <= 32);
	acc |= (value << n_bits);
	buf += char(acc);
	acc >>= 8;
	value >>= 8;
	bits -= 8;
    }
    acc |= (value << n_bits);
    n_bits += bits;
    while (n_bits >= 8) {
	buf += char(acc);
	acc >>= 8;
	n_bits -= 8;
    }
}

void
BitWriter::encode_interpolative(const vector<Xapian::termpos> &pos, int j, int k)
{
    while (j + 1 < k) {
	const size_t mid = (j + k) / 2;
	// Encode one out of (pos[k] - pos[j] + 1) values
	// (less some at either end because we must be able to fit
	// all the intervening pos in)
	const size_t outof = pos[k] - pos[j] + j - k + 1;
	const size_t lowest = pos[j] + mid - j;
	encode(pos[mid] - lowest, outof);
	encode_interpolative(pos, j, mid);
	j = mid;
    }
}

Xapian::termpos
BitReader::decode(Xapian::termpos outof)
{
    size_t bits = my_fls(outof - 1);
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

unsigned int
BitReader::read_bits(int count)
{
    unsigned int result;
    if (count > 25) {
	// If we need more than 25 bits, read in two goes to ensure that we
	// don't overflow acc.  This is a little more conservative than it
	// needs to be, but such large values will inevitably be rare (because
	// you can't fit very many of them into 2^32!)
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

void
BitReader::decode_interpolative(vector<Xapian::termpos> & pos, int j, int k)
{
    while (j + 1 < k) {
	const size_t mid = (j + k) / 2;
	// Decode one out of (pos[k] - pos[j] + 1) values
	// (less some at either end because we must be able to fit
	// all the intervening pos in)
	const size_t outof = pos[k] - pos[j] + j - k + 1;
	pos[mid] = decode(outof) + (pos[j] + mid - j);
	decode_interpolative(pos, j, mid);
	j = mid;
    }
}

}
