/** @file brass_table.cc
 * @brief Brass B-tree table.
 */
/* Copyright (C) 2010 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

/* TODO:
 *  + Track free blocks and slabs!
 *  + Prefix-compressed keys.  Store the prefix removed in the block (at the
 *    end, except for the left ptr in branch blocks).  Also, allow a shorter
 *    than stored prefix to be specified by using length bytes (256 - prefixlen
 *    to 255) to mean "ignore last X characters of prefix".
 *  + Factor out common code from different versions of BrassBlock::insert()?
 *  + B*tree style update - rotate items between 2 neighbouring sibling blocks
 *    to avoid needing to split so soon.
 *  + Don't seem to need to store depth of tree, but it's an interesting
 *    statistic, and easy to calculate (just follow down to a leaf block
 *    and count the levels), so provide a method to calculate it?
 */

#include <config.h>

#include "brass_table.h"

#include "brass_cursor.h"

#include "xapian/error.h"

// Trying to include the correct headers with the correct defines set to get
// pread() and pwrite() prototyped on every platform without breaking any other
// platform is a real can of worms.  So instead configure probes for what
// (if any) prototypes are required and puts them into PREAD_PROTOTYPE and
// PWRITE_PROTOTYPE.
#if defined HAVE_PREAD && defined PREAD_PROTOTYPE
PREAD_PROTOTYPE
#endif
#if defined HAVE_PWRITE && defined PWRITE_PROTOTYPE
PWRITE_PROTOTYPE
#endif

#include "safeerrno.h"
#include "safefcntl.h"
#include <cstdio>
#include <cstring>
#include "safesysstat.h"
#include <sys/types.h>
#include "safeunistd.h"

#include "debuglog.h"
#include "omassert.h"
#include "pack.h"
#include "utils.h"

#include <algorithm>
#include <exception> // For uncaught_exception()
#include <string>

#include <iostream> // temporarily

using namespace std;

// Zero out bits of blocks which become unused.  Mostly useful for debugging.
// If we want to provide this as a "deleted data is gone" option, we can just
// zero the freespace in the middle of a block when we write it to disk.
#define ZERO_UNUSED_SPACE

// FIXME: benchmark if it is better/worse/no different to put items at end
// and pointers at start.  It is likely to be less confusing to follow the
// code for if nothing else!

/*
  Header (HEADER_SIZE bytes):

      [ | | | ] 4 bytes: Revision
      [ | ]     2 bytes: bits 0-14 #items in this block (n)
			 bit 15: 1 for non-leaf block
      [ ]       1 byte:  length of key prefix (unimplemented)
      [ ]       1 byte:  (temporarily: levels above leaf) - reserved

  Item pointers:

      [ | ] offset to start of item 0
       ...
      [ | ] offset to start of item n-1

  [Free space]

  Items:

      [ Item n - 1 ]
       ...
      [ Item 0 ]

  [ Left-most block pointer ] ( non-leaf blocks only )
  <end of block>
 */

/* Leaf Item:
 *
 * New:
 * [ ] 1 byte: L
 *     bits 0-6 :
 *	   0 => not compressed, length = (L >> 7)
 *         1-63 => compressed = (L >> 7), length = (L & 0x3f) + 1
 *         64-127 => compressed = (L >> 7), length = ((L >> 2) & 0x0f) + 1,
 *		ptr = (L & 0x03) << (8 * length) | <pointed to value>
 * [ key ... ]
 * [ tag ... ]
 */

/* Branch Item:
 *
 * [ block pointer ] (BLOCKPTR_SIZE bytes)
 * [ key ... ]
 *
 * Keys >= the branch item's key (and < the next branch item's key if this
 * isn't the last entry in this block) belong in the subtree pointed to by the
 * block pointer.
 */

void
BrassCompressor::throw_zlib_error(int res, z_stream * zstream, const char * msg_)
{
    // Translate Z_MEM_ERROR to std::bad_alloc for consistency.
    if (res == Z_MEM_ERROR)
	throw std::bad_alloc();

    string msg(msg_);
    if (zstream->msg) {
	msg += zstream->msg;
    } else {
	// If there's no message then report the error code.
	msg += str(res);
    }
    throw Xapian::DatabaseError(msg);
}

void
BrassTable::throw_database_closed()
{
    throw Xapian::DatabaseError("Database has been closed");
}

void
BrassBlock::check_block(const string &lb, const string &ub)
{
    if (uncaught_exception())
	return;
    Assert(data);
    int C = get_count();
    if (C == 0)
	return;
//    cout << "\n" << (void*)this << "::check_block: " << n;
//    cout << (is_leaf() ? " leaf " : " branch ");
//    cout << C << " entries : " << lb << " <= " << get_key(0) << " < ";
//    if (C > 1)
//	cout << get_key(C - 1) << " < ";
//    cout << ub << endl;

    string prev_key;
    string key;
    int header_len = header_length(C);
    for (int i = 0; i < C; ++i) {
	int ptr = get_ptr(i);
	int endptr = get_endptr(i);
	// Items shouldn't overlap the block header.
	if (rare(ptr < header_len)) {
	    string msg("Block ");
	    msg += str(n);
	    msg += ": Item ";
	    msg += str(i);
	    msg += " overlaps block header";
	    throw Xapian::DatabaseCorruptError(msg);
	}
	// Items should have non-negative size.
	if (rare(endptr < ptr)) {
	    string msg("Block ");
	    msg += str(n);
	    msg += ": Item ";
	    msg += str(i);
	    msg += " has negative size";
	    throw Xapian::DatabaseCorruptError(msg);
	}
	if (is_leaf()) {
	    byte ch = static_cast<byte>(data[ptr++]);
	    int tag_len;
	    if ((ch & 0x40) == 0) {
		if ((ch & 0x7f) == 0) {
		    tag_len = (ch >> 7);
		} else {
		    tag_len = (ch & 0x3f) + 1;
		}
	    } else {
		tag_len = ((ch >> 2) & 0x0f) + 1;
	    }
	    if (rare(tag_len > 64)) {
		string msg("Leaf block ");
		msg += str(n);
		msg += ": Item ";
		msg += str(i);
		msg += " tag length ";
		msg += str(tag_len);
		msg += " > 64 bytes";
		throw Xapian::DatabaseCorruptError(msg);
	    }
	    if (rare(endptr - ptr < tag_len)) {
		string msg("Leaf block ");
		msg += str(n);
		msg += ": Item ";
		msg += str(i);
		msg += " tag length ";
		msg += str(tag_len);
		msg += " > length ";
		msg += str(endptr - ptr);
		throw Xapian::DatabaseCorruptError(msg);
	    }
	    int key_len = endptr - ptr - tag_len;
	    key.assign(data + ptr, key_len);
	} else {
	    ptr += BLOCKPTR_SIZE;
	    if (rare(endptr - ptr >= 256)) {
		string msg("Branch block ");
		msg += str(n);
		msg += ": Item ";
		msg += str(i);
		msg += " key length ";
		msg += str(endptr - ptr);
		msg += " >= 256";
		throw Xapian::DatabaseCorruptError(msg);
	    }
	    key.assign(data + ptr, endptr - ptr);
	}
	if (i == 0) {
	    if (rare(key < lb)) {
		string msg("Block ");
		msg += str(n);
		msg += ": Key ";
		msg += str(i);
		msg += " '";
		msg += key;
		msg += "' < lower bound '";
		msg += lb;
		msg += "'";
		throw Xapian::DatabaseCorruptError(msg);
	    }
	} else {
	    if (rare(key <= prev_key)) {
		string msg("Block ");
		msg += str(n);
		msg += ": Key ";
		msg += str(i);
		msg += " '";
		msg += key;
		msg += "' <= previous key '";
		msg += prev_key;
		msg += "'";
		throw Xapian::DatabaseCorruptError(msg);
	    }
	}
	// FIXME: More checks?

	swap(prev_key, key);
    }
    if (!ub.empty() && rare(prev_key >= ub)) {
	string msg("Block ");
	msg += str(n);
	msg += ": Key ";
	msg += str(C - 1);
	msg += " '";
	msg += prev_key;
	msg += "' <= upper bound '";
	msg += ub;
	msg += "'";
	throw Xapian::DatabaseCorruptError(msg);
    }

    // FIXME: More checks?
}

void
BrassBlock::save()
{
    LOGCALL_VOID(DB, "BrassBlock::save", NO_ARGS);
    if (!data)
	return;
    set_revision(table.revision);
    CHECK_BLOCK();
    if (!table.write_block(data, n))
	throw Xapian::DatabaseError("Failed to write block " + str(n), errno);
}

BrassBlock::~BrassBlock()
{
    LOGCALL_DTOR(DB, "BrassBlock");
    delete [] data;
}

brass_block_t
BrassBlock::get_left_block() const
{
    LOGCALL(DB, brass_block_t, "BrassBlock::get_left_block", NO_ARGS);
    Assert(!is_leaf());
    RETURN(LE(*(brass_block_t*)(data + table.blocksize - BLOCKPTR_SIZE)));
}

void
BrassBlock::set_left_block(brass_block_t b)
{
    LOGCALL_VOID(DB, "BrassBlock::set_left_block", b);
    Assert(!is_leaf());
    *(brass_block_t*)(data + table.blocksize - BLOCKPTR_SIZE) = LE(b);
}

void
BrassBlock::new_leaf_block()
{
    LOGCALL_VOID(DB, "BrassBlock::new_leaf_block", NO_ARGS);
    if (!data)
	data = new char[table.blocksize];
    n = const_cast<BrassTable&>(table).get_free_block();
    // Create empty leaf block.
    memset(data, 0, table.blocksize);
    random_access = RANDOM_ACCESS_THRESHOLD;
    Assert(is_leaf());
}

void
BrassBlock::new_branch_block()
{
    LOGCALL_VOID(DB, "BrassBlock::new_branch_block", NO_ARGS);
    new_leaf_block();
    // Set the branch block flag.
    data[5] = '\x80';
    Assert(!is_leaf());
}

void
BrassCBlock::check_block()
{
    std::string lb, ub;
    if (parent) {
	int C = parent->get_count();
	int i = parent->item;
	if (i >= 0)
	    lb = parent->get_key(i);
	if (i + 1 < C)
	    ub = parent->get_key(i + 1);
	AssertEq(get_level() + 1, parent->get_level());
    }
    BrassBlock::check_block(lb, ub);
}

BrassCBlock::~BrassCBlock()
{
    LOGCALL_DTOR(DB, "BrassCBlock");
    if (modified)
	save();
    delete child;
}

bool
BrassCBlock::read_tag(string &tag)
{
    AssertRel(item,>=,-1);
    AssertRel(item,<,get_count());
    if (!is_leaf())
	return child->read_tag(tag);

    const char * s;
    size_t key_len;
    bool slab;
    bool compressed = decode_leaf_key(item, s, key_len, slab);
    s += key_len;
    AssertRel(s - data,<=,get_endptr(item));
    if (slab) {
	if (table.fd_slab == table.FD_NOT_OPEN)
	    const_cast<BrassTable&>(table).open_slab_file();
	size_t tag_len;
	uint8 slab_pos;
	const char * end = data + get_endptr(item);
	if (rare(!unpack_uint(&s, end, &tag_len)))
	    throw Xapian::DatabaseCorruptError("Reading slab length");
	if (rare(!unpack_uint_last(&s, end, &slab_pos)))
	    throw Xapian::DatabaseCorruptError("Reading slab pointer");
	char * tag_buf = new char[tag_len];
	try {
	    if (!table.read_slab(tag_buf, tag_len, slab_pos))
		throw Xapian::DatabaseError("Failed to read slab " + str(slab_pos), errno);

	    if (!compressed) {
		tag.assign(tag_buf, tag_len);
	    } else {
		table.compressor.decompress(tag_buf, tag_len, tag);
	    }
	} catch (...) {
	    delete[] tag_buf;
	    throw;
	}
	delete[] tag_buf;
    } else {
	if (!compressed) {
	    tag.assign(s, data + get_endptr(item) - s);
	} else {
	    table.compressor.decompress(s, data + get_endptr(item) - s, tag);
	}
    }
    return true;
}

void
BrassCBlock::read(brass_block_t blk)
{
    LOGCALL_VOID(DB, "BrassBlock::read", blk);
    if (data) {
	if (n == blk)
	    return;
	if (modified)
	    save();
    } else {
	// FIXME use anon mmap or some sort of pool?
	data = new char[table.blocksize];
    }

    // FIXME: we only need to snoop the table's cursor if we aren't the table's
    // cursor, and we only need to look at the same level we are...
    for (BrassCBlock * c = table.my_cursor; c != NULL; c = c->child) {
	// If we find ourself in the table's cursor chain, then there's nothing
	// to snoop.
	if (c == this)
	    break;
	if (c->data && c->n == blk) {
	    // FIXME: we could share blocks using copy-on-write-or-move...
	    memcpy(data, c->data, table.blocksize);
	    goto snooped;
	}
    }

    if (!table.read_block(data, blk))
	throw Xapian::DatabaseError("Failed to read block " + str(blk), errno);

snooped:
    n = blk;
    CHECK_BLOCK();
    modified = false;
    // If the block's revision is the one we are currently working on, then it
    // must have been allocated freshly for this revision, and so we don't need
    // to clone it.
    needs_clone = (get_revision() != table.revision);
    item = -2;
    random_access = RANDOM_ACCESS_THRESHOLD;
    if (is_leaf() && child != NULL) {
	string msg = "Expected block ";
	msg += str(n);
	msg += " to be a branch block not a leaf block";
	throw Xapian::DatabaseCorruptError(msg);
    }
}

int
BrassBlock::get_endptr(int i) const
{
    LOGCALL(DB, int, "BrassBlock::get_endptr", i);
    AssertRel(i,>=,0);
    AssertRel(i,<=,get_count());
    if (i)
	RETURN(get_ptr(i - 1));
    RETURN(table.blocksize - (is_leaf() ? 0 : BLOCKPTR_SIZE));
}

bool
BrassCBlock::binary_chop_leaf(const string & key, int mode)
{
    LOGCALL(DB, bool, "BrassCBlock::binary_chop_leaf", key | mode);
    Assert(is_leaf());
    (void)mode; // FIXME
    int b = 0;
    int e = get_count();
    if (e == 0) {
	// FIXME fold in better
	item = 0;
	RETURN(false);
    }

#if 1 //def BINARY_CHOP
    // An access will often be just after the previous one, so if item is set,
    // use it to pick the first chop point.
    int m = (b + e) >> 1;
    if (item >= m) {
	if (item >= e)
	    item = e - 1;
	m = item;
    } else if (item >= 0) {
	// We expect the point we want is just after item, but chopping at item
	// will less than halve the range, so chop just after it instead.
	m = item + 1;
    }

    do {
	AssertRel(m,>=,0);
	AssertRel(m,<,get_count());
	const char * key_data;
	size_t key_len;
	bool slab;
	(void)decode_leaf_key(m, key_data, key_len, slab);
	int cmp = table.compare_keys(key_data, key_len, key.data(), key.size());
	if (cmp < 0) {
	    b = m + 1;
	} else if (cmp > 0) {
	    e = m;
	} else {
	    // Exact match.
	    AssertEq(cmp, 0);
	    item = m;
	    AssertRel(get_key(item),==,string(key_data, key_len));
	    AssertRel(get_key(item),==,key);
	    RETURN(true);
	}
	m = (b + e) >> 1;
    } while (b < e);
#else
    // Linear variant for debugging.  O(n) rather than O(log n), so slower.
    while (b < e) {
	int m = b;
	const char * key_data;
	size_t key_len;
	bool slab;
	(void)decode_leaf_key(m, key_data, key_len, slab);
	int cmp = table.compare_keys(key_data, key_len, key.data(), key.size());
	if (cmp < 0) {
	    // Keep going.
	} else if (cmp > 0) {
	    // Overshot.
	    break;
	} else {
	    // Exact match.
	    AssertEq(cmp, 0);
	    item = m;
	    AssertRel(get_key(item),==,string(key_data, key_len));
	    AssertRel(get_key(item),==,key);
	    RETURN(true);
	}
	++b;
    }
#endif
    if (item + 1 == b) {
	if (random_access)
	    --random_access;
    } else {
	random_access = RANDOM_ACCESS_THRESHOLD;
    }
    item = b;
    AssertRel(item,<=,get_count());
    if (item < get_count()) {
	AssertRel(get_key(item),>,key);
    }
    if (item > 0) {
	AssertRel(get_key(item - 1),<=,key);
    }
    RETURN(false);
}

void
BrassCBlock::insert(const string &key, brass_block_t blk, brass_block_t n_child)
{
    LOGCALL_VOID(DB, "BrassCBlock::insert", key | blk | n_child);
    Assert(n_child == get_block(item) || n_child == blk);
    Assert(!is_leaf());

    // If we needed to be cloned, that should have happened when our descendent
    // leaf block was.
    Assert(!needs_clone);

    CHECK_BLOCK();

    int C = get_count();
    if (C == 0) {
	// FIXME: special case probably not needed, but perhaps could be
	// merged into gain_level()...
	set_count(1);
	int new_ptr = table.blocksize - 2 * BLOCKPTR_SIZE - key.size();
	set_ptr(0, new_ptr);
	if (n_child == blk)
	    item = 0;
	set_unaligned_le4(data + new_ptr, blk);
	memcpy(data + new_ptr + BLOCKPTR_SIZE, key.data(), key.size());
	modified = true;
	AssertEq(get_block(item), n_child);
	CHECK_BLOCK();
	return;
    }

    // We're splitting the current child block, so insert just after item.
    int b = item + 1;
    if (item >= 0)
	AssertRel(get_key(item),<,key);
    if (b < C)
	AssertRel(key,<,get_key(b));

    int len = BLOCKPTR_SIZE + key.size();
    int free_end = get_ptr(C - 1);
    if (len + 2 <= free_end - header_length(C)) {
	// There's enough room in the block already.
	insert_entry(b, key, blk);
	if (n_child == blk) {
	    item = b;
	}
	modified = true;
	AssertEq(get_block(item), n_child);
	CHECK_BLOCK();
	return;
    }

    // Split the block.
    AssertRel(C,>,1);
    int split_at;
    int split_ptr;
    bool insert_in_left;
    if (!random_access) {
	// In sequential insert mode, split at the insertion point which
	// gives us almost full blocks.
	// b is item + 1, so must be >= 0 and <= C.
	split_at = b;
	split_ptr = get_endptr(split_at);
	// In sequential mode, splitting at the insertion point might not
	// create enough space in the left block.
	insert_in_left = (len + 2 <= split_ptr - header_length(split_at));
    } else {
	// In random insert mode, split evenly as that gives us the
	// amortised Btree performance guarantees.
	// FIXME: pick middle point better...
	split_at = C >> 1;
	split_ptr = get_endptr(split_at);
	// If split_at is before or after the insertion point, that determines
	// which block we insert into.  If split_at is the insertion point, we
	// must now have enough space in the left block so insert there.
	insert_in_left = (b <= split_at);
	if (insert_in_left) {
	    AssertRel(len + 2,<=,split_ptr - header_length(split_at));
	}
    }

    string div_key;
    brass_block_t sp_new_l;
    int sp_from;
    if (split_at == b && !insert_in_left) {
	Assert(!random_access);
	div_key = key;
	sp_new_l = blk;
	sp_from = split_at;
    } else {
	// We can't further shorten the dividing key here or it'll disagree
	// with the existing partitioning of keys at the leaf level.
	int div_ptr = get_ptr(split_at) + BLOCKPTR_SIZE;
	div_key.assign(data + div_ptr, split_ptr - div_ptr);
	AssertRel(split_at,<,C);
	sp_new_l = get_block(split_at);
	sp_from = split_at + 1;
    }

    // We put the right half of the split block in the new block, and
    // then swap the blocks around later if this is the one we want to keep.
    BrassBlock & sp = const_cast<BrassTable&>(table).split;
    sp.new_branch_block();
    sp.set_level(get_level());
    int sp_ptr = get_endptr(sp_from);
    int sp_len = sp_ptr - free_end;
    memcpy(sp.data + table.blocksize - BLOCKPTR_SIZE - sp_len, data + free_end, sp_len);
    sp.set_left_block(sp_new_l);
    for (int i = sp_from; i < C; ++i) {
	sp.set_ptr(i - sp_from, get_ptr(i) + table.blocksize - BLOCKPTR_SIZE - sp_ptr);
    }
    sp.set_count(C - sp_from);
    set_count(split_at);
#ifdef ZERO_UNUSED_SPACE
    memset(data + free_end, 0, split_ptr - free_end);
#endif
    brass_block_t n_left = n, n_right = sp.n;
    if (insert_in_left) {
	AssertRel(key,<,div_key);
	insert_entry(b, key, blk);
	if (n_child == blk) {
	    item = b;
	}
    } else {
	AssertRel(key,>=,div_key);
	if (sp_new_l == blk) {
	    // The new item forms the new dividing key and sp's left pointer.
	    if (n_child == blk) {
		swap(data, sp.data);
		swap(n, sp.n);
		item = -1;
	    }
	} else {
	    // FIXME: It would be more efficient if we made space for the item
	    // as we copied above, rather than performing the splitting and
	    // then inserting as separate operations.
	    sp.insert_entry(b - sp_from, key, blk);
	    swap(data, sp.data);
	    swap(n, sp.n);
	    if (n_child == blk)
		item = b;
	    item -= sp_from;
	}
    }
    AssertEq(get_block(item), n_child);
    // We added an item, but splitting the block means there's an extra
    // left pointer, so the total should be unchanged.
    AssertEq(C, sp.get_count() + get_count());
    sp.save();
    modified = true;

    // And now insert into parent, adding a level to the Btree if required.
    if (!parent)
	parent = const_cast<BrassTable&>(table).gain_level(n_left);
    parent->insert(div_key, n_right, n);

    CHECK_BLOCK();
}

void
BrassBlock::insert_entry(int b, const string &key, brass_block_t blk)
{
    AssertRel(b,>=,0);
    int len = BLOCKPTR_SIZE + key.size();
    int C = get_count();
    AssertRel(C,>,0);
    set_count(C + 1);
    int free_end = get_ptr(C - 1);
    // Check that we actually have enough free space!
    AssertRel(len + 2,<=,free_end - header_length(C));
    int new_ptr = get_endptr(b);
    if (b < C) {
	// Need to insert in sorted order, so shuffle existing entries along.
	// We need to move entries [b, C) down by len bytes.
	char * q = data + free_end;
	size_t l = new_ptr - free_end;
	memmove(q - len, q, l);
	for (int i = C; i > b; --i) {
	    set_ptr(i, get_ptr(i - 1) - len);
	}
    }
    new_ptr -= len;
    set_ptr(b, new_ptr);
    set_unaligned_le4(data + new_ptr, blk);
    memcpy(data + new_ptr + BLOCKPTR_SIZE, key.data(), key.size());
}

void
BrassCBlock::find_child(const string & key)
{
    LOGCALL_VOID(DB, "BrassCBlock::find_child", key);
    Assert(!is_leaf());
    int b = 0;
    int e = get_count() - 1;
    if (e == -1) {
	// We only have a left pointer.
	item = -1;
    } else {
	// An access will often be just after the previous one, so if item is
	// set, use it to pick the first chop point.
	int m = (b + e) >> 1;
	if (item >= m) {
	    if (item > e)
		item = e;
	    m = item;
	} else if (item >= -1) {
	    // We expect the point we want is just after item, but chopping at
	    // item will less than halve the range, so chop just after it
	    // instead.
	    m = item + 1;
	}

	do {
	    AssertRel(m,>=,0);
	    int key_start = get_ptr(m) + BLOCKPTR_SIZE;
	    int key_len = get_endptr(m) - key_start;
	    int cmp = table.compare_keys(data + key_start, key_len,
					 key.data(), key.size());
	    if (cmp < 0) {
		b = m + 1;
	    } else if (cmp > 0) {
		e = m - 1;
	    } else {
		// Exact match.
		e = m;
		break;
	    }
	    m = (b + e) >> 1;
	} while (b <= e);
	item = e;
    }

    if (item >= 0) {
	AssertRel(get_key(item),<=,key);
    }
    if (item < get_count() - 1) {
	AssertRel(key,<,get_key(item+1));
    }

    brass_block_t blk;
    if (item < 0) {
	blk = get_left_block();
    } else {
	blk = get_block(item);
    }
    // cout << "find_child found item " << item << " -> block " << blk << endl;
    if (!child)
	child = new BrassCBlock(table, this);
    child->read(blk);
}

void
BrassCBlock::insert(const string &key, const char * tag, size_t tag_len,
		    bool compressed)
{
    LOGCALL_VOID(DB, "BrassCBlock::insert", key | (void*)tag | tag_len | compressed);
    if (!is_leaf()) {
	find_child(key);
	child->insert(key, tag, tag_len, compressed);
	return;
    }

    if (needs_clone) {
	needs_clone = false;
	n = const_cast<BrassTable&>(table).get_free_block();
	if (parent)
	    parent->set_child_block_number(n);
    }

    CHECK_BLOCK();

    int C = get_count();

    bool exact = binary_chop_leaf(key, LE);
    // cout << n << ": insert (exact=" << exact << ") at " << item << "/" << C << endl;
    AssertRel(item,<=,C);

    string slab_pointer; // FIXME: use an auto char array?
    byte info;
    if (tag_len > MAX_INLINE_TAG_SIZE) {
	// FIXME: eliminate uses of const_cast.
	if (table.fd_slab == table.FD_NOT_OPEN)
	    const_cast<BrassTable&>(table).open_slab_file();
	off_t slab_pos = const_cast<BrassTable&>(table).get_free_slab(tag_len);
	if (!table.write_slab(tag, tag_len, slab_pos))
	    throw Xapian::DatabaseError("Failed to write slab " + str(slab_pos), errno);

	pack_uint(slab_pointer, tag_len);
	pack_uint_last(slab_pointer, uint8(slab_pos));
	tag = slab_pointer.data();
	tag_len = slab_pointer.size();

	if (rare(tag_len > 16))
	    throw Xapian::UnimplementedError("slab pointer too big");
	info = (tag_len - 1) << 2;
	info |= 0x40;
	if (compressed)
	    info |= 0x80;
	// FIXME: make use of the bottom 3 bits to store the start of the
	// pointer (or to allow longer inline tags).
    } else if (tag_len > 1) {
	info = (tag_len - 1);
	if (compressed)
	    info |= 0x80;
    } else {
	info = (tag_len << 7);
    }

    int len = tag_len + key.size() + 1;
    // Set free_end to the offset of the end of free space.
    int free_end = get_endptr(C);
    int oldlen = 0;
    // Check if the item fits!
    bool need_to_split = false;
    if (exact) {
	// We're replacing an existing item.
	oldlen = get_endptr(item) - get_ptr(item);
	if (len > oldlen && len - oldlen > free_end - header_length(C))
	    need_to_split = true;
	// cout << "*** " << len << " " << oldlen << " " << need_to_split << endl;
    } else {
	// cerr << len << " + 2 > " << free_end << " - " << header_length(C) << " ?" << endl;
	if (len + 2 > free_end - header_length(C))
	    need_to_split = true;
    }
    //cout << "need_to_split = " << need_to_split << endl;
    if (need_to_split) {
	// Split the block - if in random insert mode, split evenly as
	// that gives us the amortised Btree performance guarantees.
	//
	// If in sequential insert mode, then split at the insertion point
	// which gives us almost full blocks.
	//
	// FIXME: split_at is "after" in the sense of the order of entries
	// in the block, but "before" in item numbering order.  This is already
	// confusing me!
	int split_ptr;
	int split_at;
	if (!random_access) {
	    if (item <= 1) {
		split_at = item;
	    } else {
		split_at = item - 1;
	    }
	    split_ptr = get_endptr(split_at);
	} else {
	    split_at = C >> 1;
	    do {
		split_ptr = get_endptr(split_at);
		int percent = (table.blocksize - split_ptr) * 100;
		percent /= (table.blocksize - free_end);
		if (percent > 75) {
		    // cout << "split @ " << percent << "% " << split_at<<"/"<<C<<" too late" << endl;
		    --split_at;
		    AssertRel(split_at,>,0);
		    continue;
		}
		if (percent < 25) {
		    // cout << "split @ " << percent << "% " << split_at<<"/"<<C<<" too early" << endl;
		    ++split_at;
		    AssertRel(split_at,<,C - 1);
		    continue;
		}
		// cout << "split @ " << percent << "% " << split_at<<"/"<<C<<" acceptable" << endl;
		break;
	    } while (true);
	}

	// cout << "split : item " << item << " split point " << split_at << " out of " << C << endl;
	const char *div_p;
	size_t div_len;
	{
	    bool slab;
	    (void)decode_leaf_key(split_at, div_p, div_len, slab);
	}

	AssertRel(split_ptr,==,get_ptr(split_at - 1));

	const char *pre_p;
	size_t pre_len;
	if (split_at == 0) {
	    AssertEq(item, 0);
	    pre_p = key.data();
	    pre_len = key.size();
	} else {
	    bool slab;
	    (void)decode_leaf_key(split_at - 1, pre_p, pre_len, slab);
	}

	string divkey(table.divide(pre_p, pre_len, div_p, div_len));
	// cout << "splitting leaf block " << n << " at " << divkey << endl;
	// We want the right half of the split block to go in the new block so
	// that if the whole tree is created in one revision, we can do
	// next/previous leaf block by simply moving forwards/backwards in
	// block number order skipping non-leaf blocks.
	BrassBlock & sp = const_cast<BrassTable&>(table).split;
	sp.new_leaf_block();
	memcpy(sp.data + table.blocksize - (split_ptr - free_end), data + free_end, split_ptr - free_end);
#ifdef ZERO_UNUSED_SPACE
	memset(data + free_end, 0, split_ptr - free_end);
#endif
	for (int i = split_at; i < C; ++i) {
	    sp.set_ptr(i - split_at, get_ptr(i) + (table.blocksize - split_ptr));
	}
	sp.set_count(C - split_at);
	set_count(split_at);
	//cout << "left range " << get_key(0) << ".." << get_key(get_count()-1) << endl;
	//cout << "right range " << sp.get_key(0) << ".." << sp.get_key(sp.get_count()-1) << endl;
	brass_block_t n_left = n, n_right = sp.n;
	//cout << "Left " << n_left << " right " << n_right << endl;
	// We want the block where the new key will go to end up in the
	// cursor, so swap things around if the new key wants to go after
	// the split point.
	if (item > split_at || (item == split_at && key >= divkey)) {
	    AssertRel(key, >=, divkey);
	    // cout << "split : item after" << endl;
	    item -= split_at;
	    swap(data, sp.data);
	    swap(n, sp.n);
	    // Should already be false.
	    Assert(!needs_clone);
	} else {
	    AssertRel(key, <, divkey);
	    // cout << "split : item before" << endl;
	}
	// cout << "me | div | sp\n";
	// cout << get_key(0) << ".." << get_key(get_count()-1) << " | " << divkey << " | " << sp.get_key(0) << ".." << sp.get_key(sp.get_count()-1) << endl;
	sp.save();
	CHECK_BLOCK();
	// And now insert into parent, adding a level to the Btree if required.
	if (!parent)
	    parent = const_cast<BrassTable&>(table).gain_level(n_left);
	// cout << n << ": parent dividing key = [" << divkey << "]" << " n_left = " << n_left << " n_right = " << n_right << endl;
	parent->insert(divkey, n_right, n);
	C = get_count();
	AssertRel(item,<=,C);
	// Check that we actually created enough free space!
	// cout << "Need " << len + 2 << ", now have " << get_endptr(C) - header_length(C) << " free" << endl;
	AssertRel(len + 2,<=,get_endptr(C) - header_length(C));
    }
    int ptr;
    if (exact) {
	// If the replacement item isn't the same size, we need to move
	// other items along.
	if (len != oldlen) {
	    // Need to insert in sorted order, so shuffle existing entries
	    // along.
	    // We need to move entries [item, C) up by oldlen - len bytes (or
	    // down if that's negative).
	    if (item != C - 1) {
		char * q = data + get_ptr(C - 1);
		int l = get_ptr(item) - get_ptr(C - 1);
		AssertRel(l,>=,0);
		memmove(q + oldlen - len, q, l);
		// cout << n << ": replace shuffle! (" << q - len - data << " <- " << q - data << ", " << l << ")" << endl;
	    }
	    for (int i = C; i > item; --i) {
		set_ptr(i - 1, get_ptr(i - 1) + oldlen - len);
	    }
	}
	ptr = get_ptr(item);
    } else {
	ptr = get_endptr(item) - len;
	if (item < C) {
	    // Need to insert in sorted order, so shuffle existing entries
	    // along.
	    // We need to move entries [item, C) down by len bytes.
	    char * q = data + get_ptr(C - 1);
	    int l = get_endptr(item) - get_ptr(C - 1);
	    AssertRel(l,>=,0);
	    memmove(q - len, q, l);
	    // cout << n << ": add shuffle! (" << q - len - data << " <- " << q - data << ", " << l << ")" << endl;
	    for (int i = C; i > item; --i) {
		set_ptr(i, get_ptr(i - 1) - len);
	    }
	}
	set_count(C + 1);
	set_ptr(item, ptr);
    }

    AssertRel(ptr,>=,header_length(get_count()));
    Assert(key.size() < 256);
    char * p = data + ptr;
    *p++ = info;
    memcpy(p, key.data(), key.size());
    CHECK_BLOCK();
    memcpy(p + key.size(), tag, tag_len);

    CHECK_BLOCK();

    modified = true;
}

void
BrassCBlock::del()
{
    LOGCALL_VOID(DB, "BrassCBlock::del", NO_ARGS);
    Assert(!is_leaf());
    // The last entry in a child block was removed, so delete item "item"
    // from this block.
    int C = get_count();
    Assert(item < C);
    Assert(item >= -1);
    if (C <= 1) {
	if (!parent) {
	    // This is the root block, and would only have one child after this
	    // deletion, so the Btree can lose a level (which will delete this
	    // block, so there's no point updating its contents).

	    // It would be pointless to write this block to disk.
	    modified = false;
	    const_cast<BrassTable&>(table).lose_level();
	    return;
	}

	if (C == 0) {
	    // This block will become empty, so delete it and remove the
	    // corresponding entry from its parent.
	    const_cast<BrassTable&>(table).mark_free(n);
	    // It would be pointless to write this block to disk.
	    modified = false;
	    AssertEq(item, -1);
	    item = -2;
	    // We handle parent == NULL just above.
	    Assert(parent);
	    parent->del();
	    return;
	}
    }

    if (item == -1) {
	// If we're wanting to delete the left pointer, copy the first pointer
	// to the left pointer, then delete the first pointer along with its
	// dividing key.
	set_left_block(get_block(0));
	item = 0;
    } else if (item < C - 1) {
	// FIXME: Where is best to leave the dividing key?  We could even
	// recalculate it to see if there's a shorter one...
#if 0
	set_block(item, get_block(item + 1));
	++item;
#endif
    }

    if (item != C - 1) {
	size_t len = get_endptr(item) - get_ptr(item);
	size_t ptr = get_ptr(C - 1);
	memmove(data + ptr + len, data + ptr, get_ptr(item) - ptr);
	for (int i = item + 1; i < C; ++i) {
	    set_ptr(i - 1, get_ptr(i) + len);
	}
    }
    set_count(C - 1);
    item = -2;
    CHECK_BLOCK();
    modified = true;
}

bool
BrassCBlock::del(const string &key)
{
    LOGCALL(DB, bool, "BrassCBlock::del", key);
    Assert(!key.empty());
    // FIXME: needed?
    if (!data)
	return false;

    if (!is_leaf()) {
	find_child(key);
	RETURN(child->del(key));
    }

    bool exact = binary_chop_leaf(key, EQ);
    //cout << n << ": del at " << item << "/" << C << endl;
    if (!exact)
	RETURN(false);

    AssertEq(get_key(item), key);

    int C = get_count();
    if (C <= 2) {
	if (rare(!parent)) {
	    // This is the root block, and it has 2 entries.  One of these must
	    // be the dummy empty entry, so the table will be empty after this
	    // operation.
	    AssertEq(C, 2);
	    AssertEq(item, 1);
	    AssertEq(get_key(0), string());
	    modified = false;
	    const_cast<BrassTable&>(table).lose_level();
	    RETURN(true);
	}
	if (C <= 1) {
	    // Block now empty, so delete it and the parent's pointer to it.
	    modified = false;
	    const_cast<BrassTable&>(table).mark_free(n);
	    AssertEq(item, 0);
	    item = -2;
	    parent->del();
	    RETURN(true);
	}
    }

    if (item != C - 1) {
	size_t len = get_endptr(item) - get_ptr(item);
	size_t ptr = get_ptr(C - 1);
	memmove(data + ptr + len, data + ptr, get_ptr(item) - ptr);
	for (int i = item + 1; i < C; ++i) {
	    set_ptr(i - 1, get_ptr(i) + len);
	}
    }
    set_count(C - 1);
    CHECK_BLOCK();
    modified = true;
    RETURN(true);
}

void
BrassCBlock::cancel()
{
    if (modified) {
	const_cast<BrassTable&>(table).mark_free(n);
	modified = false;
    }
    if (child) child->cancel();
}

// FIXME: factor out repeated code from get() ?
bool
BrassCBlock::find(const string &key, int mode)
{
    LOGCALL(DB, bool, "BrassCBlock::find", key | mode);
    if (!data) {
	// The empty key should always appear to exist.
	if (mode == LE || (key.empty() ^ (mode == LT))) {
	    item = -1;
	    RETURN(key.empty());
	}
	item = -2;
	RETURN(false);
    }

    if (!is_leaf()) {
	find_child(key);
	bool result = child->find(key, mode);
	if (child->item == -2)
	    item = -2;
	RETURN(result);
    }

    CHECK_BLOCK();

    bool exact = binary_chop_leaf(key, mode);
    if (!exact) {
	if (mode == GE) {
	    if (item == get_count()) {
		--item;
		next_();
	    } else if (item >= 0)
		AssertRel(get_key(item),>=,key);
	} else {
	    prev_();
	    if (item >= 0 && item < get_count()) {
		if (mode == LT) {
		    AssertRel(get_key(item),<,key);
		} else if (mode == LE) {
		    AssertRel(get_key(item),<=,key);
		}
	    }
	}
	RETURN(false);
    }

    // Exact match must be in range.
    AssertRel(item,<,get_count());

    if (mode != LT) {
	if (item >= 0) {
	    if (mode == LE) {
		AssertRel(get_key(item),<=,key);
	    } else {
		AssertRel(get_key(item),>=,key);
	    }
	}
	RETURN(true);
    }

    // Return value is ignored, but this allows tail recursion.
    // FIXME: RETURN(prev_());
    prev_();
//    cout << "find(LT) -> " << item << endl;
    AssertRel(get_key(item),<,key);
    return false;
}

bool
BrassCBlock::key_exists(const string &key)
{
    LOGCALL(DB, bool, "BrassCBlock::key_exists", key);
    Assert(!key.empty());
    // FIXME: check we really want this...
    if (!data)
	RETURN(false);

    if (!is_leaf()) {
	find_child(key);
	RETURN(child->key_exists(key));
    }

    RETURN(binary_chop_leaf(key, EQ));
}

bool
BrassCBlock::get(const string &key, string &tag)
{
    LOGCALL(DB, bool, "BrassCBlock::get", key | tag);
    Assert(!key.empty());
    // FIXME: check we really want this...
    if (!data)
	RETURN(false);

    if (!is_leaf()) {
	find_child(key);
	RETURN(child->get(key, tag));
    }

    if (!binary_chop_leaf(key, EQ))
	RETURN(false);

    RETURN(read_tag(tag));
}

void
BrassCBlock::check()
{
    LOGCALL_VOID(DB, "BrassCBlock::check", NO_ARGS);
    check_block();
    if (is_leaf())
	return;

    if (!child)
	child = new BrassCBlock(table, this);

    // Recursively check descendent blocks.
    int C = get_count();
    for (item = -1; item < C; ++item) {
	child->read(get_block(item));
	child->check();
    }
}

BrassCursor *
BrassTable::get_cursor() const
{
    LOGCALL(DB, BrassCursor *, "BrassTable::get_cursor", NO_ARGS);
    if (rare(!is_open()))
	RETURN(NULL);
    RETURN(new BrassCursor(*this));
}

// Prefer pread if available since (assuming it's implemented as a separate
// syscall) it eliminates the overhead of an extra syscall per block read.
bool
BrassTable::read_block(char *buf, brass_block_t n) const
{
    // FIXME: factor out shared code with read_slab()?
    LOGCALL(DB, bool, "BrassTable::read_block", (void*)buf | n);
    Assert(fd != FD_NOT_OPEN);
    Assert(buf);
    Assert(n != (brass_block_t)-1);
    ssize_t count = blocksize;
#ifndef HAVE_PREAD
    if (lseek(fd, off_t(n) * blocksize, SEEK_SET) == -1)
	RETURN(false);
#else
    off_t offset = off_t(n) * blocksize;
#endif
    while (true) {
#ifndef HAVE_PREAD
	ssize_t res = read(fd, buf, count);
#else
	ssize_t res = pread(fd, buf, count, offset);
#endif
	// We should get a full read most of the time, so streamline that case.
	if (res == count)
	    RETURN(true);
	// -1 is error, 0 is EOF
	if (res <= 0) {
	    // We get EINTR if the syscall was interrupted by a signal.
	    // In this case we should retry the read.
	    if (res == -1 && errno == EINTR)
		continue;
	    if (res == 0)
		errno = 0;
	    RETURN(false);
	}
	buf += res;
#ifdef HAVE_PREAD
	offset += res;
#endif
	count -= res;
    }
}

bool
BrassTable::write_block(const char *buf, brass_block_t n) const
{
    // FIXME: factor out shared code with write_slab()?
    LOGCALL(DB, bool, "BrassTable::write_block", (void*)buf | n);
    AssertRel(fd,>=,0);
    Assert(buf);
    Assert(n != (brass_block_t)-1);

    ssize_t count = blocksize;
    // Prefer pwrite() if available since (assuming it's implemented as a
    // separate syscall) it eliminates the overhead of an extra lseek() syscall
    // per block write.
#ifndef HAVE_PWRITE
    if (lseek(fd, off_t(n) * blocksize, SEEK_SET) == -1)
	RETURN(false);
#else
    off_t offset = off_t(n) * blocksize;
#endif
    while (true) {
#ifndef HAVE_PWRITE
	ssize_t res = write(fd, buf, count);
#else
	ssize_t res = pwrite(fd, buf, count, offset);
#endif
	// We should get a full write most of the time, so streamline that case.
	if (res == count)
	    RETURN(true);
	if (res < 0) {
	    // We get EINTR if the syscall was interrupted by a signal.
	    // In this case we should retry the write.
	    if (res == -1 && errno == EINTR)
		continue;
	    RETURN(false);
	}
	buf += res;
#ifdef HAVE_PWRITE
	offset += res;
#endif
	count -= res;
    }
}

// Prefer pread if available since (assuming it's implemented as a separate
// syscall) it eliminates the overhead of an extra syscall per read.
bool
BrassTable::read_slab(char *buf, size_t len, off_t offset) const
{
    LOGCALL(DB, bool, "BrassTable::read_slab", (void*)buf | len | offset);
    Assert(fd_slab != FD_NOT_OPEN);
    Assert(buf);
    ssize_t count = len;
#ifndef HAVE_PREAD
    if (lseek(fd_slab, offset, SEEK_SET) == -1)
	RETURN(false);
#endif
    while (true) {
#ifndef HAVE_PREAD
	ssize_t res = read(fd_slab, buf, count);
#else
	ssize_t res = pread(fd_slab, buf, count, offset);
#endif
	// We should get a full read most of the time, so streamline that case.
	if (res == count)
	    RETURN(true);
	// -1 is error, 0 is EOF
	if (res <= 0) {
	    // We get EINTR if the syscall was interrupted by a signal.
	    // In this case we should retry the read.
	    if (res == -1 && errno == EINTR)
		continue;
	    if (res == 0)
		errno = 0;
	    RETURN(false);
	}
	buf += res;
#ifdef HAVE_PREAD
	offset += res;
#endif
	count -= res;
    }
}

bool
BrassTable::write_slab(const char *buf, size_t len, off_t offset) const
{
    LOGCALL(DB, bool, "BrassTable::write_slab", (void*)buf | len | offset);
    AssertRel(fd_slab,>=,0);
    Assert(buf);

    ssize_t count = len;
    // Prefer pwrite() if available since (assuming it's implemented as a
    // separate syscall) it eliminates the overhead of an extra lseek() syscall
    // per write.
#ifndef HAVE_PWRITE
    if (lseek(fd_slab, offset, SEEK_SET) == -1)
	RETURN(false);
#endif
    while (true) {
#ifndef HAVE_PWRITE
	ssize_t res = write(fd_slab, buf, count);
#else
	ssize_t res = pwrite(fd_slab, buf, count, offset);
#endif
	// We should get a full write most of the time, so streamline that case.
	if (res == count)
	    RETURN(true);
	if (res < 0) {
	    // We get EINTR if the syscall was interrupted by a signal.
	    // In this case we should retry the write.
	    if (res == -1 && errno == EINTR)
		continue;
	    RETURN(false);
	}
	buf += res;
#ifdef HAVE_PWRITE
	offset += res;
#endif
	count -= res;
    }
}

void
BrassTable::lose_level()
{
    LOGCALL_VOID(DB, "BrassTable::lose_level", NO_ARGS);
    Assert(my_cursor);
    do {
	mark_free(my_cursor->n);
	my_cursor = my_cursor->lose_level();
	if (rare(!my_cursor))
	    return;
    } while (!my_cursor->is_leaf() && my_cursor->get_count() == 0);
    if (my_cursor->is_leaf() && my_cursor->get_count() <= 1) {
	delete my_cursor;
	my_cursor = NULL;
    }
}

brass_block_t
BrassTable::get_free_block()
{
    // FIXME: crude implementation!
    if (next_free == 0) {
	struct stat statbuf;
	if (fstat(fd, &statbuf) < 0)
	    throw Xapian::DatabaseError("Couldn't stat table", errno);
	next_free = statbuf.st_size / blocksize;
	// cout << "next_free starts as " << next_free << endl;
    }
    return next_free++;
}

off_t
BrassTable::get_free_slab(size_t size)
{
    LOGCALL(DB, off_t, "BrassTable::get_free_slab", size);
    if (next_free_slab == 0) {
	struct stat statbuf;
	if (fstat(fd_slab, &statbuf) < 0)
	    throw Xapian::DatabaseError("Couldn't stat table", errno);
	next_free_slab = statbuf.st_size;
    }
    off_t res = next_free_slab;
    next_free_slab += size;
    return res;
}

int
BrassTable::compare_keys(const void *k1, size_t l1, const void *k2, size_t l2) const
{
    LOGCALL(DB, int, "BrassTable::compare_keys", k1 | l1 | k2 | l2);
    int result = memcmp(k1, k2, std::min(l1, l2));
    if (result)
	RETURN(result);
    RETURN((int)l1 - (int)l2);
}

string
BrassTable::divide(const char *k1, size_t l1, const char *k2, size_t l2) const
{
    LOGCALL(DB, string, "BrassTable::divide", k1 | l1 | k2 | l2);
    (void)l2;
    AssertRel(compare_keys(k1, l1, k2, l2),<,0);
    size_t i;
    for (i = 0; i < l1; ++i) {
	// If key1 < key2, it should be impossible for us to reach here with
	// i >= l2.
	AssertRel(i,<,l2);
	if (k1[i] != k2[i])
	    break;
    }
    RETURN(string(k2, i + 1));
}

BrassTable::~BrassTable()
{
    LOGCALL_DTOR(DB, "BrassTable");
    BrassTable::close();
}

bool
BrassTable::exists() const
{
    LOGCALL(DB, bool, "BrassTable::exists", NO_ARGS);
    RETURN(file_exists(path + BRASS_TABLE_EXTENSION));
}

bool
BrassTable::create(unsigned int blocksize_, bool from_scratch)
{
    LOGCALL(DB, bool, "BrassTable::create", blocksize_ | from_scratch);
    (void)from_scratch;
    Assert(!readonly);
    AssertRel(fd,==,FD_NOT_OPEN);

    // Check that blocksize is a power of two in the permitted range and
    // if not just use the default.
    if (blocksize_ < 2048 || blocksize_ > 65536 ||
	(blocksize_ & (blocksize_ - 1)) != 0) {
	blocksize_ = BRASS_DEFAULT_BLOCKSIZE;
    }
    blocksize = blocksize_;

    fd = ::open((path + BRASS_TABLE_EXTENSION).c_str(),
		O_RDWR|O_CREAT|O_TRUNC|O_BINARY, 0666);
    if (fd == FD_NOT_OPEN) {
	errmsg = "Failed to create '";
	errmsg += path;
	errmsg += BRASS_TABLE_EXTENSION"': ";
	errmsg += strerror(errno);
	// errcode = DATABASE_OPENING_ERROR;
	RETURN(false);
    }

    // Open the slab file lazily - we don't need it for a table which only
    // holds small items.
    fd_slab = FD_NOT_OPEN;

    modified = false;
    delete my_cursor;
    my_cursor = NULL;
    revision = 0;
    RETURN(true);
}

void
BrassTable::erase()
{
    LOGCALL_VOID(DB, "BrassTable::erase", NO_ARGS);
    close();
    fd = FD_NOT_OPEN;
    // FIXME: handle error (ENOENT is ok)
    unlink(path + BRASS_TABLE_EXTENSION);
    unlink(path + BRASS_SLAB_EXTENSION);
}

bool
BrassTable::open(unsigned int blocksize_, brass_block_t root)
{
    LOGCALL(DB, bool, "BrassTable::open", blocksize_ | root);
    blocksize = blocksize_;
    if (!blocksize)
	blocksize = BRASS_DEFAULT_BLOCKSIZE;
    if (fd == FD_NOT_OPEN) {
	// Table not already open.
	fd_slab = FD_NOT_OPEN;
	if (readonly) {
	    fd = ::open((path + BRASS_TABLE_EXTENSION).c_str(),
			O_RDONLY|O_BINARY);
	    if (fd == FD_NOT_OPEN) {
		errmsg = "Failed to open '";
		errmsg += path;
		errmsg += "' for reading: ";
		errmsg += strerror(errno);
		// errcode = DATABASE_OPENING_ERROR;
		RETURN(false);
	    }
	} else {
	    fd = ::open((path + BRASS_TABLE_EXTENSION).c_str(),
			O_RDWR|O_BINARY);
	    if (fd == FD_NOT_OPEN) {
		errmsg = "Failed to open '";
		errmsg += path;
		errmsg += "' for read/write: ";
		errmsg += strerror(errno);
		// errcode = DATABASE_OPENING_ERROR;
		RETURN(false);
	    }
	}
	modified = false;
    } else {
	if (rare(fd == FD_CLOSED))
	    throw_database_closed();
	// FIXME: can we reuse the existing cursor?  The number of levels might
	// have changed, and the blocks may be totally wrong.
	delete my_cursor;
	my_cursor = NULL;
    }

    if (root == brass_block_t(-1)) {
	revision = 0;
	RETURN(true);
    }

    my_cursor = new BrassCBlock(*this);
    my_cursor->read(root);
    // Read revision from root block.
    revision = my_cursor->get_revision();
    // Revision number to use for new blocks (resulting from splitting and
    // update).
    ++revision;
    my_cursor->set_needs_clone();

    RETURN(true);
}

void
BrassTable::open_slab_file()
{
    LOGCALL_VOID(DB, "BrassTable::open_slab_file", NO_ARGS);
    next_free_slab = 0;
    string filename = path;
    filename += BRASS_SLAB_EXTENSION;
    int mode = (readonly ? O_RDONLY|O_BINARY : O_CREAT|O_RDWR|O_BINARY);
    fd_slab = ::open(filename.c_str(), mode, 0666);
    if (rare(fd_slab == FD_NOT_OPEN)) {
	string msg = "Failed to open '";
	msg += filename;
	msg += (readonly ? "' for reading" : "' for read/write");
	throw Xapian::DatabaseError(msg, errno);
    }
}

void
BrassTable::close()
{
    LOGCALL_VOID(DB, "BrassTable::close", NO_ARGS);
    if (fd >= 0) {
	delete my_cursor;
	my_cursor = NULL;
	// FIXME: check for error...
	::close(fd);
	if (fd_slab >= 0)
	    ::close(fd_slab);
    }
    fd = fd_slab = FD_CLOSED;
}

void
BrassTable::add(const string & key, const string & tag, bool already_compressed)
{
    LOGCALL_VOID(DB, "BrassTable::add", key | tag | already_compressed);
    if (key.size() == 0 || key.size() > 255)
	throw Xapian::InvalidArgumentError("Key length must be non-zero and at most 255 bytes");

    if (fd < 0) {
	if (fd == FD_CLOSED)
	    throw_database_closed();
	create(blocksize, true);
    }
    if (rare(!my_cursor)) {
	my_cursor = new BrassCBlock(*this);
	my_cursor->new_leaf_block();
	my_cursor->insert(string(), NULL, 0, false);
    }
    modified = true;

    if (compress && !already_compressed) {
	if (compressor.try_to_compress(tag)) {
	    const char * p = compressor.compressed_data();
	    size_t len = compressor.compressed_data_len();
	    my_cursor->insert(key, p, len, true);
	    return;
	}

	// Data wasn't compressible, so store uncompressed.
    }
    my_cursor->insert(key, tag.data(), tag.size(), already_compressed);
}

bool
BrassTable::del(const string & key)
{
    LOGCALL(DB, bool, "BrassTable::del", key);
    if (key.empty())
	RETURN(false);
    if (fd == FD_CLOSED)
	throw_database_closed();
    // Can't delete if no entries!
    if (!my_cursor)
	RETURN(false);
    if (my_cursor->del(key)) {
	modified = true;
	RETURN(true);
    }
    RETURN(false);
}

brass_block_t
BrassTable::commit(brass_revision_number_t revision_)
{
    LOGCALL(DB, brass_block_t, "BrassBlock::commit", revision_);
    AssertRel(revision_,>=,revision);
    if (fd < 0 || !my_cursor) {
	if (fd == FD_CLOSED)
	    throw_database_closed();
	revision = revision_;
	RETURN(brass_block_t(-1));
    }
    if (modified) {
	revision = revision_;
	my_cursor->commit();
	modified = false;
    }
    RETURN(get_root());
}

void
BrassTable::cancel()
{
    LOGCALL_VOID(DB, "BrassTable::cancel", NO_ARGS);
    if (!modified)
	return;
    if (fd < 0 || !my_cursor) {
	if (fd == FD_CLOSED)
	    throw_database_closed();
	return;
    }
    my_cursor->cancel();
    modified = false;
}

bool
BrassTable::key_exists(const string & key) const
{
    LOGCALL(DB, bool, "BrassTable::key_exists", key);
    if (key.empty())
	RETURN(false);
    if (fd == FD_CLOSED)
	throw_database_closed();
    // Can't read if no entries!
    if (!my_cursor)
	RETURN(false);
    RETURN(my_cursor->key_exists(key));
}

bool
BrassTable::get(const string & key, string & tag) const
{
    LOGCALL(DB, bool, "BrassTable::get", key | tag);
    if (rare(key.empty())) {
	tag.resize(0);
	RETURN(true);
    }
    if (fd == FD_CLOSED)
	throw_database_closed();
    // Can't read if no entries!
    if (!my_cursor)
	RETURN(false);
    RETURN(my_cursor->get(key, tag));
}

void
BrassTable::check()
{
    LOGCALL_VOID(DB, "BrassTable::check", NO_ARGS);
    if (my_cursor)
	my_cursor->check();
}
