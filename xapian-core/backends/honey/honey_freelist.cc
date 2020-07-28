/** @file honey_freelist.cc
 * @brief Honey freelist
 */
/* Copyright 2014,2015,2016,2020 Olly Betts
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

#include "honey_freelist.h"

#include "honey_table.h"
#include "xapian/error.h"

#include "omassert.h"
#include "popcount.h"
#include "wordaccess.h"
#include <cstring>

using namespace std;
using namespace Honey;

// Allow forcing the freelist to be shorter to tickle bugs.
// FIXME: Sort out a way we can set this dynamically while running the
// testsuite.
#ifdef HONEY_FREELIST_SIZE
# define FREELIST_END_ \
    (8 + (HONEY_FREELIST_SIZE < 3 ? 3 : HONEY_FREELIST_SIZE) * 4)
# define FREELIST_END (FREELIST_END_ < 2048 ? FREELIST_END_ : 2048)
#else
# define FREELIST_END block_size
#endif

/** The first offset to use for storing free block info.
 *
 *  The first 4 bytes store the revision.  The next byte (which is the level
 *  for a block in the B-tree) is set to LEVEL_FREELIST to mark this as a
 *  freelist block).
 */
const unsigned C_BASE = 8;

/// Invalid freelist block value, so we can detect overreading bugs, etc.
const uint4 UNUSED = static_cast<uint4>(-1);

void
HoneyFreeList::read_block(const HoneyTable* B, uint4 n, uint8_t* ptr)
{
#ifdef SST_SEARCH
    (void)B; (void)n; (void)ptr;
#else
    B->read_block(n, ptr);
    if (rare(GET_LEVEL(ptr) != LEVEL_FREELIST))
	throw Xapian::DatabaseCorruptError("Freelist corrupt");
#endif
}

void
HoneyFreeList::write_block(const HoneyTable* B, uint4 n, uint8_t* ptr,
			   uint4 rev)
{
#ifdef SST_SEARCH
    (void)B; (void)n; (void)ptr; (void)rev;
#else
    SET_REVISION(ptr, rev);
    aligned_write4(ptr + 4, 0);
    SET_LEVEL(ptr, LEVEL_FREELIST);
    B->write_block(n, ptr, flw_appending);
#endif
}

uint4
HoneyFreeList::get_block(const HoneyTable* B, uint4 block_size,
			 uint4 * blk_to_free)
{
    if (fl == fl_end) {
	return first_unused_block++;
    }

    if (p == 0) {
	if (fl.n == UNUSED) {
	    throw Xapian::DatabaseCorruptError("Freelist pointer invalid");
	}
	// Actually read the current freelist block.
	p = new uint8_t[block_size];
	read_block(B, fl.n, p);
    }

    // Either the freelist end is in this block, or this freelist block has a
    // next pointer.
    Assert(fl.n == fl_end.n || aligned_read4(p + FREELIST_END - 4) != UNUSED);

    if (fl.c != FREELIST_END - 4) {
	uint4 blk = aligned_read4(p + fl.c);
	if (blk == UNUSED) {
	    throw Xapian::DatabaseCorruptError("Ran off end of freelist (" +
					       str(fl.n) + ", " + str(fl.c) +
					       ")");
	}
	fl.c += 4;
	return blk;
    }

    // Delay handling marking old block as unused until after we've
    // started a new one.
    uint4 old_fl_blk = fl.n;

    fl.n = aligned_read4(p + fl.c);
    if (fl.n == UNUSED) {
	throw Xapian::DatabaseCorruptError("Freelist next pointer invalid");
    }
    // Allow for mini-header at start of freelist block.
    fl.c = C_BASE;
    read_block(B, fl.n, p);

    // Either the freelist end is in this block, or this freelist block has
    // a next pointer.
    Assert(fl.n == fl_end.n || aligned_read4(p + FREELIST_END - 4) != UNUSED);

    if (blk_to_free) {
	Assert(*blk_to_free == BLK_UNUSED);
	*blk_to_free = old_fl_blk;
    } else {
	mark_block_unused(B, block_size, old_fl_blk);
    }

    return get_block(B, block_size);
}

uint4
HoneyFreeList::walk(const HoneyTable* B, uint4 block_size, bool inclusive)
{
    if (fl == fl_end) {
	// It's expected that the caller checks !empty() first.
	return UNUSED;
    }

    if (p == 0) {
	if (fl.n == UNUSED) {
	    throw Xapian::DatabaseCorruptError("Freelist pointer invalid");
	}
	p = new uint8_t[block_size];
	read_block(B, fl.n, p);
	if (inclusive) {
	    Assert(fl.n == fl_end.n ||
		   aligned_read4(p + FREELIST_END - 4) != UNUSED);
	    return fl.n;
	}
    }

    // Either the freelist end is in this block, or this freelist block has
    // a next pointer.
    Assert(fl.n == fl_end.n || aligned_read4(p + FREELIST_END - 4) != UNUSED);

    if (fl.c != FREELIST_END - 4) {
	uint4 blk = aligned_read4(p + fl.c);
	fl.c += 4;
	return blk;
    }

    fl.n = aligned_read4(p + fl.c);
    if (fl.n == UNUSED) {
	throw Xapian::DatabaseCorruptError("Freelist next pointer invalid");
    }
    // Allow for mini-header at start of freelist block.
    fl.c = C_BASE;
    read_block(B, fl.n, p);

    // Either the freelist end is in this block, or this freelist block has
    // a next pointer.
    Assert(fl.n == fl_end.n || aligned_read4(p + FREELIST_END - 4) != UNUSED);

    if (inclusive)
	return fl.n;
    return walk(B, block_size, inclusive);
}

void
HoneyFreeList::mark_block_unused(const HoneyTable* B,
				 uint4 block_size,
				 uint4 blk)
{
    // If the current flw block is full, we need to call get_block(), and if
    // the returned block is the last entry in its freelist block, that block
    // needs to be marked as unused.  The recursion this would create is
    // problematic, so we instead note down that block and mark it as unused
    // once we've processed the original request.
    uint4 blk_to_free = BLK_UNUSED;

    if (!pw) {
	pw = new uint8_t[block_size];
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
	aligned_write4(pw + FREELIST_END - 4, UNUSED);
    } else if (flw.c == FREELIST_END - 4) {
	// blk is free *after* the current revision gets released, so we can't
	// just use blk as the next block in the freelist chain.
	uint4 n = get_block(B, block_size, &blk_to_free);
	aligned_write4(pw + flw.c, n);
#ifdef HONEY_FREELIST_SIZE
	if (block_size != FREELIST_END) {
	    memset(pw + FREELIST_END, 0, block_size - FREELIST_END);
	}
#endif
	write_block(B, flw.n, pw, revision + 1);
	if (p && flw.n == fl.n) {
	    // FIXME: share and refcount?
	    memcpy(p, pw, block_size);
	    Assert(fl.n == fl_end.n ||
		   aligned_read4(p + FREELIST_END - 4) != UNUSED);
	}
	flw.n = n;
	flw.c = C_BASE;
	flw_appending = (n == first_unused_block - 1);
	aligned_write4(pw + FREELIST_END - 4, UNUSED);
    }

    aligned_write4(pw + flw.c, blk);
    flw.c += 4;

    if (blk_to_free != BLK_UNUSED)
	mark_block_unused(B, block_size, blk_to_free);
}

void
HoneyFreeList::commit(const HoneyTable* B, uint4 block_size)
{
    if (pw && flw.c != 0) {
	memset(pw + flw.c, 255, FREELIST_END - flw.c - 4);
#ifdef HONEY_FREELIST_SIZE
	if (block_size != FREELIST_END) {
	    memset(pw + FREELIST_END, 0xaa, block_size - FREELIST_END);
	}
#endif
	write_block(B, flw.n, pw, revision);
	if (p && flw.n == fl.n) {
	    // FIXME: share and refcount?
	    memcpy(p, pw, block_size);
	    Assert(fl.n == fl_end.n ||
		   aligned_read4(p + FREELIST_END - 4) != UNUSED);
	}
	flw_appending = true;
	fl_end = flw;
    }
}

HoneyFreeListChecker::HoneyFreeListChecker(const HoneyFreeList& fl)
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
HoneyFreeListChecker::count_set_bits(uint4 * p_first_bad_blk) const
{
    const unsigned BITS_PER_ELT = sizeof(elt_type) * 8;
    uint4 c = 0;
    for (uint4 i = 0; i < bitmap_size; ++i) {
	elt_type elt = bitmap[i];
	if (usual(elt == 0))
	    continue;
	if (c == 0 && p_first_bad_blk) {
	    uint4 first_bad_blk = i * BITS_PER_ELT;
	    if (false) {
#if HAVE_DECL___BUILTIN_CTZ
	    } else if (sizeof(elt_type) == sizeof(unsigned)) {
		first_bad_blk += __builtin_ctz(elt);
#endif
#if HAVE_DECL___BUILTIN_CTZL
	    } else if (sizeof(elt_type) == sizeof(unsigned long)) {
		first_bad_blk += __builtin_ctzl(elt);
#endif
#if HAVE_DECL___BUILTIN_CTZLL
	    } else if (sizeof(elt_type) == sizeof(unsigned long long)) {
		first_bad_blk += __builtin_ctzll(elt);
#endif
	    } else {
		for (elt_type mask = 1; (elt & mask) == 0; mask <<= 1) {
		    ++first_bad_blk;
		}
	    }
	    *p_first_bad_blk = first_bad_blk;
	}

	// Count set bits in elt.
	add_popcount(c, elt);
    }
    return c;
}
