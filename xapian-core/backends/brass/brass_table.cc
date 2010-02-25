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
 *  + Track free blocks!
 *  + Long tags stored externally.
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

#define BRASS_DEFAULT_BLOCKSIZE 8192

// FIXME: benchmark if it is better/worse/no different to put items at end
// and pointers at start.  It is likely to be less confusing to follow the
// code for if nothing else!

/*
  [ | | | ] 4 bytes: Revision
  [ | ]     2 bytes: bits 0-14 #items in this block (n)
		     bit 15: 1 for non-leaf block
  [ ]       1 byte:  length of key prefix (unimplemented)
  [ ]       1 byte:  reserved

  [ | ] offset to start of item 0
   ...
  [ | ] offset to start of item n-1

   [Free space]

  [ Item n - 1 ]
   ...
  [ Item 0 ]
  [ Left-most block pointer ] ( non-leaf blocks only )
  <end of block>
 */

/* Leaf Item:
 *
 *    (a) [ keylen ] (1 byte) : top bit set is "compressed tag?"
 * or (b) [ \x80 ]
 *        [ keylen - 128 ] : top bit set is "compressed tag?"
 * [ key ... ]
 * [ tag ... ]
 */

/* Branch Item:
 *
 * [ block pointer ] (4 bytes)
 * [ key ... ]
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
BrassBlock::check_block()
{
#ifdef XAPIAN_ASSERTIONS
    if (uncaught_exception())
	return;
    Assert(data);
    int C = get_count();
    if (C == 0)
	return;
    // string prev_key = table.key_limits[n].first;
    string prev_key;
    string key;
    int header_len = header_length(C);
    for (int i = 0; i < C; ++i) {
	int ptr = get_ptr(i);
	int endptr = get_endptr(i);
	// Pointers shouldn't be into the block header.
	AssertRel(ptr,>=,header_len);
	// Items should have non-negative size.
	AssertRel(ptr,<,endptr);
	if (is_leaf()) {
	    size_t key_len = (unsigned char)data[ptr];
	    if (key_len & 0x80) {
		if (key_len == 0x80) {
		    key_len = static_cast<unsigned char>(data[ptr++]);
		    key_len |= 0x80;
		} else {
		    key_len &= 0x7f;
		}
	    }
	    AssertRel(ptr + key_len,<=,(size_t)endptr);
	    key.assign(data + ptr, key_len);
	    int tag_len = endptr - ptr - key_len;
	    (void)tag_len;
	    AssertRel(tag_len,<=,16384);
	} else {
	    AssertRel(ptr + 4,<=,endptr);
	    AssertRel(endptr - ptr,<,256 + 4);
	    key.assign(data + ptr + 4, endptr - (ptr + 4));
	}
	if (i == 0) {
	    AssertRel(prev_key,<=,key);
	} else {
	    AssertRel(prev_key,<,key);
	}
	// FIXME: More checks...

	swap(prev_key, key);
    }
    // AssertRel(prev_key,<=,table.key_limits[n].second);
    // The last item shouldn't overlap the block header.
    AssertRel(get_ptr(C - 1),>=,header_len);
    // FIXME: More checks...
#endif
}

void
BrassBlock::save()
{
    LOGCALL_VOID(DB, "BrassBlock::save", NO_ARGS);
    if (!data)
	return;
    set_revision(table.revision);
    check_block();
    if (!table.write_block(data, n))
	throw Xapian::DatabaseError("Failed to write block " + str(n), errno);
}

BrassCBlock::~BrassCBlock()
{
    LOGCALL_DTOR(DB, "BrassCBlock");
    if (child)
	delete child;
    if (modified)
	save();
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
    Assert(is_leaf());
}

void
BrassBlock::new_branch_block()
{
    LOGCALL_VOID(DB, "BrassBlock::new_branch_block", NO_ARGS);
    if (!data)
	data = new char[table.blocksize];
    n = const_cast<BrassTable&>(table).get_free_block();
    // Create empty branch block.
    memset(data, 0, table.blocksize);
    // Set the branch block flag.
    data[5] = '\x80';
    Assert(!is_leaf());
}

bool
BrassCBlock::read_tag(string &tag)
{
    AssertRel(item,>=,-1);
    AssertRel(item,<,get_count());
    if (!is_leaf())
	return child->read_tag(tag);

    bool compressed = false;
    int key_start = get_ptr(item);
    int key_len = (unsigned char)data[key_start++];
    if (key_len & 0x80) {
	if (key_len == 0x80) {
	    key_len = (unsigned char)data[key_start++];
	    compressed = (key_len >= 0x80);
	    key_len |= 0x80;
	} else {
	    compressed = true;
	    key_len &= 0x7f;
	}
    }
    const char * s = data + key_start + key_len;
    AssertRel(s - data,<=,get_endptr(item));
    if (!compressed) {
	tag.assign(s, data + get_endptr(item) - s);
    } else {
	table.compressor.decompress(s, data + get_endptr(item) - s, tag);
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
    check_block();
    modified = false;
    // If the block's revision is the one we are currently working on, then it
    // must have been allocated freshly for this revision, and so we don't need
    // to clone it.
    needs_clone = (get_revision() != table.revision);
    item = -2;
    random_access = RANDOM_ACCESS_THRESHOLD;
    if (is_leaf()) {
	Assert(child == NULL);
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
    LOGCALL(DB, bool, "BrassCBlock::binary_chop_leaf", key << ", " << mode);
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
	const char * key_data = data + get_ptr(m);
	int key_len = static_cast<unsigned char>(*key_data++);
	if (key_len & 0x80) {
	    if (key_len == 0x80) {
		key_len = static_cast<unsigned char>(*key_data++);
		key_len |= 0x80;
	    } else {
		key_len &= 0x7f;
	    }
	}
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
	const char * key_data = data + get_ptr(m);
	int key_len = static_cast<unsigned char>(*key_data++);
	if (key_len & 0x80) {
	    if (key_len == 0x80) {
		key_len = static_cast<unsigned char>(*key_data++);
		key_len |= 0x80;
	    } else {
		key_len &= 0x7f;
	    }
	}
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
    RETURN(false);
}

void
BrassCBlock::insert(const string &key, brass_block_t tag)
{
    LOGCALL_VOID(DB, "BrassCBlock::insert", key << ", " << tag);
    // cerr << table.key_limits[n].first << "<=" << key << endl;
    // AssertRel(table.key_limits[n].first,<=,key);
    // cerr << table.key_limits[n].second << ">=" << key << endl;
    // AssertRel(table.key_limits[n].second,>=,key);
    Assert(!is_leaf());

    // NB Don't update item to point to the new block, since we're called
    // precisely when splitting a child block and it's important that item
    // points to the child block.

    if (get_count() == 0) {
	// FIXME: special case probably not needed, but perhaps could be
	// merged into gain_level()...
	set_count(1);
	int new_ptr = table.blocksize - BLOCKPTR_SIZE - BLOCKPTR_SIZE - key.size();
	set_ptr(0, new_ptr);
	set_unaligned_le4(data + new_ptr, tag);
	memcpy(data + new_ptr + BLOCKPTR_SIZE, key.data(), key.size());
	modified = true;
	check_block();
	return;
    }

    int b = 0;
    {
	int e = get_count() - 1;
	while (b <= e) {
	    int m = (b + e) >> 1;
	    int key_start = get_ptr(m) + BLOCKPTR_SIZE;
	    int key_len = get_endptr(m) - key_start;
	    int cmp = table.compare_keys(data + key_start, key_len,
					 key.data(), key.size());
	    if (cmp < 0) {
		b = m + 1;
	    } else {
		e = m - 1;
		// Shouldn't get an exact match.
		Assert(cmp != 0);
	    }
	}
	b = e + 1;
    }

    AssertRel(b,<=,get_count());
    if (b < get_count()) {
	AssertRel(get_key(b),>,key);
    }
    if (b > 0) {
	AssertRel(get_key(b - 1),<,key);
    }

    int len = BLOCKPTR_SIZE + key.size();
    int C = get_count();
    //cout << n << ": insert at " << b << "/" << C << endl;
    int freespace_end = get_endptr(C);
    // Check it fits!
    //cout << n << ": " << len + 2 << " <= " << freespace_end - header_length(C) << endl;
    // FIXME: see comment in other version of insert().
    if (len + 2 > freespace_end - header_length(C)) {
	// cout << "splitting branch block " << n << endl;
	AssertRel(C,>,1);
	// Split the block - if in random insert mode, split evenly as
	// that gives us the amortised Btree performance guarantees.
	//
	// If in sequential insert mode, then split at the insertion point
	// which gives us almost full blocks.
	int split_after;
	if (!random_access) {
	    split_after = b - 1;
	} else {
	    // FIXME: pick middle point better...
	    split_after = C >> 1;
	}
	size_t split_ptr = get_endptr(split_after);
	const char *div_p = data + get_ptr(split_after);
	// We can't further shorten the dividing key here or it'll disagree
	// with the existing partitioning of keys at the leaf level.
	string divkey(div_p + BLOCKPTR_SIZE, split_ptr - get_ptr(split_after) - BLOCKPTR_SIZE);
	brass_block_t split_new_l = get_unaligned_le4(div_p);
	++split_after;
	split_ptr = get_endptr(split_after);
	//cout << n << ": " << divkey << " <=> " << key << endl;
	// We put the right half of the split block in the new block, and
	// then swap the blocks around if we want to insert into the right
	// half since that gives code that is simpler and probably slightly
	// more efficient.  It also avoids needing to adjust the pointer
	// in the parent.
	BrassBlock & sp = const_cast<BrassTable&>(table).split;
	sp.new_branch_block();
	// cout << sp.n << " SETTING BOUNDS from split " << n << endl;
	// table.key_limits[sp.n].first = divkey;
	// table.key_limits[sp.n].second = table.key_limits[n].second;
	// cout << n << " UPDATING BOUNDS from split " << endl;
	// table.key_limits[n].second = divkey;
	memcpy(sp.data + table.blocksize - BLOCKPTR_SIZE - (split_ptr - freespace_end), data + freespace_end, split_ptr - freespace_end);
	sp.set_left_block(split_new_l);
	for (int i = split_after; i < C; ++i) {
	    sp.set_ptr(i - split_after, get_ptr(i) + table.blocksize - BLOCKPTR_SIZE - split_ptr);
	}
	sp.set_count(C - split_after);
#ifdef ZERO_UNUSED_SPACE
	memset(data + freespace_end, 0, split_ptr - freespace_end);
#endif
	set_count(split_after - 1);
	brass_block_t n_left = n, n_right = sp.n;
	if (b > split_after) {
	    b -= split_after;
	    item -= split_after;
	    swap(data, sp.data);
	    swap(n, sp.n);
	    // Should already be false.
	    Assert(!needs_clone);
	}
	sp.save();
	check_block();
	// And now insert into parent, adding a level to the Btree if
	// required.
	if (!parent)
	    parent = const_cast<BrassTable&>(table).gain_level(n_left);
	parent->insert(divkey, n_right);
	// We have a new sibling, but we want the attention.
	// FIXME: this is clumsy...
	if (parent->get_block(parent->item) != n) {
	    ++parent->item;
	    AssertEq(parent->get_block(parent->item), n);
	}
	C = get_count();
	// Check that we actually created enough free space!
	// cout << "Need " << len + 2 << ", now have " <<
	//    get_endptr(C) - header_length(C) << " free" << endl;
	AssertRel(len + 2,<=,get_endptr(C) - header_length(C));
    }
    if (b < C) {
	// Need to insert in sorted order, so shuffle existing entries along.
	// We need to move entries [b, C) down by len bytes.
	char * q = data + freespace_end;
	size_t l = get_endptr(b) - freespace_end;
	memmove(q - len, q, l);
	//cout << n << ": shuffle! (" << q - len - data << " <- " << q - data << ", " << l << ")" << endl;
	/* item can be -1 (left pointer), and we don't want to adjust that. */
	if (item >= b)
	    ++item;
	for (int i = C; i > b; --i) {
	    set_ptr(i, get_ptr(i - 1) - len);
	}
    }
    int new_ptr = get_endptr(b) - len;
    set_count(C + 1);
    set_ptr(b, new_ptr);
    //cout << n << ": setting item " << b << " to key " << key << " and block " << tag << endl;
    set_unaligned_le4(data + new_ptr, tag);
    memcpy(data + new_ptr + BLOCKPTR_SIZE, key.data(), key.size());
    modified = true;
    check_block();
}

void
BrassCBlock::find_child(const string & key)
{
    LOGCALL_VOID(DB, "BrassCBlock::find_child", key);
    Assert(!is_leaf());
    int b = 0;
    int e = get_count() - 1;
    Assert(e != -1);

    // An access will often be just after the previous one, so if item is set,
    // use it to pick the first chop point.
    int m = (b + e) >> 1;
    if (item >= m) {
	if (item > e)
	    item = e;
	m = item;
    } else if (item >= -1) {
	// We expect the point we want is just after item, but chopping at item
	// will less than halve the range, so chop just after it instead.
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

bool
BrassCBlock::insert(const string &key, const char * tag, size_t tag_len,
		    bool compressed)
{
    LOGCALL(DB, bool, "BrassCBlock::insert", key << ", " << (void*)tag << ", " << tag_len << ", " << compressed);
    if (!is_leaf()) {
	find_child(key);
	RETURN(child->insert(key, tag, tag_len, compressed));
    }

    if (key.size() > 252) {
	// FIXME: Make limit 252 for now for compatibility with chert.
	// However, we can support 255 byte keys just as easily.
	throw Xapian::InvalidArgumentError("Max key length is 252 bytes");
    }

    // AssertRel(table.key_limits[n].first,<=,key);
    // cerr << table.key_limits[n].first << "<=" << key << endl;
    // AssertRel(table.key_limits[n].second,>=,key);
    // cerr << table.key_limits[n].second << ">=" << key << endl;

    // FIXME: support this!
    if (tag_len > 16384) {
	throw Xapian::UnimplementedError("Tags > 16K not currently supported");
    }

    int C = get_count();

    if (C > 0) {
	check_block();
    }

    // cout << "block " << n << " needs_clone " << needs_clone << endl;
    if (needs_clone) {
	needs_clone = false;
	//int n_orig = n;
	n = const_cast<BrassTable&>(table).get_free_block();
	// cout << n << " COPYING BOUNDS case 2 from  " << n_orig << endl;
	// table.key_limits[n] = table.key_limits[n_orig];
	// cout << "cloned to " << n << endl;
	if (parent)
	    parent->set_child_block_number(n);
	// cout << "post clone check" << endl;
	check_block();
	// cout << "done post clone check" << endl;
    } else {
	check_block();
    }

    bool exact = binary_chop_leaf(key, LE);
    // cout << n << ": insert (exact=" << exact << ") at " << item << "/" << C << endl;

    int len = tag_len + key.size() + 1;
    if (key.size() >= 128)
	++len;
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
	// FIXME: split_after is "after" in the sense of the order of entries
	// in the block, but "before" in item numbering order.  This is already
	// confusing me!
	int split_ptr;
	int split_after;
	if (!random_access) {
	    split_after = item - 1;
	    split_ptr = get_endptr(split_after);
	} else {
	    split_after = C >> 1;
	    do {
		split_ptr = get_endptr(split_after);
		int percent = (table.blocksize - split_ptr) * 100;
		percent /= (table.blocksize - free_end);
		if (percent > 75) {
		    // cout << "split @ " << percent << "% " << split_after<<"/"<<C<<" too late" << endl;
		    --split_after;
		    AssertRel(split_after,>,0);
		    continue;
		}
		if (percent < 25) {
		    // cout << "split @ " << percent << "% " << split_after<<"/"<<C<<" too early" << endl;
		    ++split_after;
		    AssertRel(split_after,<,C - 1);
		    continue;
		}
		// cout << "split @ " << percent << "% " << split_after<<"/"<<C<<" acceptable" << endl;
		break;
	    } while (true);
	}
	// cerr << "split : item " << item << " split point " << split_after << " out of " << C << endl;
	const char *div_p = data + get_ptr(split_after);
	size_t div_len = static_cast<unsigned char>(*div_p++);
	if (div_len == 0x80) {
	    div_len = static_cast<unsigned char>(*div_p++);
	    div_len |= 0x80;
	} else {
	    div_len &= 0x7f;
	}

	AssertRel(split_ptr,==,get_ptr(split_after - 1));
	const char *pre_p = data + split_ptr; // data + get_ptr(split_after - 1);
	size_t pre_len = static_cast<unsigned char>(*pre_p++);
	if (pre_len == 0x80) {
	    pre_len = static_cast<unsigned char>(*pre_p++);
	    pre_len |= 0x80;
	} else {
	    pre_len &= 0x7f;
	}
	string divkey(table.divide(pre_p, pre_len, div_p, div_len));
	// cout << "splitting leaf block " << n << " at " << divkey << endl;
	// We want the right half of the split block to go in the new block so
	// that if the whole tree is created in one revision, we can do
	// next/previous leaf block by simply moving forwards/backwards in
	// block number order skipping non-leaf blocks.
	BrassBlock & sp = const_cast<BrassTable&>(table).split;
	sp.new_leaf_block();
	// cout << sp.n << " SETTING BOUNDS leaf case from split " << n << endl;
	// table.key_limits[sp.n].first = divkey;
	// table.key_limits[sp.n].second = table.key_limits[n].second;
	// cout << n << " UPDATING BOUNDS leaf case from split " << endl;
	// table.key_limits[n].second = divkey;
	memcpy(sp.data + table.blocksize - (split_ptr - free_end), data + free_end, split_ptr - free_end);
#ifdef ZERO_UNUSED_SPACE
	memset(data + free_end, 0, split_ptr - free_end);
#endif
	for (int i = split_after; i < C; ++i) {
	    sp.set_ptr(i - split_after, get_ptr(i) + (table.blocksize - split_ptr));
	}
	sp.set_count(C - split_after);
	set_count(split_after);
	//cout << "left range " << get_key(0) << ".." << get_key(get_count()-1) << endl;
	//cout << "right range " << sp.get_key(0) << ".." << sp.get_key(sp.get_count()-1) << endl;
	brass_block_t n_left = n, n_right = sp.n;
	//cout << "Left " << n_left << " right " << n_right << endl;
	// We want the block where the new key will go to end up in the
	// cursor, so swap things around if the new key wants to go after
	// the split point.
	if (item >= split_after) {
	    // cout << "split : item after" << endl;
	    item -= split_after;
	    swap(data, sp.data);
	    swap(n, sp.n);
	    // Should already be false.
	    Assert(!needs_clone);
	} else {
	    // cout << "split : item before" << endl;
	}
	// cout << "me | div | sp\n";
	// cout << get_key(0) << ".." << get_key(get_count()-1) << " | " << divkey << " | " << sp.get_key(0) << ".." << sp.get_key(sp.get_count()-1) << endl;
	sp.save();
	sp.check_block();
	check_block();
	// And now insert into parent, adding a level to the Btree if required.
	if (!parent)
	    parent = const_cast<BrassTable&>(table).gain_level(n_left);
	//cout << n << ": parent dividing key = [" << divkey << "]" << " n_left = " << n_left << " n_right = " << n_right << endl;
	parent->insert(divkey, n_right);
	// We have a new sibling, but we want the attention.
	// FIXME: this is clumsy...
	// cerr << "parent item is " << parent->item << ", n is " << n << endl;
	if (parent->get_block(parent->item) != n) {
	    ++parent->item;
	    AssertEq(parent->get_block(parent->item), n);
	}
	C = get_count();
	// Check that we actually created enough free space!
	// cout << "Need " << len + 2 << ", now have " <<
	//    get_endptr(C) - header_length(C) << " free" << endl;
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
	    }
	    // cout << n << ": replace shuffle! (" << q - len - data << " <- " << q - data << ", " << l << ")" << endl;
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
	    // cerr << n << ": add shuffle! (" << q - len - data << " <- " << q - data << ", " << l << ")" << endl;
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
    if (key.size() < 128) {
	unsigned char ch = key.size();
	if (compressed) ch |= 0x80;
	Assert(ch != 0x80); // the zero length key has no tag, so shouldn't compress.
	*p++ = ch;
    } else {
	*p++ = '\x80';
	unsigned char ch = key.size() - 128;
	if (compressed) ch |= 0x80;
	*p++ = ch;
    }

    memcpy(p, key.data(), key.size());
    check_block();
    memcpy(p + key.size(), tag, tag_len);

    check_block();

    modified = true;
    RETURN(!exact);
}

void
BrassCBlock::del()
{
    LOGCALL_VOID(DB, "BrassCBlock::del", NO_ARGS);
    Assert(!is_leaf());
    // The last entry in the leaf block was removed, so delete item "item"
    // from the parent.
    size_t C = get_count();
    if (C == 1) {
	// This block will become empty, so delete it and remove
	// the corresponding entry from its parent.
	const_cast<BrassTable&>(table).mark_free(n);
	modified = false;
	if (parent)
	    parent->del();
	return;
    }

    if (C == 2 && !parent) {
	// This is the root block, and will now only have one entry,
	// so the Btree can lose a level.
	modified = false;
	const_cast<BrassTable&>(table).lose_level();
	return;
    }

    size_t len = get_endptr(item) - get_ptr(item);
    size_t ptr = get_endptr(C);
    memmove(data + ptr + len, data + ptr, get_ptr(item) - ptr);
    for (size_t i = C - 1; i > size_t(item); --i) {
	set_ptr(i - 1, get_ptr(i) + len);
    }
    set_count(C - 1);
    item = -2;
    check_block();
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

    int C = get_count();
    if (C == 1) {
	if (rare(!parent)) {
	    // Removing the last entry from the root block - the table is now
	    // empty.
	    lose_level();
	    RETURN(true);
	}
	// Block now empty, so delete it and the parent's pointer to it.
	const_cast<BrassTable&>(table).mark_free(n);
	modified = false;
	parent->del();
	RETURN(true);
    }

    size_t len = get_endptr(item) - get_ptr(item);
    size_t ptr = get_endptr(C);
    memmove(data + ptr + len, data + ptr, get_ptr(item) - ptr);
    for (int i = item + 1; i < C; ++i) {
	set_ptr(i - 1, get_ptr(i) + len);
    }
    set_count(C - 1);
    check_block();
    modified = true;
    RETURN(true);
}

// FIXME: factor out repeated code from get() ?
bool
BrassCBlock::find(const string &key, int mode)
{
    LOGCALL(DB, bool, "BrassCBlock::find", key << ", " << mode);
    if (!data) {
	// The empty key should always appear to exist.
	if (key.empty() ^ (mode == LT)) {
	    item = -1;
	    RETURN(true);
	}
	item = -2;
	RETURN(false);
    }

    if (!is_leaf()) {
	find_child(key);
	RETURN(child->find(key, mode));
    }

    check_block();

    bool exact = binary_chop_leaf(key, mode);
    if (!exact) {
	if (mode == GE) {
	    if (item == get_count())
		item = -2;
	    else
		AssertRel(get_key(item),>=,key);
	} else {
	    prev_();
	    if (mode == LT) {
		if (item < get_count())
		    AssertRel(get_key(item),<,key);
	    } else if (mode == LE) {
		if (item < get_count()) {
		    AssertRel(get_key(item),<=,key);
		}
	    }
	}
	RETURN(false);
    }

    // Exact match must be in range.
    AssertRel(item,<,get_count());

    if (mode != LT) {
	if (mode == LE) {
	    AssertRel(get_key(item),<=,key);
	} else {
	    AssertRel(get_key(item),>=,key);
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
    LOGCALL(DB, bool, "BrassCBlock::get", key << ", " << tag);
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
    LOGCALL(DB, bool, "BrassTable::read_block", (void*)buf << ", " << n);
    Assert(fd != FD_NOT_OPEN /*read_block*/);
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
    LOGCALL(DB, bool, "BrassTable::write_block", (void*)buf << ", " << n);
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

BrassCBlock *
BrassTable::gain_level(brass_block_t child)
{
    LOGCALL(DB, BrassCBlock *, "BrassTable::gain_level", child);
    my_cursor = new BrassCBlock(*this, my_cursor, child);
    // cout << my_cursor->n << " GAIN LEVEL, setting wide bounds for new root" << endl;
    // key_limits[my_cursor->n].second.assign(256, '\xff');
    RETURN(my_cursor);
}

void
BrassTable::lose_level()
{
    LOGCALL_VOID(DB, "BrassTable::lose_level", NO_ARGS);
    do {
	my_cursor = my_cursor->lose_level();
	if (rare(!my_cursor))
	    return;
    } while (!my_cursor->is_leaf() && my_cursor->get_count() == 1);
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
    // Assert(key_limits.size() == next_free);
    // key_limits.resize(next_free + 1);
    return next_free++;
}

int
BrassTable::compare_keys(const void *k1, size_t l1, const void *k2, size_t l2) const
{
    LOGCALL(DB, int, "BrassTable::compare_keys", k1 << ", " << l1 << ", " << k2 << ", " << l2);
    int result = memcmp(k1, k2, std::min(l1, l2));
    if (result)
	RETURN(result);
    RETURN((int)l1 - (int)l2);
}

string
BrassTable::divide(const char *k1, size_t l1, const char *k2, size_t l2) const
{
    LOGCALL(DB, string, "BrassTable::divide", k1 << ", " << l1 << ", " << k2 << ", " << l2);
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
    LOGCALL(DB, bool, "BrassTable::create", blocksize_ << ", " << from_scratch);
    (void)from_scratch;
    Assert(!readonly);
    AssertRel(fd,==,FD_NOT_OPEN);

    // FIXME: hardwire blocksize to 64K for now, since we don't support tags
    // stored outside the tree yet.
    blocksize_ = 65536;
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
	errmsg += "': ";
	errmsg += strerror(errno);
	// errcode = DATABASE_OPENING_ERROR;
	RETURN(false);
    }

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
}

bool
BrassTable::open(brass_block_t root)
{
    LOGCALL(DB, bool, "BrassTable::open", root);
    if (fd == FD_NOT_OPEN) {
	// Table not already open.
	blocksize = 65536; // FIXME

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
BrassTable::close()
{
    LOGCALL_VOID(DB, "BrassTable::close", "[bool]");
    if (fd >= 0) {
	delete my_cursor;
	my_cursor = NULL;
	// FIXME: check for error...
	::close(fd);
    }
    fd = FD_CLOSED;
}

bool
BrassTable::add(const string & key, const string & tag, bool already_compressed)
{
    LOGCALL(DB, bool, "BrassTable::add", key << ", " << tag << ", " << already_compressed);
    if (key.empty())
	RETURN(false);
    if (fd < 0) {
	if (fd == FD_CLOSED)
	    throw_database_closed();
	create(blocksize, true);
    }
    if (rare(!my_cursor)) {
	my_cursor = new BrassCBlock(*this);
	my_cursor->new_leaf_block();
	// cout << my_cursor->n << " [0, INF] LIMITS SET for first block in cursor" << endl;
	// key_limits[my_cursor->n].second.assign(256, '\xff');
	(void)my_cursor->insert(string(), NULL, 0, false);
    }
    modified = true;

    if (compress && !already_compressed) {
	if (compressor.try_to_compress(tag)) {
	    const char * p = compressor.compressed_data();
	    size_t len = compressor.compressed_data_len();
	    (void)my_cursor->insert(key, p, len, true);
	    RETURN(true);
	}

	// Data wasn't compressible, so store uncompressed.
    }
    (void)my_cursor->insert(key, tag.data(), tag.size(), already_compressed);
    RETURN(true);
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
    if (fd < 0) {
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
    if (fd == FD_CLOSED)
	throw_database_closed();
    my_cursor->commit();
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
    LOGCALL(DB, bool, "BrassTable::get", key << ", " << tag);
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
