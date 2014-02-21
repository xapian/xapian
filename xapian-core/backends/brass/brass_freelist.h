/** @file brass_freelist.h
 * @brief Brass freelist
 */
/* Copyright 2014 Olly Betts
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

#ifndef XAPIAN_INCLUDED_BRASS_FREELIST_H
#define XAPIAN_INCLUDED_BRASS_FREELIST_H

#include "brass_types.h"
#include "pack.h"

class BrassTable;

class BrassFLCursor {
  public:
    /// Block number of current freelist chunk.
    uint4 n;

    /// Current offset in block.
    unsigned c;

    BrassFLCursor() : n(0), c(0) { }

    bool operator==(const BrassFLCursor & o) const {
	return n == o.n && c == o.c;
    }

    bool operator!=(const BrassFLCursor & o) const {
	return !(*this == o);
    }

    void swap(BrassFLCursor &o) {
	std::swap(n, o.n);
	std::swap(c, o.c);
    }

    void pack(std::string & buf) {
	pack_uint(buf, n);
	pack_uint(buf, c / 4);
    }

    bool unpack(const char ** p, const char * end) {
	bool r = unpack_uint(p, end, &n) && unpack_uint(p, end, &c);
	if (usual(r))
	    c *= 4;
	return r;
    }
};

class BrassFreeList {
    BrassFreeList(const BrassFreeList &);

    void operator=(const BrassFreeList &);

    void read_block(BrassTable * B, uint4 n, byte * p);

    void write_block(BrassTable * B, uint4 n, byte * p);

  protected:
    uint4 revision;

    uint4 block_size;

    uint4 first_unused_block;

    BrassFLCursor fl, fl_end, flw;

    bool flw_appending;

  private:
    /// Current freelist block.
    byte * p;

    /// Current freelist block we're writing.
    byte * pw;

  public:
    BrassFreeList()
	: revision(0), block_size(0), first_unused_block(0),
	  flw_appending(false), p(0), pw(0) { }

    ~BrassFreeList() { delete [] p; delete [] pw; }

    bool empty() const { return fl == fl_end; }

    uint4 get_block(BrassTable * B);

    uint4 walk(BrassTable *B, bool inclusive);

    void mark_block_unused(BrassTable * B, uint4 n);

    uint4 get_revision() const { return revision; }
    void set_revision(uint4 revision_) { revision = revision_; }

    uint4 get_block_size() const { return block_size; }
    void set_block_size(uint4 block_size_) { block_size = block_size_; }

    uint4 get_first_unused_block() const { return first_unused_block; }

    void commit(BrassTable * B);

    void swap(BrassFreeList &o) {
	std::swap(revision, o.revision);
	std::swap(block_size, o.block_size);
	std::swap(first_unused_block, o.first_unused_block);
	std::swap(fl, o.fl);
	std::swap(fl_end, o.fl_end);
	std::swap(flw, o.flw);
	std::swap(p, o.p);
	std::swap(pw, o.pw);
    }

    void pack(std::string & buf) {
	pack_uint(buf, revision);
	pack_uint(buf, block_size / 2048);
	pack_uint(buf, first_unused_block);
	fl.pack(buf);
	flw.pack(buf);
    }

    bool unpack(const char ** pstart, const char * end) {
	bool r = unpack_uint(pstart, end, &revision) &&
		 unpack_uint(pstart, end, &block_size) &&
		 unpack_uint(pstart, end, &first_unused_block) &&
		 fl.unpack(pstart, end) &&
		 flw.unpack(pstart, end);
	if (r) {
	    fl_end = flw;
	    block_size *= 2048;
	}
	return r;
    }
};

class BrassFreeListChecker {
    // FIXME: uint_fast32_t is probably a good choice.
    typedef unsigned long elt_type;

    uint4 bitmap_size;

    elt_type * bitmap;


  public:
    BrassFreeListChecker(const BrassFreeList & fl);

    ~BrassFreeListChecker() {
	delete [] bitmap;
    }

    bool mark_used(uint4 n) {
	const unsigned BITS_PER_ELT = sizeof(elt_type) * 8;
	elt_type mask = static_cast<elt_type>(1) << (n & (BITS_PER_ELT - 1));
	n /= BITS_PER_ELT;
	if (rare(n >= bitmap_size)) return false;
	if ((bitmap[n] & mask) == 0) return false;
	bitmap[n] &= ~mask;
	return true;
    }

    /// Count how many bits are still set.
    uint4 count_set_bits(uint4 * p_first_bad_blk) const;
};

#endif // XAPIAN_INCLUDED_BRASS_FREELIST_H
