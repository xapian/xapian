/** @file bitstream.cc
 * @brief Classes to encode/decode a bitstream.
 */
/* Copyright (C) 2004,2005,2006,2008,2013,2014,2016 Olly Betts
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

// Highly optimised fls() implementation.
inline int highest_order_bit(unsigned mask)
{
#if HAVE_DECL___BUILTIN_CLZ
    return mask ? 32 - __builtin_clz(mask) : 0;
#else
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
#endif
}

namespace Xapian {

void
BitWriter::encode(size_t value, size_t outof)
{
    Assert(value < outof);
    size_t bits = highest_order_bit(outof - 1);
    const size_t spare = (1 << bits) - outof;
    if (spare) {
	/* If we have spare values, we can use one fewer bit to encode some
	 * values.  We shorten the values in the middle of the range, as
	 * testing (on positional data) shows this works best.  "Managing
	 * Gigabytes" suggests reversing this for the lowest level and encoding
	 * the end values of the range shorter, which is contrary to our
	 * testing (MG is talking about posting lists, which probably have
	 * different characteristics).
	 *
	 * For example, if outof is 11, the codes emitted are:
	 *
	 * value	output
	 * 0		0000
	 * 1		0001
	 * 2		0010
	 * 3		 011
	 * 4		 100
	 * 5		 101
	 * 6		 110
	 * 7		 111
	 * 8		1000
	 * 9		1001
	 * 10		1010
	 *
	 * Note the LSB comes first in the bitstream, so these codes need to be
	 * suffix-free to be decoded.
	 */
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
    // "Interpolative code" - for an algorithm description, see "Managing
    // Gigabytes" - pages 126-127 in the second edition.  You can probably
    // view those pages in google books.
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
BitReader::decode(Xapian::termpos outof, bool force)
{
    (void)force;
    Assert(force == di_current.is_initialized());
    size_t bits = highest_order_bit(outof - 1);
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
BitReader::decode_interpolative(int j, int k,
				Xapian::termpos pos_j, Xapian::termpos pos_k)
{
    Assert(!di_current.is_initialized());
    di_stack.reserve(highest_order_bit(pos_k - pos_j));
    di_current.set_j(j, pos_j);
    di_current.set_k(k, pos_k);
}

Xapian::termpos
BitReader::decode_interpolative_next()
{
    Assert(di_current.is_initialized());
    while (!di_stack.empty() || di_current.is_next()) {
	if (!di_current.is_next()) {
	    Xapian::termpos pos_ret = di_current.pos_k;
	    di_current = di_stack.back();
	    di_stack.pop_back();
	    int mid = (di_current.j + di_current.k) / 2;
	    di_current.set_j(mid, pos_ret);
	    return pos_ret;
	}
	di_stack.push_back(di_current);
	int mid = (di_current.j + di_current.k) / 2;
	int pos_mid = decode(di_current.outof(), true) +
	    (di_current.pos_j + mid - di_current.j);
	di_current.set_k(mid, pos_mid);
    }
#ifdef XAPIAN_ASSERTIONS
    di_current.uninit();
#endif
    return di_current.pos_k;
}

}
