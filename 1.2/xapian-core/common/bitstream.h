/** @file bitstream.h
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

#ifndef XAPIAN_INCLUDED_BITSTREAM_H
#define XAPIAN_INCLUDED_BITSTREAM_H

#include <xapian/types.h>
#include <xapian/visibility.h>

#include <string>
#include <vector>

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT BitWriter {
    std::string buf;
    int n_bits;
    unsigned int acc;

  public:
    BitWriter() : n_bits(0), acc(0) { }

    BitWriter(const std::string & seed) : buf(seed), n_bits(0), acc(0) { }

    void encode(size_t value, size_t outof);

    std::string & freeze() {
	if (n_bits) {
	    buf += char(acc);
	    n_bits = 0;
	    acc = 0;
	}
	return buf;
    }

    void encode_interpolative(const std::vector<Xapian::termpos> &pos, int j, int k);
};

class XAPIAN_VISIBILITY_DEFAULT BitReader {
    std::string buf;
    size_t idx;
    int n_bits;
    unsigned int acc;

    unsigned int read_bits(int count);

  public:
    BitReader(const std::string &buf_)
	: buf(buf_), idx(0), n_bits(0), acc(0) { }

    BitReader(const std::string &buf_, size_t skip)
	: buf(buf_, skip), idx(0), n_bits(0), acc(0) { }

    Xapian::termpos decode(Xapian::termpos outof);

    // Check all the data has been read.  Because it'll be zero padded
    // to fill a byte, the best we can actually do is check that
    // there's less than a byte left and that all remaining bits are
    // zero.
    bool check_all_gone() const {
	return (idx == buf.size() && n_bits <= 7 && acc == 0);
    }

    void decode_interpolative(std::vector<Xapian::termpos> & pos, int j, int k);
};

}

using Xapian::BitWriter;
using Xapian::BitReader;

#endif // XAPIAN_INCLUDED_BITSTREAM_H
