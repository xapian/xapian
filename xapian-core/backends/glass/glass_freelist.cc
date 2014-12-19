/** @file glass_freelist.cc
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

#include <config.h>

#include "glass_freelist.h"

#include "glass_table.h"
#include "xapian/error.h"

#include "unaligned.h"
#include <cstring>

using namespace std;

/** The first offset to use for storing free block info.
 *
 *  The first 4 bytes store the revision.  The next byte (which is the level
 *  for a block in the B-tree) is set to LEVEL_FREELIST to mark this as a
 *  freelist block).
 */
const unsigned C_BASE = 8;

void
GlassFreeList::read_block(const GlassTable * B, uint4 n, byte * ptr)
{
    B->read_block(n, ptr);
    if (rare(GET_LEVEL(ptr) != LEVEL_FREELIST))
	throw Xapian::DatabaseCorruptError("Freelist corrupt");
}

void
GlassFreeList::write_block(const GlassTable * B, uint4 n, byte * ptr, uint4 rev)
{
    SET_REVISION(ptr, rev);
    setint4(ptr, 4, 0);
    SET_LEVEL(ptr, LEVEL_FREELIST);
    B->write_block(n, ptr, flw_appending);
}

uint4
GlassFreeList::get_block(const GlassTable *B, uint4 block_size,
			 uint4 * blk_to_free)
{
    if (fl == fl_end) {
	return first_unused_block++;
    }

    if (p == 0 || fl.c == block_size - 4) {
	if (p == 0) {
	    p = new byte[block_size];
	    read_block(B, fl.n, p);
	} else {
	    // Delay handling marking old block as unused until after we've
	    // started a new one.
	    uint4 old_fl_blk = fl.n;

	    fl.n = getint4(p, fl.c);
	    // Allow for mini-header at start of freelist block.
	    fl.c = C_BASE;
	    read_block(B, fl.n, p);
	    if (blk_to_free) {
		Assert(*blk_to_free == BLK_UNUSED);
		*blk_to_free = old_fl_blk;
	    } else {
		mark_block_unused(B, block_size, old_fl_blk);
	    }
	}

	// Either the freelist end is in this block, or this freelist block has
	// a next pointer.
	Assert(fl.n == fl_end.n || getint4(p, block_size - 4) != -1);

	return get_block(B, block_size);
    }

    // Either the freelist end is in this block, or this freelist block has a
    // next pointer.
    Assert(fl.n == fl_end.n || getint4(p, block_size - 4) != -1);

    uint4 blk = getint4(p, fl.c);
    if (blk == uint4(-1))
	throw Xapian::DatabaseCorruptError("Ran off end of freelist (" + str(fl.n) + ", " + str(fl.c) + ")");
    fl.c += 4;

    return blk;
}

uint4
GlassFreeList::walk(const GlassTable *B, uint4 block_size, bool inclusive)
{
    if (fl == fl_end) {
	// It's expected that the caller checks !empty() first.
	return static_cast<uint4>(-1);
    }

    if (p == 0 || fl.c == block_size - 4) {
	if (p == 0) {
	    p = new byte[block_size];
	} else {
	    fl.n = getint4(p, fl.c);
	    // Allow for mini-header at start of freelist block.
	    fl.c = C_BASE;
	}
	read_block(B, fl.n, p);

	// Either the freelist end is in this block, or this freelist block has
	// a next pointer.
	Assert(fl.n == fl_end.n || getint4(p, block_size - 4) != -1);

	if (inclusive)
	    return fl.n;
	return walk(B, block_size, inclusive);
    }

    // Either the freelist end is in this block, or this freelist block has a
    // next pointer.
    Assert(fl.n == fl_end.n || getint4(p, block_size - 4) != -1);

    uint4 blk = getint4(p, fl.c);
    fl.c += 4;

    return blk;
}

void
GlassFreeList::mark_block_unused(const GlassTable * B, uint4 block_size, uint4 blk)
{
    // If the current flw block is full, we need to call get_block(), and if
    // the returned block is the last entry in its freelist block, that block
    // needs to be marked as unused.  The recursion this would create is
    // problematic, so we instead note down that block and mark it as unused
    // once we've processed the original request.
    uint4 blk_to_free = BLK_UNUSED;

    if (!pw) {
	pw = new byte[block_size];
	if (flw.c != 0) {
	    read_block(B, flw.n, pw);
	    flw_appending = true;
	}
    }
    if (flw.c == 0) {
	uint4 n = get_block(B, block_size, &blk_to_free);
	flw.n = n;
	flw.c = C_BASE;
	if (fl.c == 0) {
	    fl = fl_end = flw;
	}
	flw_appending = (n == first_unused_block - 1);
	setint4(pw, block_size - 4, -1);
    } else if (flw.c == block_size - 4) {
	// blk is free *after* the current revision gets released, so we can't
	// just use blk as the next block in the freelist chain.
	uint4 n = get_block(B, block_size, &blk_to_free);
	setint4(pw, flw.c, n);
	write_block(B, flw.n, pw, revision + 1);
	if (p && flw.n == fl.n) {
	    // FIXME: share and refcount?
	    memcpy(p, pw, block_size);
	}
	flw.n = n;
	flw.c = C_BASE;
	flw_appending = (n == first_unused_block - 1);
	setint4(pw, block_size - 4, -1);
    }

    setint4(pw, flw.c, blk);
    flw.c += 4;

    if (blk_to_free != BLK_UNUSED)
	mark_block_unused(B, block_size, blk_to_free);
}

void
GlassFreeList::commit(const GlassTable * B, uint4 block_size)
{
    if (pw && flw.c != 0) {
	memset(pw + flw.c, 255, block_size - flw.c - 4);
	write_block(B, flw.n, pw, revision);
	if (p && flw.n == fl.n) {
	    // FIXME: share and refcount?
	    memcpy(p, pw, block_size);
	}
	flw_appending = true;
	fl_end = flw;
    }
}

GlassFreeListChecker::GlassFreeListChecker(const GlassFreeList & fl)
{
    const unsigned BITS_PER_ELT = sizeof(elt_type) * 8;
    const elt_type ALL_BITS = static_cast<elt_type>(-1);
    uint4 first_unused = fl.get_first_unused_block();
    bitmap_size = (first_unused + BITS_PER_ELT - 1) / BITS_PER_ELT;
    bitmap = new elt_type[bitmap_size];
    std::fill_n(bitmap, bitmap_size - 1, ALL_BITS);
    // Only set the bits in the final element of bitmap which correspond to
    // blocks < first_unused.
    uint4 remainder = first_unused & (BITS_PER_ELT - 1);
    if (remainder)
	bitmap[bitmap_size - 1] = (static_cast<elt_type>(1) << remainder) - 1;
    else
	bitmap[bitmap_size - 1] = ALL_BITS;
}

uint4
GlassFreeListChecker::count_set_bits(uint4 * p_first_bad_blk) const
{
    const unsigned BITS_PER_ELT = sizeof(elt_type) * 8;
    uint4 c = 0;
    for (uint4 i = 0; i < bitmap_size; ++i) {
	elt_type elt = bitmap[i];
	if (usual(elt == 0))
	    continue;
	if (c == 0 && p_first_bad_blk) {
	    uint4 first_bad_blk = i * BITS_PER_ELT;
#if defined __GNUC__
#if __GNUC__ * 100 + __GNUC_MINOR__ >= 304
	    // GCC 3.4 added __builtin_ctz() (with l and ll variants).
	    if (sizeof(elt_type) == sizeof(unsigned))
		first_bad_blk += __builtin_ctz(elt);
	    else if (sizeof(elt_type) == sizeof(unsigned long))
		first_bad_blk += __builtin_ctzl(elt);
	    else if (sizeof(elt_type) == sizeof(unsigned long long))
		first_bad_blk += __builtin_ctzll(elt);
#else
	    // GCC has had __builtin_ffs() in all versions we support, which
	    // returns one more than ctz (and is defined for an input of 0,
	    // which we don't need).
	    if (sizeof(elt_type) == sizeof(unsigned))
		first_bad_blk += __builtin_ffs(elt) - 1;
#endif
	    else
#endif
	    {
		for (elt_type mask = 1; (elt & mask) == 0; mask <<= 1) {
		    ++first_bad_blk;
		}
	    }
	    *p_first_bad_blk = first_bad_blk;
	}

	// Count set bits in elt.
	// GCC 3.4 added __builtin_popcount and variants.
#if defined __GNUC__ && __GNUC__ * 100 + __GNUC_MINOR__ >= 304
	if (sizeof(elt_type) == sizeof(unsigned))
	    c += __builtin_popcount(elt);
	else if (sizeof(elt_type) == sizeof(unsigned long))
	    c += __builtin_popcountl(elt);
	else if (sizeof(elt_type) == sizeof(unsigned long long))
	    c += __builtin_popcountll(elt);
	else
#elif defined _MSC_VER
	if (sizeof(elt_type) == sizeof(unsigned))
	    c += __popcnt(elt);
	else if (sizeof(elt_type) == sizeof(__int64))
	    c += __popcnt64(elt);
	else
#endif
	{
	    do {
		++c;
		elt &= elt - 1;
	    } while (elt);
	}
    }
    return c;
}
