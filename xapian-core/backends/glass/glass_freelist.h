/** @file glass_freelist.h
 * @brief Glass freelist
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

#ifndef XAPIAN_INCLUDED_GLASS_FREELIST_H
#define XAPIAN_INCLUDED_GLASS_FREELIST_H

#include "glass_defs.h"
#include "pack.h"

class GlassTable;

class GlassFLCursor {
  public:
    /// Block number of current freelist chunk.
    uint4 n;

    /// Current offset in block.
    unsigned c;

    GlassFLCursor() : n(0), c(0) { }

    bool operator==(const GlassFLCursor & o) const {
	return n == o.n && c == o.c;
    }

    bool operator!=(const GlassFLCursor & o) const {
	return !(*this == o);
    }

    void swap(GlassFLCursor &o) {
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

class GlassFreeList {
    GlassFreeList(const GlassFreeList &);

    void operator=(const GlassFreeList &);

    void read_block(const GlassTable * B, uint4 n, byte * p);

    void write_block(const GlassTable * B, uint4 n, byte * p, uint4 rev);

  protected:
    uint4 revision;

    uint4 first_unused_block;

    GlassFLCursor fl, fl_end, flw;

    bool flw_appending;

  private:
    /// Current freelist block.
    byte * p;

    /// Current freelist block we're writing.
    byte * pw;

  public:
    GlassFreeList() {
	revision = 0;
	first_unused_block = 0;
	flw_appending = false;
	p = pw = NULL;
    }

    void reset() {
	revision = 0;
	first_unused_block = 0;
	flw_appending = false;
    }

    ~GlassFreeList() { delete [] p; delete [] pw; }

    bool empty() const { return fl == fl_end; }

    uint4 get_block(const GlassTable * B, uint4 block_size,
		    uint4 * blk_to_free = NULL);

    uint4 walk(const GlassTable *B, uint4 block_size, bool inclusive);

    void mark_block_unused(const GlassTable * B, uint4 block_size, uint4 n);

    uint4 get_revision() const { return revision; }
    void set_revision(uint4 revision_) { revision = revision_; }

    uint4 get_first_unused_block() const { return first_unused_block; }

    // Used when compacting to a single file.
    void set_first_unused_block(uint4 base) { first_unused_block = base; }

    void commit(const GlassTable * B, uint4 block_size);

    void pack(std::string & buf) {
	pack_uint(buf, revision);
	pack_uint(buf, first_unused_block);
	fl.pack(buf);
	flw.pack(buf);
    }

    bool unpack(const char ** pstart, const char * end) {
	bool r = unpack_uint(pstart, end, &revision) &&
		 unpack_uint(pstart, end, &first_unused_block) &&
		 fl.unpack(pstart, end) &&
		 flw.unpack(pstart, end);
	if (r) {
	    fl_end = flw;
	    flw_appending = false;
	}
	return r;
    }

    bool unpack(const std::string & s) {
	const char * ptr = s.data();
	const char * end = ptr + s.size();
	return unpack(&ptr, end) && ptr == end;
    }
};

class GlassFreeListChecker {
    // FIXME: uint_fast32_t is probably a good choice.
    typedef unsigned long elt_type;

    uint4 bitmap_size;

    elt_type * bitmap;

    // Prevent copying
    GlassFreeListChecker(const GlassFreeListChecker&);
    GlassFreeListChecker& operator=(const GlassFreeListChecker&);

  public:
    explicit GlassFreeListChecker(const GlassFreeList & fl);

    ~GlassFreeListChecker() {
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

#endif // XAPIAN_INCLUDED_GLASS_FREELIST_H
