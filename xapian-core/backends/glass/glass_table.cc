/** @file
 * @brief Btree implementation
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2011,2012,2013,2014,2015,2016 Olly Betts
 * Copyright 2008 Lemur Consulting Ltd
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

#include "glass_table.h"

#include <xapian/error.h>

#include "omassert.h"
#include "posixy_wrapper.h"
#include "str.h"
#include "stringutils.h" // For STRINGIZE().

#include <sys/types.h>

#include <cerrno>
#include <cstring>   /* for memmove */
#include <climits>   /* for CHAR_BIT */

#include "glass_freelist.h"
#include "glass_changes.h"
#include "glass_cursor.h"
#include "glass_defs.h"
#include "glass_version.h"

#include "debuglog.h"
#include "filetests.h"
#include "io_utils.h"
#include "pack.h"
#include "wordaccess.h"

#include <algorithm>  // for std::min()
#include <string>

#include "xapian/constants.h"

using namespace Glass;
using namespace std;

//#define BTREE_DEBUG_FULL 1
#undef BTREE_DEBUG_FULL

#ifdef BTREE_DEBUG_FULL
/*------debugging aids from here--------*/

static void print_key(const uint8_t * p, int c, int j);
static void print_tag(const uint8_t * p, int c, int j);

/*
static void report_cursor(int N, Btree * B, Glass::Cursor * C)
{
    int i;
    printf("%d)\n", N);
    for (i = 0; i <= B->level; i++)
	printf("p=%d, c=%d, n=[%d], rewrite=%d\n",
		C[i].p, C[i].c, C[i].n, C[i].rewrite);
}
*/

/*------to here--------*/
#endif /* BTREE_DEBUG_FULL */

static inline uint8_t *zeroed_new(size_t size)
{
    uint8_t *temp = new uint8_t[size];
    memset(temp, 0, size);
    return temp;
}

/* A B-tree consists of a file with extension GLASS_TABLE_EXTENSION divided
   into a sequence of equal sized blocks, numbered 0, 1, 2 ... some of which
   are free, some in use. Those in use are arranged in a tree.  Lists of
   currently unused blocks are stored in a freelist which is itself stored in
   unused blocks.

   Some "root info" is needed to locate a particular revision of the B-tree
   - this is stored in the "version file" (we store this data for all tables
   at a particular revision together).

   Each block, b, has a structure like this:

     R L M T D o1 o2 o3 ... oN <gap> [item] .. [item] .. [item] ...
     <---------- D ----------> <-M->

   And then,

   R = REVISION(b)  is the revision number the B-tree had when the block was
		    written into the DB file.
   L = GET_LEVEL(b) is the level of the block, which is the number of levels
		    towards the root of the B-tree structure. So leaf blocks
		    have level 0 and the one root block has the highest level
		    equal to the number of levels in the B-tree.  For blocks
		    storing the freelist, the level is set to 254.
   M = MAX_FREE(b)  is the size of the gap between the end of the directory and
		    the first item of data. (It is not necessarily the maximum
		    size among the bits of space that are free, but I can't
		    think of a better name.)
   T = TOTAL_FREE(b)is the total amount of free space left in b.
   D = DIR_END(b)   gives the offset to the end of the directory.

   o1, o2 ... oN are a directory of offsets to the N items held in the block.
   The items are key-tag pairs, and as they occur in the directory are ordered
   by the keys.

   An item has this form:

	   I K key X tag
	       ←K→
	   <------I---->

   A long tag presented through the API is split up into C pieces small enough
   to be accommodated in the blocks of the B-tree. The key is extended to
   include a counter, x, which runs from 1 to C. The key is preceded by a
   length, K, and the whole item with a length, I, as depicted above.  The
   upper bits of I encode a flag indicating if this item is compressed, and a
   flag saying if this is the last piece of a tag (i.e. if x == C).

   Here are the corresponding definitions:

*/

/** Flip to sequential addition block-splitting after this number of observed
 *  sequential additions (in negated form). */
#define SEQ_START_POINT (-10)

/* Note use of the limits.h values:
   UCHAR_MAX = 255, an unsigned with all bits set, and
   CHAR_BIT = 8, the number of bits per byte

   BYTE_PAIR_RANGE below is the smallest +ve number that can't be held in two
   bytes -- 64K effectively.
*/

#define BYTE_PAIR_RANGE (1 << 2 * CHAR_BIT)

/// read_block(n, p) reads block n of the DB file to address p.
void
GlassTable::read_block(uint4 n, uint8_t * p) const
{
    // Log the value of p, not the contents of the block it points to...
    LOGCALL_VOID(DB, "GlassTable::read_block", n | (void*)p);
    if (rare(handle == -2))
	GlassTable::throw_database_closed();
    AssertRel(n,<,free_list.get_first_unused_block());

    io_read_block(handle, reinterpret_cast<char *>(p), block_size, n, offset);

    if (GET_LEVEL(p) != LEVEL_FREELIST) {
	int dir_end = DIR_END(p);
	if (rare(dir_end < DIR_START || unsigned(dir_end) > block_size)) {
	    string msg("dir_end invalid in block ");
	    msg += str(n);
	    throw Xapian::DatabaseCorruptError(msg);
	}
    }
}

/** write_block(n, p, appending) writes block n in the DB file from address p.
 *
 *  If appending is true (not specified it defaults to false), then this
 *  indicates that we've added data to a block in space which was previously
 *  unused, and are writing the block back in place - we use this to add
 *  free list entries (the information about where the freelist data for a
 *  revision begins and ends is stored in the "iamglass" file).  We don't
 *  currently use this flag for much, but it signifies that we don't risk
 *  invalidating any existing revisions, which may be useful information.
 */
void
GlassTable::write_block(uint4 n, const uint8_t * p, bool appending) const
{
    LOGCALL_VOID(DB, "GlassTable::write_block", n | p | appending);
    Assert(writable);
    /* Check that n is in range. */
    AssertRel(n,<,free_list.get_first_unused_block());

    /* don't write to non-free */
    // FIXME: We can no longer check this easily.
    // AssertParanoid(free_list.block_free_at_start(block_size, n));

    /* write revision is okay */
    AssertEqParanoid(REVISION(p), revision_number + 1);

    if (appending) {
	// In the case of the freelist, new entries can safely be written into
	// the space at the end of the latest freelist block, as that's unused
	// by previous revisions.  It is also safe to write into a block which
	// wasn't used in the previous revision.
	//
	// It's only when we start a new freelist block that we need to worry
	// about invalidating old revisions.
    } else if (flags & Xapian::DB_DANGEROUS) {
	// FIXME: We should somehow flag this to prevent readers opening the
	// database.  Removing "iamglass" or setting a flag in it doesn't deal
	// with any existing readers though.  Perhaps we want to have readers
	// read lock and try to take an exclusive lock here?
    }

    const char * p_char = reinterpret_cast<const char *>(p);
    io_write_block(handle, p_char, block_size, n, offset);

    if (!changes_obj) return;

    unsigned char v;
    // FIXME: track table_type in this class?
    if (strcmp(tablename, "position") == 0) {
	v = int(Glass::POSITION);
    } else if (strcmp(tablename, "postlist") == 0) {
	v = int(Glass::POSTLIST);
    } else if (strcmp(tablename, "docdata") == 0) {
	v = int(Glass::DOCDATA);
    } else if (strcmp(tablename, "spelling") == 0) {
	v = int(Glass::SPELLING);
    } else if (strcmp(tablename, "synonym") == 0) {
	v = int(Glass::SYNONYM);
    } else if (strcmp(tablename, "termlist") == 0) {
	v = int(Glass::TERMLIST);
    } else {
	return; // FIXME
    }

    if (block_size == 2048) {
	v |= 0 << 3;
    } else if (block_size == 4096) {
	v |= 1 << 3;
    } else if (block_size == 8192) {
	v |= 2 << 3;
    } else if (block_size == 16384) {
	v |= 3 << 3;
    } else if (block_size == 32768) {
	v |= 4 << 3;
    } else if (block_size == 65536) {
	v |= 5 << 3;
    } else {
	return; // FIXME
    }

    string buf;
    buf += char(v);
    // Write the block number to the file
    pack_uint(buf, n);

    changes_obj->write_block(buf);
    changes_obj->write_block(reinterpret_cast<const char *>(p), block_size);
}

/* A note on cursors:

   Each B-tree level has a corresponding array element C[j] in a
   cursor, C. C[0] is the leaf (or data) level, and C[B->level] is the
   root block level. Within a level j,

       C[j].p  addresses the block
       C[j].c  is the offset into the directory entry in the block
       C[j].n  is the number of the block at C[j].p

   A look up in the B-tree causes navigation of the blocks starting
   from the root. In each block, p, we find an offset, c, to an item
   which gives the number, n, of the block for the next level. This
   leads to an array of values p,c,n which are held inside the cursor.

   Any Btree object B has a built-in cursor, at B->C. But other cursors may
   be created.  If BC is a created cursor, BC->C is the cursor in the
   sense given above, and BC->B is the handle for the B-tree again.
*/

void
GlassTable::set_overwritten() const
{
    LOGCALL_VOID(DB, "GlassTable::set_overwritten", NO_ARGS);
    // If we're writable, there shouldn't be another writer who could cause
    // overwritten to be flagged, so that's a DatabaseCorruptError.
    if (writable)
	throw Xapian::DatabaseCorruptError("Block overwritten - run xapian-check on this database");
    throw Xapian::DatabaseModifiedError("The revision being read has been discarded - you should call Xapian::Database::reopen() and retry the operation");
}

/* block_to_cursor(C, j, n) puts block n into position C[j] of cursor
   C, writing the block currently at C[j] back to disk if necessary.
   Note that

       C[j].rewrite

   is true iff C[j].n is different from block n in file DB. If it is
   false no rewriting is necessary.
*/

void
GlassTable::block_to_cursor(Glass::Cursor * C_, int j, uint4 n) const
{
    LOGCALL_VOID(DB, "GlassTable::block_to_cursor", (void*)C_ | j | n);
    if (n == C_[j].get_n()) return;

    if (writable && C_[j].rewrite) {
	Assert(C == C_);
	write_block(C_[j].get_n(), C_[j].get_p());
	C_[j].rewrite = false;
    }

    // Check if the block is in the built-in cursor (potentially in
    // modified form).
    const uint8_t * p;
    if (n == C[j].get_n()) {
	p = C_[j].clone(C[j]);
    } else {
	uint8_t * q = C_[j].init(block_size);
	read_block(n, q);
	p = q;
	C_[j].set_n(n);
    }

    if (j < level) {
	/* unsigned comparison */
	if (rare(REVISION(p) > REVISION(C_[j + 1].get_p()))) {
	    set_overwritten();
	    return;
	}
    }

    if (rare(j != GET_LEVEL(p))) {
	string msg = "Expected block ";
	msg += str(n);
	msg += " to be level ";
	msg += str(j);
	msg += ", not ";
	msg += str(GET_LEVEL(p));
	throw Xapian::DatabaseCorruptError(msg);
    }
}

/** Btree::alter(); is called when the B-tree is to be altered.

   It causes new blocks to be forced for the current set of blocks in
   the cursor.

   The point is that if a block at level 0 is to be altered it may get
   a new number. Then the pointer to this block from level 1 will need
   changing. So the block at level 1 needs altering and may get a new
   block number. Then the pointer to this block from level 2 will need
   changing ... and so on back to the root.

   The clever bit here is spotting the cases when we can make an early
   exit from this process. If C[j].rewrite is true, C[j+k].rewrite
   will be true for k = 1,2 ... We have been through all this before,
   and there is no need to do it again. If C[j].n was free at the
   start of the transaction, we can copy it back to the same place
   without violating the integrity of the B-tree. We don't then need a
   new n and can return. The corresponding C[j].rewrite may be true or
   false in that case.
*/

void
GlassTable::alter()
{
    LOGCALL_VOID(DB, "GlassTable::alter", NO_ARGS);
    Assert(writable);
    if (flags & Xapian::DB_DANGEROUS) {
	C[0].rewrite = true;
	return;
    }
    int j = 0;
    while (true) {
	if (C[j].rewrite) return; /* all new, so return */
	C[j].rewrite = true;

	glass_revision_number_t rev = REVISION(C[j].get_p());
	if (rev == revision_number + 1) {
	    return;
	}
	Assert(rev < revision_number + 1);
	uint4 n = C[j].get_n();
	free_list.mark_block_unused(this, block_size, n);
	SET_REVISION(C[j].get_modifiable_p(block_size), revision_number + 1);
	n = free_list.get_block(this, block_size);
	C[j].set_n(n);

	if (j == level) return;
	j++;
	BItem_wr(C[j].get_modifiable_p(block_size), C[j].c).set_block_given_by(n);
    }
}

/** find_in_leaf(p, key, c, exact) searches for the key in the leaf block at p.

   What we get is the directory entry to the last key <= the key being searched
   for.

   The lookup is by binary chop, with i and j set to the left and
   right ends of the search area. In sequential addition, c will often
   be the answer, so we test the keys round c and move i and j towards
   c if possible.

   exact is set to true if the match was exact (otherwise exact is unchanged).
*/

int
GlassTable::find_in_leaf(const uint8_t * p, LeafItem item, int c, bool& exact)
{
    LOGCALL_STATIC(DB, int, "GlassTable::find_in_leaf", (const void*)p | (const void *)item.get_address() | c | Literal("bool&"));
    // c should be odd (either -1, or an even offset from DIR_START).
    Assert((c & 1) == 1);
    int i = DIR_START;
    i -= D2;
    if (c != -1) {
	AssertRel(i,<=,c);
    }
    int j = DIR_END(p);

    if (c != -1) {
	if (c < j && i < c) {
	    int r = compare(LeafItem(p, c), item);
	    if (r == 0) {
		exact = true;
		return c;
	    }
	    if (r < 0) i = c;
	}
	c += D2;
	if (c < j && i < c) {
	    int r = compare(item, LeafItem(p, c));
	    if (r == 0) {
		exact = true;
		return c;
	    }
	    if (r < 0) j = c;
	}
    }

    while (j - i > D2) {
	int k = i + ((j - i) / (D2 * 2)) * D2; /* mid way */
	int r = compare(item, LeafItem(p, k));
	if (r < 0) {
	    j = k;
	} else {
	    i = k;
	    if (r == 0) {
		exact = true;
		break;
	    }
	}
    }
    AssertRel(DIR_START - D2,<=,i);
    AssertRel(i,<,DIR_END(p));
    RETURN(i);
}

template<typename ITEM> int
find_in_branch_(const uint8_t * p, ITEM item, int c)
{
    // c should be odd (either -1, or an even offset from DIR_START).
    Assert((c & 1) == 1);
    int i = DIR_START;
    if (c != -1) {
	AssertRel(i,<=,c);
    }
    int j = DIR_END(p);

    if (c != -1) {
	if (c < j && i < c) {
	    int r = compare(BItem(p, c), item);
	    if (r == 0) return c;
	    if (r < 0) i = c;
	}
	c += D2;
	if (c < j && i < c) {
	    int r = compare(item, BItem(p, c));
	    if (r == 0) return c;
	    if (r < 0) j = c;
	}
    }

    while (j - i > D2) {
	int k = i + ((j - i) / (D2 * 2)) * D2; /* mid way */
	int r = compare(item, BItem(p, k));
	if (r < 0) {
	    j = k;
	} else {
	    i = k;
	    if (r == 0) break;
	}
    }
    AssertRel(DIR_START,<=,i);
    AssertRel(i,<,DIR_END(p));
    return i;
}

int
GlassTable::find_in_branch(const uint8_t * p, LeafItem item, int c)
{
    LOGCALL_STATIC(DB, int, "GlassTable::find_in_branch", (const void*)p | (const void *)item.get_address() | c);
    RETURN(find_in_branch_(p, item, c));
}

int
GlassTable::find_in_branch(const uint8_t * p, BItem item, int c)
{
    LOGCALL_STATIC(DB, int, "GlassTable::find_in_branch", (const void*)p | (const void *)item.get_address() | c);
    RETURN(find_in_branch_(p, item, c));
}

/** find(C_) searches for the key of B->kt in the B-tree.

   Result is true if found, false otherwise.  When false, the B_tree
   cursor is positioned at the last key in the B-tree <= the search
   key.  Goes to first (null) item in B-tree when key length == 0.
*/

bool
GlassTable::find(Glass::Cursor * C_) const
{
    LOGCALL(DB, bool, "GlassTable::find", (void*)C_);
    // Note: the parameter is needed when we're called by GlassCursor
    const uint8_t * p;
    int c;
    for (int j = level; j > 0; --j) {
	p = C_[j].get_p();
	c = find_in_branch(p, kt, C_[j].c);
#ifdef BTREE_DEBUG_FULL
	printf("Block in GlassTable:find - code position 1");
	report_block_full(j, C_[j].get_n(), p);
#endif /* BTREE_DEBUG_FULL */
	C_[j].c = c;
	block_to_cursor(C_, j - 1, BItem(p, c).block_given_by());
    }
    p = C_[0].get_p();
    bool exact = false;
    c = find_in_leaf(p, kt, C_[0].c, exact);
#ifdef BTREE_DEBUG_FULL
    printf("Block in GlassTable:find - code position 2");
    report_block_full(0, C_[0].get_n(), p);
#endif /* BTREE_DEBUG_FULL */
    C_[0].c = c;
    RETURN(exact);
}

/** compact(p) compact the block at p by shuffling all the items up to the end.

   MAX_FREE(p) is then maximized, and is equal to TOTAL_FREE(p).
*/

void
GlassTable::compact(uint8_t * p)
{
    LOGCALL_VOID(DB, "GlassTable::compact", (void*)p);
    Assert(p != buffer);
    Assert(writable);
    int e = block_size;
    uint8_t * b = buffer;
    int dir_end = DIR_END(p);
    if (GET_LEVEL(p) == 0) {
	// Leaf.
	for (int c = DIR_START; c < dir_end; c += D2) {
	    LeafItem item(p, c);
	    int l = item.size();
	    e -= l;
	    memcpy(b + e, item.get_address(), l);
	    LeafItem_wr::setD(p, c, e);  /* reform in b */
	}
    } else {
	// Branch.
	for (int c = DIR_START; c < dir_end; c += D2) {
	    BItem item(p, c);
	    int l = item.size();
	    e -= l;
	    memcpy(b + e, item.get_address(), l);
	    BItem_wr::setD(p, c, e);  /* reform in b */
	}
    }
    memcpy(p + e, b + e, block_size - e);  /* copy back */
    e -= dir_end;
    SET_TOTAL_FREE(p, e);
    SET_MAX_FREE(p, e);
}

/** Btree needs to gain a new level to insert more items: so split root block
 *  and construct a new one.
 */
void
GlassTable::split_root(uint4 split_n)
{
    LOGCALL_VOID(DB, "GlassTable::split_root", split_n);
    /* gain a level */
    ++level;

    /* check level overflow - this isn't something that should ever happen
     * but deserves more than an Assert()... */
    if (level == GLASS_BTREE_CURSOR_LEVELS) {
	throw Xapian::DatabaseCorruptError("Btree has grown impossibly large ("
					   STRINGIZE(GLASS_BTREE_CURSOR_LEVELS)
					   " levels)");
    }

    uint8_t * q = C[level].init(block_size);
    memset(q, 0, block_size);
    C[level].c = DIR_START;
    C[level].set_n(free_list.get_block(this, block_size));
    C[level].rewrite = true;
    SET_REVISION(q, revision_number + 1);
    SET_LEVEL(q, level);
    SET_DIR_END(q, DIR_START);
    compact(q);   /* to reset TOTAL_FREE, MAX_FREE */

    /* form a null key in b with a pointer to the old root */
    uint8_t b[10]; /* 7 is exact */
    BItem_wr item(b);
    item.form_null_key(split_n);
    add_branch_item(item, level);
}

/** enter_key_above_leaf(previtem, newitem) is called after a leaf block split.

   It enters in the block at level C[1] a separating key for the block
   at level C[0]. The key itself is newitem.key(). previtem is the
   preceding item, and at level 1 newitem.key() can be trimmed down to the
   first point of difference to previtem.key() for entry in C[j].

   This code looks longer than it really is. If j exceeds the number
   of B-tree levels the root block has split and we have to construct
   a new one, but this is a rare event.

   The key is constructed in b, with block number C[0].n as tag,
   and this is added in with add_item. add_item may itself cause a
   block split, with a further call to enter_key. Hence the recursion.
*/
void
GlassTable::enter_key_above_leaf(LeafItem previtem, LeafItem newitem)
{
    LOGCALL_VOID(DB, "GlassTable::enter_key_above_leaf", Literal("previtem") | Literal("newitem"));
    Assert(writable);
    Assert(compare(previtem, newitem) < 0);

    Key prevkey = previtem.key();
    Key newkey = newitem.key();
    int new_comp = newitem.component_of();

    uint4 blocknumber = C[0].get_n();

    // FIXME update to use Key
    // Keys are truncated here: but don't truncate the count at the end away.
    const int newkey_len = newkey.length();
    AssertRel(newkey_len,>,0);

    // Truncate the key to the minimal key which differs from prevkey,
    // the preceding key in the block.
    int i = 0;
    const int min_len = min(newkey_len, prevkey.length());
    while (i < min_len && prevkey[i] == newkey[i]) {
	i++;
    }

    // Want one byte of difference.
    if (i < newkey_len) i++;

    // Enough space for a branch item with maximum length key.
    uint8_t b[BYTES_PER_BLOCK_NUMBER + K1 + 255 + X2];
    BItem_wr item(b);
    AssertRel(i, <=, 255);
    item.set_truncated_key_and_block(newkey, new_comp, i, blocknumber);

    // The split block gets inserted into the parent after the pointer to the
    // current child.
    AssertEq(C[1].c, find_in_branch(C[1].get_p(), item, C[1].c));
    C[1].c += D2;
    C[1].rewrite = true; /* a subtle point: this *is* required. */
    add_branch_item(item, 1);
}

/** enter_key_above_branch(j, newkey) is called after a branch block split.

   It enters in the block at level C[j] a separating key for the block
   at level C[j - 1]. The key itself is newkey.

   This code looks longer than it really is. If j exceeds the number
   of B-tree levels the root block has split and we have to construct
   a new one, but this is a rare event.

   The key is constructed in b, with block number C[j - 1].n as tag,
   and this is added in with add_item. add_item may itself cause a
   block split, with a further call to enter_key. Hence the recursion.
*/
void
GlassTable::enter_key_above_branch(int j, BItem newitem)
{
    LOGCALL_VOID(DB, "GlassTable::enter_key_above_branch", j | Literal("newitem"));
    Assert(writable);
    AssertRel(j,>,1);

    /* Can't truncate between branch levels, since the separated keys
     * are in at the leaf level, and truncating again will change the
     * branch point.
     */

    uint4 blocknumber = C[j - 1].get_n();

    // Enough space for a branch item with maximum length key.
    uint8_t b[BYTES_PER_BLOCK_NUMBER + K1 + 255 + X2];
    BItem_wr item(b);
    item.set_key_and_block(newitem.key(), blocknumber);

    // The split block gets inserted into the parent after the pointer to the
    // current child.
    AssertEq(C[j].c, find_in_branch(C[j].get_p(), item, C[j].c));
    C[j].c += D2;
    C[j].rewrite = true; /* a subtle point: this *is* required. */
    add_branch_item(item, j);
}

/** mid_point(p) finds the directory entry in c that determines the
   approximate mid point of the data in the block at p.
 */

int
GlassTable::mid_point(uint8_t * p) const
{
    LOGCALL(DB, int, "GlassTable::mid_point", (void*)p);
    int n = 0;
    int dir_end = DIR_END(p);
    int size = block_size - TOTAL_FREE(p) - dir_end;
    for (int c = DIR_START; c < dir_end; c += D2) {
	int l;
	if (GET_LEVEL(p) == 0) {
	    l = LeafItem(p, c).size();
	} else {
	    l = BItem(p, c).size();
	}
	n += 2 * l;
	if (n >= size) {
	    if (l < n - size) RETURN(c);
	    RETURN(c + D2);
	}
    }

    /* This shouldn't happen, as the sum of the item sizes should be the same
     * as the value calculated in size, so assert but return a sane value just
     * in case. */
    Assert(false);
    RETURN(dir_end);
}

/** add_item_to_leaf(p, kt_, c) adds item kt_ to the leaf block at p.

   c is the offset in the directory that needs to be expanded to accommodate
   the new entry for the item.  We know before this is called that there is
   enough contiguous room for the item in the block, so it's just a matter of
   shuffling up any directory entries after where we're inserting and copying
   in the item.
*/

void
GlassTable::add_item_to_leaf(uint8_t * p, LeafItem kt_, int c)
{
    LOGCALL_VOID(DB, "GlassTable::add_item_to_leaf", (void*)p | Literal("kt_") | c);
    Assert(writable);
    int dir_end = DIR_END(p);
    int kt_len = kt_.size();
    int needed = kt_len + D2;
    int new_total = TOTAL_FREE(p) - needed;
    int new_max = MAX_FREE(p) - needed;

    Assert(new_total >= 0);

    AssertRel(MAX_FREE(p),>=,needed);

    AssertRel(DIR_START,<=,c);
    AssertRel(c,<=,dir_end);

    memmove(p + c + D2, p + c, dir_end - c);
    dir_end += D2;
    SET_DIR_END(p, dir_end);

    int o = dir_end + new_max;
    LeafItem_wr::setD(p, c, o);
    memmove(p + o, kt_.get_address(), kt_len);

    SET_MAX_FREE(p, new_max);

    SET_TOTAL_FREE(p, new_total);
}

/** add_item_to_branch(p, kt_, c) adds item kt_ to the branch block at p.

   c is the offset in the directory that needs to be expanded to accommodate
   the new entry for the item.  We know before this is called that there is
   enough contiguous room for the item in the block, so it's just a matter of
   shuffling up any directory entries after where we're inserting and copying
   in the item.
*/

void
GlassTable::add_item_to_branch(uint8_t * p, BItem kt_, int c)
{
    LOGCALL_VOID(DB, "GlassTable::add_item_to_branch", (void*)p | Literal("kt_") | c);
    Assert(writable);
    int dir_end = DIR_END(p);
    int kt_len = kt_.size();
    int needed = kt_len + D2;
    int new_total = TOTAL_FREE(p) - needed;
    int new_max = MAX_FREE(p) - needed;

    Assert(new_total >= 0);

    AssertRel(MAX_FREE(p),>=,needed);

    AssertRel(DIR_START,<=,c);
    AssertRel(c,<=,dir_end);

    memmove(p + c + D2, p + c, dir_end - c);
    dir_end += D2;
    SET_DIR_END(p, dir_end);

    int o = dir_end + new_max;
    BItem_wr::setD(p, c, o);
    memmove(p + o, kt_.get_address(), kt_len);

    SET_MAX_FREE(p, new_max);

    SET_TOTAL_FREE(p, new_total);
}

/** GlassTable::add_leaf_item(kt_) adds item kt_ to the leaf block.
 *
 *  If there is not enough room the block splits and the item is then
 *  added to the appropriate half.
 */
void
GlassTable::add_leaf_item(LeafItem kt_)
{
    LOGCALL_VOID(DB, "GlassTable::add_leaf_item", Literal("kt_"));
    Assert(writable);
    uint8_t * p = C[0].get_modifiable_p(block_size);
    int c = C[0].c;
    uint4 n;

    int needed = kt_.size() + D2;
    if (TOTAL_FREE(p) < needed) {
	int m;
	// Prepare to split p. After splitting, the block is in two halves, the
	// lower half is split_p, the upper half p again. add_to_upper_half
	// becomes true when the item gets added to p, false when it gets added
	// to split_p.

	if (seq_count < 0) {
	    // If we're not in sequential mode, we split at the mid point
	    // of the node.
	    m = mid_point(p);
	} else {
	    // During sequential addition, split at the insert point
	    AssertRel(c,>=,DIR_START);
	    m = c;
	}

	uint4 split_n = C[0].get_n();
	C[0].set_n(free_list.get_block(this, block_size));

	memcpy(split_p, p, block_size);  // replicate the whole block in split_p
	SET_DIR_END(split_p, m);
	compact(split_p);      /* to reset TOTAL_FREE, MAX_FREE */

	{
	    int residue = DIR_END(p) - m;
	    int new_dir_end = DIR_START + residue;
	    memmove(p + DIR_START, p + m, residue);
	    SET_DIR_END(p, new_dir_end);
	}

	compact(p);      /* to reset TOTAL_FREE, MAX_FREE */

	bool add_to_upper_half;
	if (seq_count < 0) {
	    add_to_upper_half = (c >= m);
	} else {
	    // And add item to lower half if split_p has room, otherwise upper
	    // half
	    add_to_upper_half = (TOTAL_FREE(split_p) < needed);
	}

	if (add_to_upper_half) {
	    c -= (m - DIR_START);
	    Assert(seq_count < 0 || c <= DIR_START + D2);
	    Assert(c >= DIR_START);
	    Assert(c <= DIR_END(p));
	    add_item_to_leaf(p, kt_, c);
	    n = C[0].get_n();
	} else {
	    Assert(c >= DIR_START);
	    Assert(c <= DIR_END(split_p));
	    add_item_to_leaf(split_p, kt_, c);
	    n = split_n;
	}
	write_block(split_n, split_p);

	// Check if we're splitting the root block.
	if (0 == level) split_root(split_n);

	/* Enter a separating key at level 1 between */
	/* the last key of block split_p, and the first key of block p */
	enter_key_above_leaf(LeafItem(split_p, DIR_END(split_p) - D2),
			     LeafItem(p, DIR_START));
    } else {
	AssertRel(TOTAL_FREE(p),>=,needed);

	if (MAX_FREE(p) < needed) {
	    compact(p);
	    AssertRel(MAX_FREE(p),>=,needed);
	}

	add_item_to_leaf(p, kt_, c);
	n = C[0].get_n();
    }

    changed_n = n;
    changed_c = c;
}

/** GlassTable::add_item(kt_, j) adds item kt_ to the block at cursor level C[j].
 *
 *  If there is not enough room the block splits and the item is then
 *  added to the appropriate half.
 */
void
GlassTable::add_branch_item(BItem kt_, int j)
{
    LOGCALL_VOID(DB, "GlassTable::add_branch_item", Literal("kt_") | j);
    Assert(writable);
    uint8_t * p = C[j].get_modifiable_p(block_size);
    int c = C[j].c;

    int needed = kt_.size() + D2;
    if (TOTAL_FREE(p) < needed) {
	int m;
	// Prepare to split p. After splitting, the block is in two halves, the
	// lower half is split_p, the upper half p again. add_to_upper_half
	// becomes true when the item gets added to p, false when it gets added
	// to split_p.

	if (seq_count < 0) {
	    // If we're not in sequential mode, we split at the mid point
	    // of the node.
	    m = mid_point(p);
	} else {
	    // During sequential addition, split at the insert point
	    AssertRel(c,>=,DIR_START);
	    m = c;
	}

	uint4 split_n = C[j].get_n();
	C[j].set_n(free_list.get_block(this, block_size));

	memcpy(split_p, p, block_size);  // replicate the whole block in split_p
	SET_DIR_END(split_p, m);
	compact(split_p);      /* to reset TOTAL_FREE, MAX_FREE */

	{
	    int residue = DIR_END(p) - m;
	    int new_dir_end = DIR_START + residue;
	    memmove(p + DIR_START, p + m, residue);
	    SET_DIR_END(p, new_dir_end);
	}

	compact(p);      /* to reset TOTAL_FREE, MAX_FREE */

	bool add_to_upper_half;
	if (seq_count < 0) {
	    add_to_upper_half = (c >= m);
	} else {
	    // And add item to lower half if split_p has room, otherwise upper
	    // half
	    add_to_upper_half = (TOTAL_FREE(split_p) < needed);
	}

	if (add_to_upper_half) {
	    c -= (m - DIR_START);
	    Assert(seq_count < 0 || c <= DIR_START + D2);
	    Assert(c >= DIR_START);
	    Assert(c <= DIR_END(p));
	    add_item_to_branch(p, kt_, c);
	} else {
	    Assert(c >= DIR_START);
	    Assert(c <= DIR_END(split_p));
	    add_item_to_branch(split_p, kt_, c);
	}
	write_block(split_n, split_p);

	// Check if we're splitting the root block.
	if (j == level) split_root(split_n);

	/* Enter a separating key at level j + 1 between */
	/* the last key of block split_p, and the first key of block p */
	enter_key_above_branch(j + 1, BItem(p, DIR_START));

	// In branch levels, we can make the first key of block p null and
	// save a bit of disk space.  Other redundant keys will still creep
	// in though.
	BItem_wr item(p, DIR_START);
	int new_total_free = TOTAL_FREE(p) + item.key().length();
	item.form_null_key(item.block_given_by());
	SET_TOTAL_FREE(p, new_total_free);
    } else {
	AssertRel(TOTAL_FREE(p),>=,needed);

	if (MAX_FREE(p) < needed) {
	    compact(p);
	    AssertRel(MAX_FREE(p),>=,needed);
	}

	add_item_to_branch(p, kt_, c);
    }
}

/** GlassTable::delete_leaf_item(repeatedly) is (almost) the converse of add_leaf_item.
 *
 * If repeatedly is true, the process repeats at the next level when a
 * block has been completely emptied, freeing the block and taking out
 * the pointer to it.  Emptied root blocks are also removed, which
 * reduces the number of levels in the B-tree.
 */
void
GlassTable::delete_leaf_item(bool repeatedly)
{
    LOGCALL_VOID(DB, "GlassTable::delete_leaf_item", repeatedly);
    Assert(writable);
    uint8_t * p = C[0].get_modifiable_p(block_size);
    int c = C[0].c;
    AssertRel(DIR_START,<=,c);
    AssertRel(c,<,DIR_END(p));
    int kt_len = LeafItem(p, c).size(); /* size of the item to be deleted */
    int dir_end = DIR_END(p) - D2;   /* directory length will go down by 2 bytes */

    memmove(p + c, p + c + D2, dir_end - c);
    SET_DIR_END(p, dir_end);
    SET_MAX_FREE(p, MAX_FREE(p) + D2);
    SET_TOTAL_FREE(p, TOTAL_FREE(p) + kt_len + D2);

    if (!repeatedly) return;
    if (0 < level) {
	if (dir_end == DIR_START) {
	    free_list.mark_block_unused(this, block_size, C[0].get_n());
	    C[0].rewrite = false;
	    C[0].set_n(BLK_UNUSED);
	    C[1].rewrite = true;  /* *is* necessary */
	    delete_branch_item(1);
	}
    }
}

/** GlassTable::delete_branch_item(j, repeatedly) is (almost) the converse of add_branch_item.
 *
 * The process repeats at the next level when a block has been completely
 * emptied, freeing the block and taking out the pointer to it.  Emptied root
 * blocks are also removed, which reduces the number of levels in the B-tree.
 */
void
GlassTable::delete_branch_item(int j)
{
    LOGCALL_VOID(DB, "GlassTable::delete_branch_item", j);
    Assert(writable);
    uint8_t * p = C[j].get_modifiable_p(block_size);
    int c = C[j].c;
    AssertRel(DIR_START,<=,c);
    AssertRel(c,<,DIR_END(p));
    int kt_len = BItem(p, c).size(); /* size of the item to be deleted */
    int dir_end = DIR_END(p) - D2;   /* directory length will go down by 2 bytes */

    memmove(p + c, p + c + D2, dir_end - c);
    SET_DIR_END(p, dir_end);
    SET_MAX_FREE(p, MAX_FREE(p) + D2);
    SET_TOTAL_FREE(p, TOTAL_FREE(p) + kt_len + D2);

    if (j < level) {
	if (dir_end == DIR_START) {
	    free_list.mark_block_unused(this, block_size, C[j].get_n());
	    C[j].rewrite = false;
	    C[j].set_n(BLK_UNUSED);
	    C[j + 1].rewrite = true;  /* *is* necessary */
	    delete_branch_item(j + 1);
	}
    } else {
	Assert(j == level);
	while (dir_end == DIR_START + D2 && level > 0) {
	    /* single item in the root block, so lose a level */
	    uint4 new_root = BItem(C[level].get_p(), DIR_START).block_given_by();
	    free_list.mark_block_unused(this, block_size, C[level].get_n());
	    C[level].destroy();
	    level--;

	    block_to_cursor(C, level, new_root);

	    dir_end = DIR_END(C[level].get_p()); /* prepare for the loop */
	}
    }
}

/* debugging aid:
static addcount = 0;
*/

/** add_kt(found) adds the item (key-tag pair) at B->kt into the
   B-tree, using cursor C.

   found == find() is handed over as a parameter from Btree::add.
   Btree::alter() prepares for the alteration to the B-tree. Then
   there are a number of cases to consider:

     If an item with the same key is in the B-tree (found is true),
     the new kt replaces it.

     If then kt is smaller, or the same size as, the item it replaces,
     kt is put in the same place as the item it replaces, and the
     TOTAL_FREE measure is reduced.

     If kt is larger than the item it replaces it is put in the
     MAX_FREE space if there is room, and the directory entry and
     space counts are adjusted accordingly.

     - But if there is not room we do it the long way: the old item is
     deleted with delete_leaf_item and kt is added in with add_item.

     If the key of kt is not in the B-tree (found is false), the new
     kt is added in with add_item.

     Returns:
	 0 : added kt
	 1 : replaced kt
	 2 : replaced kt and it was the final one
*/

int
GlassTable::add_kt(bool found)
{
    LOGCALL(DB, int, "GlassTable::add_kt", found);
    Assert(writable);

    /*
    {
	printf("%d) %s ", addcount++, (found ? "replacing" : "adding"));
	print_bytes(kt[I2], kt + I2 + K1); putchar('\n');
    }
    */
    alter();

    int result = 0;
    if (found) { /* replacement */
	seq_count = SEQ_START_POINT;
	sequential = false;

	uint8_t * p = C[0].get_modifiable_p(block_size);
	int c = C[0].c;
	AssertRel(DIR_START,<=,c);
	AssertRel(c,<,DIR_END(p));
	LeafItem item(p, c);
	int kt_size = kt.size();
	int needed = kt_size - item.size();

	result = item.last_component() ? 2 : 1;

	if (needed <= 0) {
	    /* simple replacement */
	    memmove(const_cast<uint8_t *>(item.get_address()),
		    kt.get_address(), kt_size);
	    SET_TOTAL_FREE(p, TOTAL_FREE(p) - needed);
	} else {
	    /* new item into the block's freespace */
	    int new_max = MAX_FREE(p) - kt_size;
	    if (new_max >= 0) {
		int o = DIR_END(p) + new_max;
		memmove(p + o, kt.get_address(), kt_size);
		LeafItem_wr::setD(p, c, o);
		SET_MAX_FREE(p, new_max);
		SET_TOTAL_FREE(p, TOTAL_FREE(p) - needed);
	    } else {
		/* do it the long way */
		delete_leaf_item(false);
		add_leaf_item(kt);
	    }
	}
    } else {
	/* addition */
	if (changed_n == C[0].get_n() && changed_c == C[0].c) {
	    if (seq_count < 0) seq_count++;
	} else {
	    seq_count = SEQ_START_POINT;
	    sequential = false;
	}
	C[0].c += D2;
	add_leaf_item(kt);
    }
    RETURN(result);
}

/* delete_kt() corresponds to add_kt(found), but there are only
   two cases: if the key is not found nothing is done, and if it is
   found the corresponding item is deleted with delete_leaf_item.

     Returns:
	 0 : nothing to delete
	 1 : deleted kt
	 2 : deleted kt and it was the final one
*/

int
GlassTable::delete_kt()
{
    LOGCALL(DB, int, "GlassTable::delete_kt", NO_ARGS);
    Assert(writable);

    seq_count = SEQ_START_POINT;
    sequential = false;

    if (!find(C))
	return 0;

    int result = LeafItem(C[0].get_p(), C[0].c).last_component() ? 2 : 1;
    alter();
    delete_leaf_item(true);

    RETURN(result);
}

/* GlassTable::form_key(key) treats address kt as an item holder and fills in
the key part:

	   (I) K key c (C tag)

The bracketed parts are left blank. The key is filled in with key_len bytes and
K set accordingly. c is set to 1.
*/

void GlassTable::form_key(const string & key) const
{
    LOGCALL_VOID(DB, "GlassTable::form_key", key);
    kt.form_key(key);
}

/* GlassTable::add(key, tag) adds the key/tag item to the
   B-tree, replacing any existing item with the same key.

   For a long tag, we end up having to add m components, of the form

       key 1 m tag1
       key 2 m tag2
       ...
       key m m tagm

   and tag1+tag2+...+tagm are equal to tag. These in their turn may be replacing
   n components of the form

       key 1 n TAG1
       key 2 n TAG2
       ...
       key n n TAGn

   and n may be greater than, equal to, or less than m. These cases are dealt
   with in the code below. If m < n for example, we end up with a series of
   deletions.
*/

void
GlassTable::add(const string& key, const string& tag, bool already_compressed)
{
    LOGCALL_VOID(DB, "GlassTable::add", key | tag | already_compressed);
    Assert(writable);

    if (handle < 0) {
	if (handle == -2) {
	    GlassTable::throw_database_closed();
	}
	RootInfo root_info;
	root_info.init(block_size, compress_min);
	do_open_to_write(&root_info);
    }

    form_key(key);

    const char* tag_data = tag.data();
    size_t tag_size = tag.size();

    bool compressed = false;
    if (already_compressed) {
	compressed = true;
    } else if (compress_min > 0 && tag_size > compress_min) {
	const char * res = comp_stream.compress(tag_data, &tag_size);
	if (res) {
	    compressed = true;
	    tag_data = res;
	}
    }

    // sort of matching kt.append_chunk(), but setting the chunk
    const size_t cd = kt.key().length() + K1 + I2 + X2;  // offset to the tag data
    const size_t L = max_item_size - cd; // largest amount of tag data for any chunk
    size_t first_L = L + X2;             // - amount for tag1 (we don't store X there)
    bool found = find(C);
    if (tag_size <= first_L) {
	// The whole tag clearly fits in one item, so no need to make this
	// complicated.
	first_L = tag_size;
    } else if (!found) {
	const uint8_t * p = C[0].get_p();
	size_t n = TOTAL_FREE(p) % (max_item_size + D2);
	if (n > D2 + cd) {
	    n -= (D2 + cd);
	    // if n >= last then fully filling this block won't produce
	    // an extra item, so we might as well do this even if
	    // full_compaction isn't active.
	    //
	    // In the full_compaction case, it turns out we shouldn't always
	    // try to fill every last byte.  Doing so can actually increase the
	    // total space required (I believe this effect is due to longer
	    // dividing keys being required in the index blocks).  Empirically,
	    // n >= key.size() + K appears a good criterion for K ~= 34.  This
	    // seems to save about 0.2% in total database size over always
	    // splitting the tag.  It'll also give be slightly faster retrieval
	    // as we can avoid reading an extra block occasionally.
	    size_t last = (tag_size - X2) % L;
	    if (n >= last || (full_compaction && n >= key.size() + 34)) {
		// first_L < max_item_size + D2 - D2 - cd
		// Total size of first item = cd + first_L < max_item_size
		first_L = n + X2;
	    }
	}
    }

    // There are m items to add.
    int m = (tag_size - first_L + L - 1) / L + 1;
    /* FIXME: sort out this error higher up and turn this into
     * an assert.
     */
    if (m >= BYTE_PAIR_RANGE)
	throw Xapian::UnimplementedError("Can't handle insanely large tags");

    size_t o = 0;                     // Offset into the tag
    size_t residue = tag_size;        // Bytes of the tag remaining to add in
    bool replacement = false;         // Has there been a replacement?
    bool components_to_del = false;   // Are there components to delete?
    int i;
    for (i = 1; i <= m; ++i) {
	size_t l = (i == m ? residue : (i == 1 ? first_L : L));
	size_t this_cd = (i == 1 ? cd - X2 : cd);
	Assert(this_cd + l <= block_size);
	Assert(o + l <= tag_size);
	kt.set_tag(this_cd, tag_data + o, l, compressed, i, m);

	o += l;
	residue -= l;

	if (i > 1) found = find(C);
	int result = add_kt(found);
	if (result) replacement = true;
	components_to_del = (result == 1);
    }
    AssertEq(o, tag_size);
    if (components_to_del) {
	i = m;
	do {
	    kt.set_component_of(++i);
	} while (delete_kt() == 1);
    }
    if (!replacement) ++item_count;
    Btree_modified = true;
    if (cursor_created_since_last_modification) {
	cursor_created_since_last_modification = false;
	++cursor_version;
    }
}

/* GlassTable::del(key) returns false if the key is not in the B-tree,
   otherwise deletes it and returns true.

   Again, this is parallel to GlassTable::add, but simpler in form.
*/

bool
GlassTable::del(const string &key)
{
    LOGCALL(DB, bool, "GlassTable::del", key);
    Assert(writable);

    if (handle < 0) {
	if (handle == -2) {
	    GlassTable::throw_database_closed();
	}
	RETURN(false);
    }

    // We can't delete a key which we is too long for us to store.
    if (key.size() > GLASS_BTREE_MAX_KEY_LEN) RETURN(false);

    if (key.empty()) RETURN(false);
    form_key(key);

    int r = delete_kt();
    if (r == 0) RETURN(false);
    int i = 1;
    while (r == 1) {
	kt.set_component_of(++i);
	r = delete_kt();
    }

    item_count--;
    Btree_modified = true;
    if (cursor_created_since_last_modification) {
	cursor_created_since_last_modification = false;
	++cursor_version;
    }
    RETURN(true);
}

bool
GlassTable::readahead_key(const string &key) const
{
    LOGCALL(DB, bool, "GlassTable::readahead_key", key);
    Assert(!key.empty());

    // Three cases:
    //
    // handle == -1:  Lazy table in a multi-file database which isn't yet open.
    //
    // handle == -2:  Table has been closed.  Since the readahead is just a
    // hint, we can safely ignore it for a closed table.
    //
    // handle <= -3:  Lazy table in a single-file database which isn't yet
    // open.
    if (handle < 0)
	RETURN(false);

    // If the table only has one level, there are no branch blocks to preread.
    if (level == 0)
	RETURN(false);

    // An overlong key cannot be found.
    if (key.size() > GLASS_BTREE_MAX_KEY_LEN)
	RETURN(true);

    form_key(key);

    // We'll only readahead the first level, since descending the B-tree would
    // require actual reads that would likely hurt performance more than help.
    const uint8_t * p = C[level].get_p();
    int c = find_in_branch(p, kt, C[level].c);
    uint4 n = BItem(p, c).block_given_by();
    // Don't preread if it's the block we last preread or already in the
    // cursor.
    if (n != last_readahead && n != C[level - 1].get_n()) {
	last_readahead = n;
	if (!io_readahead_block(handle, block_size, n, offset))
	    RETURN(false);
    }
    RETURN(true);
}

bool
GlassTable::get_exact_entry(const string &key, string & tag) const
{
    LOGCALL(DB, bool, "GlassTable::get_exact_entry", key | tag);
    Assert(!key.empty());

    if (handle < 0) {
	if (handle == -2) {
	    GlassTable::throw_database_closed();
	}
	RETURN(false);
    }

    // An oversized key can't exist, so attempting to search for it should fail.
    if (key.size() > GLASS_BTREE_MAX_KEY_LEN) RETURN(false);

    form_key(key);
    if (!find(C)) RETURN(false);

    (void)read_tag(C, &tag, false);
    RETURN(true);
}

bool
GlassTable::key_exists(const string &key) const
{
    LOGCALL(DB, bool, "GlassTable::key_exists", key);
    Assert(!key.empty());

    // An oversized key can't exist, so attempting to search for it should fail.
    if (key.size() > GLASS_BTREE_MAX_KEY_LEN) RETURN(false);

    form_key(key);
    RETURN(find(C));
}

bool
GlassTable::read_tag(Glass::Cursor * C_, string *tag, bool keep_compressed) const
{
    LOGCALL(DB, bool, "GlassTable::read_tag", Literal("C_") | tag | keep_compressed);

    tag->resize(0);

    bool first = true;
    bool compressed = false;
    bool decompress = false;
    while (true) {
	LeafItem item(C_[0].get_p(), C_[0].c);
	if (first) {
	    first = false;
	    compressed = item.get_compressed();
	    if (compressed && !keep_compressed) {
		comp_stream.decompress_start();
		decompress = true;
	    }
	}
	bool last = item.last_component();
	if (decompress) {
	    // Decompress each chunk as we read it so we don't need both the
	    // full compressed and uncompressed tags in memory at once.
	    bool done = item.decompress_chunk(comp_stream, *tag);
	    if (done != last) {
		throw Xapian::DatabaseCorruptError(done ?
		    "Too many chunks of compressed data" :
		    "Too few chunks of compressed data");
	    }
	} else {
	    item.append_chunk(tag);
	}
	if (last) break;
	if (!next(C_, 0)) {
	    throw Xapian::DatabaseCorruptError("Unexpected end of table when reading continuation of tag");
	}
    }
    // At this point the cursor is on the last item - calling next will move
    // it to the next key (GlassCursor::read_tag() relies on this).

    RETURN(compressed && keep_compressed);
}

void
GlassTable::set_full_compaction(bool parity)
{
    LOGCALL_VOID(DB, "GlassTable::set_full_compaction", parity);
    Assert(writable);

    if (parity) seq_count = 0;
    full_compaction = parity;
}

GlassCursor * GlassTable::cursor_get() const {
    LOGCALL(DB, GlassCursor *, "GlassTable::cursor_get", NO_ARGS);
    if (handle < 0) {
	if (handle == -2) {
	    GlassTable::throw_database_closed();
	}
	RETURN(NULL);
    }
    // FIXME Ick - casting away const is nasty
    RETURN(new GlassCursor(const_cast<GlassTable *>(this)));
}

/************ B-tree opening and closing ************/

void
GlassTable::basic_open(const RootInfo * root_info, glass_revision_number_t rev)
{
    LOGCALL_VOID(DB, "GlassTable::basic_open", root_info|rev);
    revision_number = rev;
    root =		   root_info->get_root();
    level =		   root_info->get_level();
    item_count =	   root_info->get_num_entries();
    faked_root_block = root_info->get_root_is_fake();
    sequential =	   root_info->get_sequential();
    const string & fl_serialised = root_info->get_free_list();
    if (!fl_serialised.empty()) {
	if (!free_list.unpack(fl_serialised))
	    throw Xapian::DatabaseCorruptError("Bad freelist metadata");
    } else {
	free_list.reset();
    }

    compress_min = root_info->get_compress_min();

    /* kt holds constructed items as well as keys */
    kt = LeafItem_wr(zeroed_new(block_size));

    set_max_item_size(BLOCK_CAPACITY);

    for (int j = 0; j <= level; ++j) {
	C[j].init(block_size);
    }

    read_root();

    if (cursor_created_since_last_modification) {
	cursor_created_since_last_modification = false;
	++cursor_version;
    }
}

void
GlassTable::read_root()
{
    LOGCALL_VOID(DB, "GlassTable::read_root", NO_ARGS);
    if (faked_root_block) {
	/* root block for an unmodified database. */
	uint8_t * p = C[0].init(block_size);
	Assert(p);

	/* clear block - shouldn't be necessary, but is a bit nicer,
	 * and means that the same operations should always produce
	 * the same database. */
	memset(p, 0, block_size);

	int o = block_size - I2 - K1;
	LeafItem_wr(p + o).fake_root_item();

	LeafItem_wr::setD(p, DIR_START, o);         // its directory entry
	SET_DIR_END(p, DIR_START + D2);// the directory size

	o -= (DIR_START + D2);
	SET_MAX_FREE(p, o);
	SET_TOTAL_FREE(p, o);
	SET_LEVEL(p, 0);

	if (!writable) {
	    /* reading - revision number doesn't matter as long as
	     * it's not greater than the current one. */
	    SET_REVISION(p, 0);
	    C[0].set_n(0);
	} else {
	    /* writing - */
	    SET_REVISION(p, revision_number + 1);
	    C[0].set_n(free_list.get_block(this, block_size));
	    C[0].rewrite = true;
	}
    } else {
	/* using a root block stored on disk */
	block_to_cursor(C, level, root);

	if (REVISION(C[level].get_p()) > revision_number) set_overwritten();
	/* although this is unlikely */
    }
}

void
GlassTable::do_open_to_write(const RootInfo * root_info,
			     glass_revision_number_t rev)
{
    LOGCALL_VOID(DB, "GlassTable::do_open_to_write", root_info|rev);
    if (handle == -2) {
	GlassTable::throw_database_closed();
    }
    if (handle <= -2) {
	// Single file database.
	handle = -3 - handle;
    } else {
	handle = io_open_block_wr(name + GLASS_TABLE_EXTENSION, (rev == 0));
	if (handle < 0) {
	    // lazy doesn't make a lot of sense when we're creating a DB (which
	    // is the case when rev==0), but ENOENT with O_CREAT means a parent
	    // directory doesn't exist.
	    if (lazy && rev && errno == ENOENT) {
		revision_number = rev;
		return;
	    }
	    string message((rev == 0) ? "Couldn't create " : "Couldn't open ");
	    message += name;
	    message += GLASS_TABLE_EXTENSION" read/write";
	    throw Xapian::DatabaseOpeningError(message, errno);
	}
    }

    writable = true;
    basic_open(root_info, rev);

    split_p = new uint8_t[block_size];

    buffer = zeroed_new(block_size);

    changed_n = 0;
    changed_c = DIR_START;
    seq_count = SEQ_START_POINT;
}

GlassTable::GlassTable(const char * tablename_, const string & path_,
		       bool readonly_, bool lazy_)
	: tablename(tablename_),
	  revision_number(0),
	  item_count(0),
	  block_size(0),
	  faked_root_block(true),
	  sequential(true),
	  handle(-1),
	  level(0),
	  root(0),
	  kt(0),
	  buffer(0),
	  free_list(),
	  name(path_),
	  seq_count(0),
	  changed_n(0),
	  changed_c(0),
	  max_item_size(0),
	  Btree_modified(false),
	  full_compaction(false),
	  writable(!readonly_),
	  cursor_created_since_last_modification(false),
	  cursor_version(0),
	  changes_obj(NULL),
	  split_p(0),
	  compress_min(0),
	  comp_stream(Z_DEFAULT_STRATEGY),
	  lazy(lazy_),
	  last_readahead(BLK_UNUSED),
	  offset(0)
{
    LOGCALL_CTOR(DB, "GlassTable", tablename_ | path_ | readonly_ | lazy_);
}

GlassTable::GlassTable(const char * tablename_, int fd, off_t offset_,
		       bool readonly_, bool lazy_)
	: tablename(tablename_),
	  revision_number(0),
	  item_count(0),
	  block_size(0),
	  faked_root_block(true),
	  sequential(true),
	  handle(-3 - fd),
	  level(0),
	  root(0),
	  kt(0),
	  buffer(0),
	  free_list(),
	  name(),
	  seq_count(0),
	  changed_n(0),
	  changed_c(0),
	  max_item_size(0),
	  Btree_modified(false),
	  full_compaction(false),
	  writable(!readonly_),
	  cursor_created_since_last_modification(false),
	  cursor_version(0),
	  changes_obj(NULL),
	  split_p(0),
	  compress_min(0),
	  comp_stream(Z_DEFAULT_STRATEGY),
	  lazy(lazy_),
	  last_readahead(BLK_UNUSED),
	  offset(offset_)
{
    LOGCALL_CTOR(DB, "GlassTable", tablename_ | fd | offset_ | readonly_ | lazy_);
}

bool
GlassTable::exists() const {
    LOGCALL(DB, bool, "GlassTable::exists", NO_ARGS);
    // We know a single-file database exists, since we have an fd open on it!
    return single_file() || file_exists(name + GLASS_TABLE_EXTENSION);
}

void
GlassTable::create_and_open(int flags_, const RootInfo & root_info)
{
    LOGCALL_VOID(DB, "GlassTable::create_and_open", flags_|root_info);
    if (handle == -2) {
	GlassTable::throw_database_closed();
    }
    Assert(writable);
    close();

    unsigned int block_size_ = root_info.get_blocksize();
    Assert(block_size_ >= 2048);
    Assert(block_size_ <= BYTE_PAIR_RANGE);
    // Must be a power of two.
    Assert((block_size_ & (block_size_ - 1)) == 0);

    flags = flags_;
    block_size = block_size_;

    if (lazy) {
	close();
	(void)io_unlink(name + GLASS_TABLE_EXTENSION);
	compress_min = root_info.get_compress_min();
    } else {
	// FIXME: it would be good to arrange that this works such that there's
	// always a valid table in place if you run create_and_open() on an
	// existing table.

	do_open_to_write(&root_info);
    }
}

GlassTable::~GlassTable() {
    LOGCALL_DTOR(DB, "GlassTable");
    GlassTable::close();
}

void GlassTable::close(bool permanent) {
    LOGCALL_VOID(DB, "GlassTable::close", permanent);

    if (handle >= 0) {
	if (single_file()) {
	    handle = -3 - handle;
	} else {
	    // If an error occurs here, we just ignore it, since we're just
	    // trying to free everything.
	    (void)::close(handle);
	    handle = -1;
	}
    }

    if (permanent) {
	handle = -2;
	// Don't delete the resources in the table, since they may
	// still be used to look up cached content.
	return;
    }
    for (int j = level; j >= 0; --j) {
	C[j].destroy();
    }
    delete [] split_p;
    split_p = 0;

    delete [] kt.get_address();
    kt = LeafItem_wr(0);
    delete [] buffer;
    buffer = 0;
}

void
GlassTable::flush_db()
{
    LOGCALL_VOID(DB, "GlassTable::flush_db", NO_ARGS);
    Assert(writable);
    if (handle < 0) {
	if (handle == -2) {
	    GlassTable::throw_database_closed();
	}
	return;
    }

    for (int j = level; j >= 0; --j) {
	if (C[j].rewrite) {
	    write_block(C[j].get_n(), C[j].get_p());
	}
    }

    faked_root_block = false;
}

void
GlassTable::commit(glass_revision_number_t revision, RootInfo * root_info)
{
    LOGCALL_VOID(DB, "GlassTable::commit", revision|root_info);
    Assert(writable);

    if (revision <= revision_number) {
	throw Xapian::DatabaseError("New revision too low");
    }

    if (handle < 0) {
	if (handle == -2) {
	    GlassTable::throw_database_closed();
	}
	revision_number = revision;
	root_info->set_blocksize(block_size);
	root_info->set_level(0);
	root_info->set_num_entries(0);
	root_info->set_root_is_fake(true);
	root_info->set_sequential(true);
	root_info->set_root(0);
	return;
    }

    try {
	root = C[level].get_n();

	root_info->set_blocksize(block_size);
	root_info->set_level(level);
	root_info->set_num_entries(item_count);
	root_info->set_root_is_fake(faked_root_block);
	root_info->set_sequential(sequential);
	root_info->set_root(root);

	Btree_modified = false;

	for (int i = 0; i <= level; ++i) {
	    C[i].init(block_size);
	}

	free_list.set_revision(revision);
	free_list.commit(this, block_size);

	// Save the freelist details into the root_info.
	string serialised;
	free_list.pack(serialised);
	root_info->set_free_list(serialised);

	revision_number = revision;

	read_root();

	changed_n = 0;
	changed_c = DIR_START;
	seq_count = SEQ_START_POINT;
    } catch (...) {
	GlassTable::close();
	throw;
    }
}

void
GlassTable::cancel(const RootInfo & root_info, glass_revision_number_t rev)
{
    LOGCALL_VOID(DB, "GlassTable::cancel", root_info|rev);
    Assert(writable);

    if (handle < 0) {
	if (handle == -2) {
	    GlassTable::throw_database_closed();
	}
	return;
    }

    // This causes problems: if (!Btree_modified) return;

    if (flags & Xapian::DB_DANGEROUS)
	throw Xapian::InvalidOperationError("cancel() not supported under Xapian::DB_DANGEROUS");

    revision_number = rev;
    block_size =       root_info.get_blocksize();
    root =             root_info.get_root();
    level =            root_info.get_level();
    item_count =       root_info.get_num_entries();
    faked_root_block = root_info.get_root_is_fake();
    sequential =       root_info.get_sequential();
    const string & fl_serialised = root_info.get_free_list();
    if (!fl_serialised.empty()) {
	if (!free_list.unpack(fl_serialised))
	    throw Xapian::DatabaseCorruptError("Bad freelist metadata");
    } else {
	free_list.reset();
    }

    Btree_modified = false;

    for (int j = 0; j <= level; ++j) {
	C[j].init(block_size);
	C[j].rewrite = false;
    }
    read_root();

    changed_n = 0;
    changed_c = DIR_START;
    seq_count = SEQ_START_POINT;

    if (cursor_created_since_last_modification) {
	cursor_created_since_last_modification = false;
	++cursor_version;
    }
}

/************ B-tree reading ************/

void
GlassTable::do_open_to_read(const RootInfo * root_info,
			    glass_revision_number_t rev)
{
    LOGCALL(DB, bool, "GlassTable::do_open_to_read", root_info|rev);
    if (handle == -2) {
	GlassTable::throw_database_closed();
    }
    if (single_file()) {
	handle = -3 - handle;
    } else {
	handle = io_open_block_rd(name + GLASS_TABLE_EXTENSION);
	if (handle < 0) {
	    if (lazy) {
		// This table is optional when reading!
		revision_number = rev;
		return;
	    }
	    string message("Couldn't open ");
	    message += name;
	    message += GLASS_TABLE_EXTENSION" to read";
	    throw Xapian::DatabaseOpeningError(message, errno);
	}
    }

    basic_open(root_info, rev);

    read_root();
}

void
GlassTable::open(int flags_, const RootInfo & root_info,
		 glass_revision_number_t rev)
{
    LOGCALL_VOID(DB, "GlassTable::open", flags_|root_info|rev);
    close();

    flags = flags_;
    block_size = root_info.get_blocksize();
    root = root_info.get_root();

    if (!writable) {
	do_open_to_read(&root_info, rev);
	return;
    }

    do_open_to_write(&root_info, rev);
}

bool
GlassTable::prev_for_sequential(Glass::Cursor * C_, int /*dummy*/) const
{
    LOGCALL(DB, bool, "GlassTable::prev_for_sequential", Literal("C_") | Literal("/*dummy*/"));
    int c = C_[0].c;
    AssertRel(DIR_START,<=,c);
    AssertRel(c,<,DIR_END(C_[0].get_p()));
    if (c == DIR_START) {
	uint4 n = C_[0].get_n();
	const uint8_t * p;
	while (true) {
	    if (n == 0) RETURN(false);
	    n--;
	    if (n == C[0].get_n()) {
		// Block is a leaf block in the built-in cursor (potentially in
		// modified form if the table is writable).
		p = C_[0].clone(C[0]);
	    } else {
		if (writable) {
		    // Blocks in the built-in cursor may not have been written
		    // to disk yet, so we have to check that the block number
		    // isn't in the built-in cursor or we'll read an
		    // uninitialised block (for which GET_LEVEL(p) will
		    // probably return 0).
		    int j;
		    for (j = 1; j <= level; ++j) {
			if (n == C[j].get_n()) break;
		    }
		    if (j <= level) continue;
		}

		// Block isn't in the built-in cursor, so the form on disk
		// is valid, so read it to check if it's the next level 0
		// block.
		uint8_t * q = C_[0].init(block_size);
		read_block(n, q);
		p = q;
		C_[0].set_n(n);
	    }
	    if (REVISION(p) > revision_number + writable) {
		set_overwritten();
		RETURN(false);
	    }
	    if (GET_LEVEL(p) == 0) break;
	}
	c = DIR_END(p);
	AssertRel(DIR_START,<,c);
    }
    c -= D2;
    C_[0].c = c;
    RETURN(true);
}

bool
GlassTable::next_for_sequential(Glass::Cursor * C_, int /*dummy*/) const
{
    LOGCALL(DB, bool, "GlassTable::next_for_sequential", Literal("C_") | Literal("/*dummy*/"));
    const uint8_t * p = C_[0].get_p();
    Assert(p);
    int c = C_[0].c;
    AssertRel(c,<,DIR_END(p));
    c += D2;
    Assert(unsigned(c) < block_size);
    if (c == DIR_END(p)) {
	uint4 n = C_[0].get_n();
	while (true) {
	    n++;
	    if (n >= free_list.get_first_unused_block()) RETURN(false);
	    if (writable) {
		if (n == C[0].get_n()) {
		    // Block is a leaf block in the built-in cursor
		    // (potentially in modified form).
		    p = C_[0].clone(C[0]);
		} else {
		    // Blocks in the built-in cursor may not have been written
		    // to disk yet, so we have to check that the block number
		    // isn't in the built-in cursor or we'll read an
		    // uninitialised block (for which GET_LEVEL(p) will
		    // probably return 0).
		    int j;
		    for (j = 1; j <= level; ++j) {
			if (n == C[j].get_n()) break;
		    }
		    if (j <= level) continue;

		    // Block isn't in the built-in cursor, so the form on disk
		    // is valid, so read it to check if it's the next level 0
		    // block.
		    uint8_t * q = C_[0].init(block_size);
		    read_block(n, q);
		    p = q;
		}
	    } else {
		uint8_t * q = C_[0].init(block_size);
		read_block(n, q);
		p = q;
	    }
	    if (REVISION(p) > revision_number + writable) {
		set_overwritten();
		RETURN(false);
	    }
	    if (GET_LEVEL(p) == 0) break;
	}
	c = DIR_START;
	C_[0].set_n(n);
    }
    C_[0].c = c;
    RETURN(true);
}

bool
GlassTable::prev_default(Glass::Cursor * C_, int j) const
{
    LOGCALL(DB, bool, "GlassTable::prev_default", Literal("C_") | j);
    const uint8_t * p = C_[j].get_p();
    int c = C_[j].c;
    AssertRel(DIR_START,<=,c);
    AssertRel(c,<,DIR_END(p));
    AssertRel(unsigned(DIR_END(p)),<=,block_size);
    if (c == DIR_START) {
	if (j == level) RETURN(false);
	if (!prev_default(C_, j + 1)) RETURN(false);
	p = C_[j].get_p();
	c = DIR_END(p);
	AssertRel(DIR_START,<,c);
    }
    c -= D2;
    C_[j].c = c;
    if (j > 0) {
	block_to_cursor(C_, j - 1, BItem(p, c).block_given_by());
    }
    RETURN(true);
}

bool
GlassTable::next_default(Glass::Cursor * C_, int j) const
{
    LOGCALL(DB, bool, "GlassTable::next_default", Literal("C_") | j);
    const uint8_t * p = C_[j].get_p();
    int c = C_[j].c;
    AssertRel(c,<,DIR_END(p));
    AssertRel(unsigned(DIR_END(p)),<=,block_size);
    c += D2;
    if (j > 0) {
	AssertRel(DIR_START,<,c);
    } else {
	AssertRel(DIR_START,<=,c);
    }
    // Sometimes c can be DIR_END(p) + 2 here it appears...
    if (c >= DIR_END(p)) {
	if (j == level) RETURN(false);
	if (!next_default(C_, j + 1)) RETURN(false);
	p = C_[j].get_p();
	c = DIR_START;
    }
    C_[j].c = c;
    if (j > 0) {
	block_to_cursor(C_, j - 1, BItem(p, c).block_given_by());
#ifdef BTREE_DEBUG_FULL
	printf("Block in GlassTable:next_default");
	report_block_full(j - 1, C_[j - 1].get_n(), C_[j - 1].get_p());
#endif /* BTREE_DEBUG_FULL */
    }
    RETURN(true);
}

void
GlassTable::throw_database_closed()
{
    throw Xapian::DatabaseClosedError("Database has been closed");
}
