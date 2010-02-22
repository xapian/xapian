/** @file brass_table.h
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

#ifndef XAPIAN_INCLUDED_BRASS_TABLE_H
#define XAPIAN_INCLUDED_BRASS_TABLE_H

#include "brass_defs.h"

#include "noreturn.h"
#include "omassert.h"
#include <string>
#include <vector>

#include "xapian/visibility.h"

#include<iostream> // FIXME
using namespace std;

static const int BLOCKPTR_SIZE = 4;

const bool COMPRESS = true;
const bool DONT_COMPRESS = false;

const bool LAZY = true;
const bool NOT_LAZY = false;

// Convert to little endian form:
#ifdef WORDS_BIGENDIAN
static uint2 LE(uint2 x) {
    return (x & 0xff) << 8 | ((x >> 8) & 0xff);
}

static uint4 LE(uint4 x) {
    return (x & 0xff) << 24 | (x & 0xff00) << 8 | (x >> 8) & 0xff00 | ((x >> 24) & 0xff);
}
#else
#define LE(X) (X)
#endif

static inline brass_block_t get_unaligned_le4(const char *p) {
    const unsigned char * a = (const unsigned char *)p;
    return (brass_block_t)*a | (brass_block_t)a[1] << 8 | (brass_block_t)a[2] << 16 | (brass_block_t)a[3] << 24;
}

static inline void set_unaligned_le4(char *p, brass_block_t b) {
    unsigned char * a = (unsigned char *)p;
    *a = b;
    a[1] = b >> 8;
    a[2] = b >> 16;
    a[3] = b >> 24;
}

class BrassTable;
class BrassCursor;

class XAPIAN_VISIBILITY_DEFAULT BrassBlock {
    friend class BrassTable;
    friend class BrassCursor;
    friend class BrassCBlock;

    /// Copying is not allowed.
    BrassBlock(const BrassBlock &);

    /// Assignment is not allowed.
    void operator=(const BrassBlock &);

    void check_block();

  protected:
    brass_block_t n;
    char * data;
    const BrassTable & table;

    /// Return the length of the block header for @a count items.
    int header_length(int count) const { return 8 + 2 * count; }

  public:
    BrassBlock(const BrassTable & table_) : data(0), table(table_) { }
    void save();
    virtual ~BrassBlock();
    bool is_leaf() const { return (((unsigned char *)data)[5] & 0x80) == 0; }

    brass_revision_number_t get_revision() const {
	return LE(*(brass_revision_number_t*)data);
    }

    void set_revision(brass_revision_number_t revision) {
	*(brass_revision_number_t*)data = LE(revision);
    }

    /// Returns the number of items in this block.
    int get_count() const {
	return LE(((uint2 *)data)[2]) & 0x7fff;
    }

    /// Sets the number of items in this block.
    void set_count(int count) {
	((uint2 *)data)[2] = LE((uint2)count) | (is_leaf() ? 0 : 0x8000);
    }

    int get_ptr(int i) const {
	return LE(((uint2 *)data)[4 + i]);
    }

    int get_endptr(int i) const;

    const string get_key(int i) const {
	const char * p = data + get_ptr(i);
	size_t len;
	if (is_leaf()) {
	    len = static_cast<unsigned char>(*p++);
	} else {
	    p += 4;
	    len = get_endptr(i) - get_ptr(i) - 4;
	}
	return string(p, len);
    }

    void set_ptr(int i, int ptr) {
	((uint2 *)data)[4 + i] = LE((uint2)ptr);
    }

    brass_block_t get_left_block() const;

    void set_left_block(brass_block_t b);

    brass_block_t get_block(int i) const;
    void set_block(int i, brass_block_t b) {
	Assert(i != -1);
	set_unaligned_le4(data + get_ptr(i), b);
    }
    void new_leaf_block();
    void new_branch_block();
};

class XAPIAN_VISIBILITY_DEFAULT BrassCBlock : public BrassBlock {
    /// Copying is not allowed.
    BrassCBlock(const BrassCBlock &);

    /// Assignment is not allowed.
    void operator=(const BrassCBlock &);

    void set_child_block_number(brass_block_t n_child);

    /** For a branch block, the item number of child.  If child is the left
     *  pointer, then item is -1. */
  protected: // FIXME: what?  Definitely: item
    int item;

    bool modified;

    /// If we modify this block, do we need to clone it first?
    bool needs_clone;

    BrassCBlock * parent, * child;

    bool next_() {
//	cout << "next_() data = " << (void*)data << " item = " << item << "/" << get_count() << endl;
	if (!data)
	   item = -2;

	if (item == -2)
	    return false;

	if (++item >= get_count()) {
	    if (!parent || !parent->next_()) {
		// Root block or parent at end, so end of iteration.
		item = -2;
//	cout << "next_() returned false, item now " << item << "/" << get_count() << endl;
		return false;
	    }
	    item = (child ? -1 : 0);
	}

	if (child)
	    child->read(get_block(item));

//	cout << "next_() returned true, item now " << item << "/" << get_count() << endl;
	return true;
    }

    bool prev_() {
	if (!data)
	    item = -2;

	if (item == -2)
	    return false;

	if (item == (child ? -1 : 0)) {
	    if (!parent || !parent->prev_()) {
		// Root block or parent at end, so end of iteration.
		item = -2;
		return false;
	    }
	    item = get_count();
	}
	--item;

	if (child)
	    child->read(get_block(item));

	return true;
    }

    void read_key(std::string & key) {
	AssertRel(item,>=,-1);
	AssertRel(item,<,get_count());
	if (child) {
	    child->read_key(key);
	    return;
	}
	int key_start = get_ptr(item) + 1;
	int key_len = (unsigned char)data[key_start - 1];
	key.assign(data + key_start, key_len);
    }

    // Always returns true to allow tail-calling.
    bool read_tag(std::string &tag) {
	AssertRel(item,>=,-1);
	AssertRel(item,<,get_count());
	if (!is_leaf())
	    return child->read_tag(tag);
	const char * s = data + get_ptr(item);
	size_t key_len = static_cast<unsigned char>(*s++);
	s += key_len;
	AssertRel(s - data,<=,get_endptr(item));
	tag.assign(s, data + get_endptr(item) - s);
	return true;
    }

  public:
    BrassCBlock(const BrassTable & table_)
	: BrassBlock(table_), item(-2), modified(false), needs_clone(false),
	  parent(NULL), child(NULL)
    { }

    BrassCBlock(const BrassTable & table_, BrassCBlock * parent_)
	: BrassBlock(table_), item(-2), modified(false), needs_clone(false),
	  parent(parent_), child(NULL)
    { }

    BrassCBlock(const BrassTable & table_, BrassCBlock * child_, brass_block_t blk)
	: BrassBlock(table_), item(-1), modified(true), needs_clone(false),
	  parent(NULL), child(child_)
    {
	new_branch_block();
	set_left_block(blk);
    }

    virtual ~BrassCBlock();
    BrassCBlock * lose_level() {
	Assert(!parent);
	BrassCBlock * new_root = child;
	delete this;
	return new_root;
    }
    void read(brass_block_t blk);

    /** Find which child block @a key belongs in, set item to it and load it
     *  into the child cursor.
     *
     *  @param key The key we want to find or add.
     */
    void find_child(const std::string & key);

    bool binary_chop_leaf(const std::string & key, int mode);

    void insert(const std::string &key, brass_block_t tag);
    bool insert(const std::string &key, const std::string &tag);
    void del();
    bool del(const std::string &key);
    void commit() {
	if (modified) {
	    save();
	    modified = false;
	}
	if (child) child->commit();
    }
    void cancel() {
	modified = false;
	if (child) child->cancel();
    }

    enum { EQ = 1, LT = 2, LE = 3, GE = 5 };

    bool find(const std::string &key, int mode);

    bool key_exists(const std::string &key);

    bool get(const std::string &key, std::string &tag);

    bool next() {
//	cout << "in next(), item is " << item << endl;
	if (!child) {
	    Assert(!data || is_leaf());
	    return next_();
	}
	return child->next();
    }

    bool prev() {
	if (!child) {
	    Assert(!data || is_leaf());
	    return prev_();
	}
	return child->prev();
    }

    // NB Doesn't need to be virtual since we never store a BrassCBlock * in a
    // BrassBlock * and then call functions on it.
    void new_leaf_block() {
	needs_clone = false;
	BrassBlock::new_leaf_block();
    }

    void set_needs_clone() { needs_clone = true; }
};

class XAPIAN_VISIBILITY_DEFAULT BrassTable {
    friend class BrassBlock;
    friend class BrassCBlock;
    friend class BrassCursor;

    /// Copying is not allowed.
    BrassTable(const BrassTable &);

    /// Assignment is not allowed.
    void operator=(const BrassTable &);

    /// Throw an exception indicating that the database is closed.
    XAPIAN_NORETURN(static void throw_database_closed());

  protected:
    enum {
	FD_CLOSED = -2, // Database::close() called explicitly.
	FD_NOT_OPEN = -1 // Table not yet opened (maybe non-existent lazy table).
    };

    int fd;
    unsigned int blocksize;
    std::string path;
    bool readonly;
    std::string errmsg;
    brass_block_t next_free;
    brass_revision_number_t revision;

    BrassBlock split;
    BrassCBlock * my_cursor;

    bool compress;
    bool lazy;
    bool modified;

    // mutable std::vector<std::pair<std::string, std::string> > key_limits;

    bool read_block(char *buf, brass_block_t n) const;
    bool write_block(const char *buf, brass_block_t n) const;

    BrassCBlock * gain_level(brass_block_t child);
    void lose_level();

    brass_block_t get_free_block();

    void mark_free(brass_block_t n) {
	AssertRel(n,<,next_free);
	// key_limits[n].second = std::string();
	// FIXME: add "n" to the freelist for revision "revision".
    }

    virtual int compare_keys(const void *k1, size_t l1,
			     const void *k2, size_t l2) const;
    virtual std::string divide(const char *k1, size_t l1, const char *k2, size_t l2) const;

  public:
    BrassTable(const char * name_, const std::string & path_, bool readonly_,
	       bool compress_ = DONT_COMPRESS, bool lazy_ = NOT_LAZY)
	: fd(FD_NOT_OPEN), blocksize(0), path(path_), readonly(readonly_),
	  next_free(0), revision(0), split(*this), my_cursor(NULL),
	  compress(compress_), lazy(lazy_), modified(false)
    {
	(void)name_; // FIXME
    }

    virtual ~BrassTable();

    bool exists() const;

    /** Create the table on disk.
     *
     *  @param blocksize_	The blocksize to use for this table.
     *  @param from_scratch	True if this table is know to not be present
     *				already (e.g. because the parent directory
     *				was just created).
     */
    bool create(unsigned int blocksize_, bool from_scratch);

    void erase();

    void set_block_size(unsigned int blocksize_) { blocksize = blocksize_; }

    unsigned int get_block_size() const { return blocksize; }

    /** Open (or reopen) the table with the specified root block. */
    bool open(brass_block_t root_);

    void close();

    bool add(const std::string & key, const std::string & tag,
	     bool already_compressed = false);

    bool del(const std::string & key);

    bool is_modified() const { return modified; }

    // FIXME: Re-merge flush() and commit()?  or do they still need to stay
    // split for replication now per-table base files are gone?
    void flush() { }

    void commit(brass_revision_number_t revision_);

    void cancel();

    bool key_exists(const std::string & key) const;

    bool get(const std::string & key, std::string & tag) const;

    bool is_open() const { if (rare(fd == FD_CLOSED))
	BrassTable::throw_database_closed(); return (fd >= 0); }

    BrassCursor * get_cursor() const;

    //brass_revision_number_t get_open_revision_number() const { return
    //revision;
//    }

    bool empty() const {
	if (rare(fd == -2))
	    throw_database_closed();
	return my_cursor == NULL;
    }

    void set_full_compaction(bool) { } // FIXME support?

    void set_max_item_size(size_t) { } // FIXME support?

    brass_block_t get_root() const {
       if (!my_cursor)
	   return brass_block_t(-1);
       return my_cursor->n;
    }
};

inline void BrassCBlock::set_child_block_number(brass_block_t n_child) {
    AssertRel(item,>=,0);
    set_block(item, n_child);
    // cout << "parent block " << n << " needs_clone " << needs_clone << endl;

    if (!needs_clone)
	return;
    needs_clone = false;
    //int n_orig = n;
    n = const_cast<BrassTable&>(table).get_free_block();
    // cout << n << " COPYING BOUNDS from " << n_orig << endl;
    // table.key_limits[n] = table.key_limits[n_orig];
    // cout << "cloned to " << n << endl;
    parent->set_child_block_number(n);
    // cout << "post parent clone check" << endl;
    check_block();
    // cout << "done post parent clone check" << endl;
}

inline brass_block_t BrassBlock::get_block(int i) const {
    if (i == -1)
	return get_left_block();
    return get_unaligned_le4(data + get_ptr(i));
}

#endif // XAPIAN_INCLUDED_BRASS_TABLE_H
