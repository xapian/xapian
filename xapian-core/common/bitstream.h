/** @file bitstream.h
 * @brief Classes to encode/decode a bitstream.
 */
/* Copyright (C) 2004,2005,2006,2008,2012,2013,2014 Olly Betts
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

#include <string>
#include <vector>

namespace Xapian {

/// Create a stream to which non-byte-aligned values can be written.
class BitWriter {
    std::string buf;
    int n_bits;
    unsigned int acc;

  public:
    /// Construct empty.
    BitWriter() : n_bits(0), acc(0) { }

    /// Construct with the contents of seed already in the stream.
    explicit BitWriter(const std::string & seed)
	: buf(seed), n_bits(0), acc(0) { }

    /// Encode value, known to be less than outof.
    void encode(size_t value, size_t outof);

    /// Finish encoding and return the encoded data as a std::string.
    std::string & freeze() {
	if (n_bits) {
	    buf += char(acc);
	    n_bits = 0;
	    acc = 0;
	}
	return buf;
    }

    /// Perform interpolative encoding of pos elements between j and k.
    void encode_interpolative(const std::vector<Xapian::termpos> &pos, int j, int k);
};

/// Read a stream created by BitWriter.
class BitReader {
    std::string buf;
    size_t idx;
    int n_bits;
    unsigned int acc;

    unsigned int read_bits(int count);

    struct DIStack {
	int j, k;
	Xapian::termpos pos_k;
    };

    struct DIState : public DIStack {
	Xapian::termpos pos_j;

	void set_j(int j_, Xapian::termpos pos_j_) {
	    j = j_;
	    pos_j = pos_j_;
	}
	void set_k(int k_, Xapian::termpos pos_k_) {
	    k = k_;
	    pos_k = pos_k_;
	}
	void uninit() {
	    j = 1;
	    k = 0;
	}
	DIState() { uninit(); }
	DIState(int j_, int k_,
		Xapian::termpos pos_j_, Xapian::termpos pos_k_) {
	    set_j(j_, pos_j_);
	    set_k(k_, pos_k_);
	}
	void operator=(const DIStack & o) {
	    j = o.j;
	    set_k(o.k, o.pos_k);
	}
	bool is_next() const { return j + 1 < k; }
	bool is_initialized() const {
	    return j <= k;
	}
	// Given pos[j] = pos_j and pos[k] = pos_k, how many possible position
	// values are there for the value midway between?
	Xapian::termpos outof() const {
	    return pos_k - pos_j + j - k + 1;
	}
    };

    std::vector<DIStack> di_stack;
    DIState di_current;

  public:
    // Construct.
    BitReader() { }

    // Construct with the contents of buf_.
    explicit BitReader(const std::string &buf_)
	: buf(buf_), idx(0), n_bits(0), acc(0) { }

    // Construct with the contents of buf_, skipping some bytes.
    BitReader(const std::string &buf_, size_t skip)
	: buf(buf_, skip), idx(0), n_bits(0), acc(0) { }

    // Initialise from buf_, optionally skipping some bytes.
    void init(const std::string &buf_, size_t skip = 0) {
	buf.assign(buf_, skip, std::string::npos);
	idx = 0;
	n_bits = 0;
	acc = 0;
	di_stack.clear();
	di_current.uninit();
    }

    // Decode value, known to be less than outof.
    Xapian::termpos decode(Xapian::termpos outof, bool force = false);

    // Check all the data has been read.  Because it'll be zero padded
    // to fill a byte, the best we can actually do is check that
    // there's less than a byte left and that all remaining bits are
    // zero.
    bool check_all_gone() const {
	return (idx == buf.size() && n_bits <= 7 && acc == 0);
    }

    /// Perform interpolative decoding between elements between j and k.
    void decode_interpolative(int j, int k,
			      Xapian::termpos pos_j, Xapian::termpos pos_k);

    /// Perform on-demand interpolative decoding.
    Xapian::termpos decode_interpolative_next();
};

}

using Xapian::BitWriter;
using Xapian::BitReader;

#endif // XAPIAN_INCLUDED_BITSTREAM_H
