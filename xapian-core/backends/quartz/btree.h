/* btree.h: Btree implementation
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2005,2006 Olly Betts
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

#ifndef OM_HGUARD_BTREE_H
#define OM_HGUARD_BTREE_H

#include <algorithm>
#include <string>
using std::string;

#include "quartz_types.h"
#include "btree_base.h"
#include "btree_util.h"
#include "bcursor.h"

/** The largest possible value of a key_len.
 *
 *  This gives the upper limit of the size of a key that may be stored in the
 *  B-tree (252 bytes with the present implementation).
 */
const string::size_type BTREE_MAX_KEY_LEN = 252;

// FIXME: This named constant probably isn't used everywhere it should be...
#define BYTES_PER_BLOCK_NUMBER 4

/*  The B-tree blocks have a number of internal lengths and offsets held in 1, 2
    or 4 bytes. To make the coding a little clearer,
       we use  for
       ------  ---
       K1      the 1 byte length of key
       I2      the 2 byte length of an item (key-tag pair)
       D2      the 2 byte offset to the item from the directory
       C2      the 2 byte counter that ends each key and begins each tag
*/

#define K1 1
#define I2 2
#define D2 2
#define C2 2

/*  and when getting K1 or setting D2, we use GETK, SETD defined as: */

#define GETK(p, c)    GETINT1(p, c)
#define SETD(p, c, x) SETINT2(p, c, x)

/* A B-tree comprises (a) a base file, containing essential information (Block
   size, number of the B-tree root block etc), (b) a bitmap, the Nth bit of the
   bitmap being set if the Nth block of the B-tree file is in use, and (c) a
   file DB containing the B-tree proper. The DB file is divided into a sequence
   of equal sized blocks, numbered 0, 1, 2 ... some of which are free, some in
   use. Those in use are arranged in a tree.

   Each block, b, has a structure like this:

     R L M T D o1 o2 o3 ... oN <gap> [item] .. [item] .. [item] ...
     <---------- D ----------> <-M->

   And then,

   R = REVISION(b)  is the revision number the B-tree had when the block was
                    written into the DB file.
   L = GET_LEVEL(b) is the level of the block, which is the number of levels
                    towards the root of the B-tree structure. So leaf blocks
                    have level 0 and the one root block has the highest level
                    equal to the number of levels in the B-tree.
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

           I K key x C tag
             <--K-->
           <------I------>

   A long tag presented through the API is split up into C tags small enough to
   be accommodated in the blocks of the B-tree. The key is extended to include
   a counter, x, which runs from 1 to C. The key is preceded by a length, K,
   and the whole item with a length, I, as depicted above.

   Here are the corresponding definitions:

*/

#define REVISION(b)      static_cast<unsigned int>(get_int4(b, 0))
#define GET_LEVEL(b)     GETINT1(b, 4)
#define MAX_FREE(b)      GETINT2(b, 5)
#define TOTAL_FREE(b)    GETINT2(b, 7)
#define DIR_END(b)       GETINT2(b, 9)
#define DIR_START        11

#define SET_REVISION(b, x)      set_int4(b, 0, x)
#define SET_LEVEL(b, x)         SETINT1(b, 4, x)
#define SET_MAX_FREE(b, x)      SETINT2(b, 5, x)
#define SET_TOTAL_FREE(b, x)    SETINT2(b, 7, x)
#define SET_DIR_END(b, x)       SETINT2(b, 9, x)

/** Flip to sequential addition block-splitting after this number of observed
 *  sequential additions (in negated form). */
#define SEQ_START_POINT (-10)

/** Even for items of at maximum size, it must be possible to get this number of
 *  items in a block */
#define BLOCK_CAPACITY 4



/* if you've been reading the comments from the top, the next four procedures
   will not cause any headaches.

   Recall that item has this form:

           i k
           | |
           I K key x C tag
             <--K-->
           <------I------>


   item_of(p, c) returns i, the address of the item at block address p,
   directory offset c,

   component_of(p, c) returns the number marked 'x' above,

   components_of(p, c) returns the number marked 'C' above,
*/

class Key {
    const byte *p;
public:
    explicit Key(const byte * p_) : p(p_) { }
    const byte * get_address() const { return p; }
    void read(string * key) const {
	key->assign(reinterpret_cast<const char *>(p + K1), length());
    }
    bool operator==(Key key2) const;
    bool operator!=(Key key2) const { return !(*this == key2); }
    bool operator<(Key key2) const;
    bool operator>=(Key key2) const { return !(*this < key2); }
    bool operator>(Key key2) const { return key2 < *this; }
    bool operator<=(Key key2) const { return !(key2 < *this); }
    int length() const {
	return GETK(p, 0) - C2 - K1;
    }
    char operator[](size_t i) const {
	return p[i + K1];
    }
};

// Item_wr wants to be "Item with non-const p and more methods" - we can't
// achieve that nicely with inheritance, so we use a template base class.
template <class T> class Item_base {
protected:
    T p;
public:
    /* Item from block address and offset to item pointer */
    Item_base(T p_, int c) : p(p_ + GETINT2(p_, c)) { }
    Item_base(T p_) : p(p_) { }
    T get_address() const { return p; }
    int size() const { return GETINT2(p, 0); } /* I in diagram above */
    int component_of() const {
	return GETINT2(p, GETK(p, I2) + I2 - C2);
    }
    int components_of() const {
	return GETINT2(p, GETK(p, I2) + I2);
    }
    Key key() const { return Key(p + I2); }
    void append_chunk(string * tag) const {
	/* number of bytes to extract from current component */
	int cd = GETK(p, I2) + I2 + C2;
	int l = size() - cd;
	tag->append(reinterpret_cast<const char *>(p + cd), l);
    }
    /** Get this item's tag as a block number (this block should not be at
     *  level 0).
     */
    uint4 block_given_by() const {
	return get_int4(p, size() - BYTES_PER_BLOCK_NUMBER);
    }
};

class Item : public Item_base<const byte *> {
public:
    /* Item from block address and offset to item pointer */
    Item(const byte * p_, int c) : Item_base<const byte *>(p_, c) { }
    Item(const byte * p_) : Item_base<const byte *>(p_) { }
};

class Item_wr : public Item_base<byte *> {
    void set_key_len(int x) { SETINT1(p, I2, x); }
public:
    /* Item_wr from block address and offset to item pointer */
    Item_wr(byte * p_, int c) : Item_base<byte *>(p_, c) { }
    Item_wr(byte * p_) : Item_base<byte *>(p_) { }
    void set_component_of(int i) {
	SETINT2(p, GETK(p, I2) + I2 - C2, i);
    }
    void set_components_of(int m) {
	SETINT2(p, GETK(p, I2) + I2, m);
    }
    // Takes size as we may be truncating newkey.
    void set_key_and_block(Key newkey, int truncate_size, uint4 n) {
	int i = truncate_size;
	// Read the length now because we may be copying the key over itself.
	// FIXME that's stupid!  sort this out
	int newkey_len = newkey.length();
	int newsize = I2 + K1 + i + C2;
	// Item size (4 since tag contains block number)
	SETINT2(p, 0, newsize + 4);
	// Key size
	SETINT1(p, I2, newsize - I2);
	// Copy the main part of the key, possibly truncating.
	memmove(p + I2 + K1, newkey.get_address() + K1, i);
	// Copy the count part.
	memmove(p + I2 + K1 + i, newkey.get_address() + K1 + newkey_len, C2);
	// Set tag contents to block number
//	set_block_given_by(n);
	set_int4(p, newsize, n);
    }

    /** Set this item's tag to point to block n (this block should not be at
     *  level 0).
     */
    void set_block_given_by(uint4 n) {
	set_int4(p, size() - BYTES_PER_BLOCK_NUMBER, n);
    }
    void set_size(int l) { SETINT2(p, 0, l); }
    /** Form an item with a null key and with block number n in the tag.
     */
    void form_null_key(uint4 n) {
	set_int4(p, I2 + K1, n);
	set_key_len(K1);        /* null key */
	set_size(I2 + K1 + 4);  /* total length */
    }
    void form_key(const string & key_) {
	Assert(key_.length() <= BTREE_MAX_KEY_LEN);

	// This just so it doesn't fall over horribly in non-debug builds.
	string::size_type key_len = std::min(key_.length(), BTREE_MAX_KEY_LEN);

	set_key_len(key_len + K1 + C2);
	memmove(p + I2 + K1, key_.data(), key_len);
	set_component_of(1);
    }
    // FIXME passing cd here is icky
    void set_tag(int cd, const char *start, int len) {
	memmove(p + cd, start, len);
	set_size(cd + len);
    }
    void fake_root_item() {
	set_key_len(K1 + C2);   // null key length
	set_size(I2 + K1 + 2 * C2);   // length of the item
	set_component_of(1);
	set_components_of(1);
    }
};

// Allow for BTREE_CURSOR_LEVELS levels in the B-tree.
// With 10, overflow is practically impossible
// FIXME: but we want it to be completely impossible...
#define BTREE_CURSOR_LEVELS 10

/** Class managing a Btree table in a Quartz database.
 *
 *  A table is a store holding a set of key/tag pairs.
 *
 *  A key is used to access a block of data in a quartz table.
 * 
 *  Keys are of limited length.
 *
 *  Keys may not be empty (each Btree has a special empty key for internal use).
 *
 *  A tag is a piece of data associated with a given key.  The contents
 *  of the tag are opaque to the Btree.
 *
 *  Tags may be of arbitrary length (the Btree imposes a very large limit).
 *  Note though that they will be loaded into memory in their entirety, so
 *  should not be permitted to grow without bound in normal usage.
 *
 *  Tags which are null strings _are_ valid, and are different from a
 *  tag simply not being in the table.
 */
class Btree {
    friend class Bcursor; /* Should probably fix this. */
    private:
	/// Copying not allowed
        Btree(const Btree &);

	/// Assignment not allowed
        Btree & operator=(const Btree &);

    public:
	/** Create a new Btree object.
	 *
	 *  This does not create the table on disk - the create() method must
	 *  be called to create the table on disk.
	 *
	 *  This also does not open the table - the open() method must be
	 *  called before use is made of the table.
	 *
	 *  @param path_          - Path at which the table is stored.
	 *  @param readonly_      - whether to open the table for read only
	 *                          access.
	 */
	Btree(string path_, bool readonly_);

	/** Close the Btree.
	 *
	 *  Any outstanding changes (ie, changes made without commit() having
	 *  subsequently been called) will be lost.
	 */
	~Btree();

	/** Close the Btree.  This closes and frees any of the btree
	 *  structures which have been created and opened.
	 */
	void close();

	/** Determine whether the btree exists on disk.
	 */
	bool exists() const;

	/** Open the btree at the latest revision.
	 *
	 *  @exception Xapian::DatabaseCorruptError will be thrown if the table
	 *	is in a corrupt state.
	 *  @exception Xapian::DatabaseOpeningError will be thrown if the table
	 *	cannot be opened (but is not corrupt - eg, permission problems,
	 *	not present, etc).
	 */
	void open();

	/** Open the btree at a given revision.
	 *
	 *  Like Btree::open, but try to open at the given revision number
	 *  and fail if that isn't possible.
	 *
	 *  @param revision_      - revision number to open.
	 *
	 *  @return true if table is successfully opened at desired revision;
	 *          false if table cannot be opened at desired revision (but
	 *          table is otherwise consistent).
	 *
	 *  @exception Xapian::DatabaseCorruptError will be thrown if the table
	 *	is in a corrupt state.
	 *  @exception Xapian::DatabaseOpeningError will be thrown if the table
	 *	cannot be opened (but is not corrupt - eg, permission problems,
	 *	not present, etc).
	 */
	bool open(quartz_revision_number_t revision_);

	/** Commit any outstanding changes to the table.
	 *
	 *  Commit changes made by calling add() and del() to the Btree.
	 *
	 *  If an error occurs during the operation, this will be signalled
	 *  by an exception.  In case of error, changes made will not be
	 *  committed to the Btree - they will be discarded.
	 *
	 *  @param new_revision  The new revision number to store.  This must
	 *          be greater than the latest revision number (see
	 *          get_latest_revision_number()), or an exception will be
	 *          thrown.
	 */
	void commit(quartz_revision_number_t revision);

	/** Cancel any outstanding changes.
	 *
	 *  This will discard any modifications which haven't been committed
	 *  by calling commit().
	 */
	void cancel();

	/** Read an entry from the table, if and only if it is exactly that
	 *  being asked for.
	 *
	 *  If the key is found in the table, the tag will be filled with
	 *  the data associated with the key.  If the key is not found,
	 *  the tag will be unmodified.
	 *
	 *  @param key  The key to look for in the table.
	 *  @param tag  A tag object to fill with the value if found.
	 *
	 *  @return true if key is found in table,
	 *          false if key is not found in table.
	 */
	bool get_exact_entry(const string & key, string & tag) const;

	/** Find a key in the Btree and read its tag.
	 *
	 *  If the key is found the tag is copied to tag.  If the key is not
	 *  found tag is left unchanged.
	 * 
	 *  The result is true iff the specified key is found in the Btree.
	 *
	 *  e.g.
	 *
	 *    string t;
	 *    btree.find_tag("TODAY", &t); // get today's date
	 */
	bool find_tag(const string &key, string * tag) const;

	/** Read the tag value for the key pointed to by cursor C_.
	 */
	void read_tag(Cursor * C_, string *tag) const;

	/** Add a key/tag pair to the table, replacing any existing pair with
	 *  the same key.
	 *
	 *  If an error occurs during the operation, this will be signalled
	 *  by a return value of false.  All modifications since the
	 *  previous commit() will be lost.
	 *
	 *  If key is empty, then the null item is replaced.  If key.length()
	 *  exceeds the the limit on key size, false is returned.
	 *
	 *  e.g.    ok = btree.add("TODAY", "Mon 9 Oct 2000");
	 *
	 *  @param key   The key to store in the table.
	 *  @param tag   The tag to store in the table.
	 *
	 *  @return true if the operation completed successfully, false
	 *          otherwise.
	 */
	bool add(const string &key, string tag);

	/** Delete an entry from the table.
	 *
	 *  The entry will be removed from the table, if it exists.  If
	 *  it does not exist, no action will be taken.  The item with
	 *  an empty key can't be removed, and false is returned.
	 *
	 *  If an error occurs during the operation, this will be signalled
	 *  by a return value of false.  All modifications since the
	 *  previous commit() will be lost.
	 *
	 *  e.g.    ok = btree.del("TODAY")
	 *
	 *  @param key   The key to remove from the table.
	 *
	 *  @return true if the operation completed successfully, false
	 *          otherwise.
	 */
	bool del(const string &key);

	/** Create a new empty btree structure on disk.
	 *
	 *  The block size must be less than 64K, where K = 1024. It is unwise
	 *  to use a small block size (less than 1024 perhaps), so we enforce a
	 *  minimum block size of 2K.
	 *
	 *  Example:
	 *
	 *    Btree btree("X-");
	 *    btree.create(8192);  // files will be X-DB, X-baseA (and X-baseB)
	 *
	 *  @param blocksize     - Size of blocks to use.
	 *
	 *  @exception Xapian::DatabaseCreateError if the table can't be created.
	 *  @exception Xapian::InvalidArgumentError if the requested blocksize
	 *  is unsuitable.
	 */
	void create(unsigned int blocksize);

	void set_full_compaction(bool parity);

	/** Get the latest revision number stored in this table.
	 *
	 *  This gives the higher of the revision numbers held in the base
	 *  files of the B-tree, or just the revision number if there's only
	 *  one base file.
	 *
	 *  It is possible that there are other, older, revisions of this
	 *  table available, and indeed that the revision currently open
	 *  is one of these older revisions.
	 */
	quartz_revision_number_t get_latest_revision_number() const {
	    return latest_revision_number;
	}

	/** Get the revision number at which this table
	 *  is currently open.
	 *
	 *  It is possible that there are other, more recent or older
	 *  revisions available.
	 *
	 *  @return the current revision number.
	 */
	quartz_revision_number_t get_open_revision_number() const {
	    return revision_number;
	}

	/** Return a count of the number of entries in the table.
	 *
	 *  The count does not include the ever-present item with null key.
	 *
	 *  @return The number of entries in the table.
	 */
	quartz_tablesize_t get_entry_count() const {
	    return item_count;
	}

	/** Get a cursor for reading from the table.
	 *
	 *  The cursor is owned by the caller - it is the caller's
	 *  responsibility to ensure that it is deleted.
	 */
	Bcursor * cursor_get() const;

	/** Determine whether the object contains uncommitted modifications.
	 *
	 *  @return true if there have been modifications since the last
	 *          the last call to commit().
	 */
	bool is_modified() const { return Btree_modified; }

	/** Set the maximum item size given the block capacity.
	 *
	 *  At least this many items of maximum size must fit into a block.
	 *  The default is BLOCK_CAPACITY (which is currently 4).
	 */
	void set_max_item_size(size_t block_capacity) {
	    if (block_capacity > 4) block_capacity = 4;
	    max_item_size = (block_size - DIR_START - block_capacity * D2)
		/ block_capacity;
	}

    protected:

	/** Perform the opening operation to read.
	 *
	 *  Return true iff the open succeeded.
	 */
	bool do_open_to_read(bool revision_supplied, quartz_revision_number_t revision_);

	/** Perform the opening operation to write.
	 *
	 *  Return true iff the open succeeded.
	 */
	bool do_open_to_write(bool revision_supplied, quartz_revision_number_t revision_);
	bool basic_open(bool revision_supplied, quartz_revision_number_t revision);

	bool find(Cursor *) const;
	int delete_kt();
	void read_block(uint4 n, byte *p) const;
	void write_block(uint4 n, const byte *p) const;
	void set_overwritten() const;
	void block_to_cursor(Cursor *C_, int j, uint4 n) const;
	void alter();
	void compact(byte *p);
	void enter_key(int j, Key prevkey, Key newkey);
	int mid_point(byte *p);
	void add_item_to_block(byte *p, Item_wr kt, int c);
	void add_item(Item_wr kt, int j);
	void delete_item(int j, bool repeatedly);
	int add_kt(bool found);
	void read_root();
	void split_root(uint4 split_n);
	void form_key(const string & key) const;

	/** revision number of the opened B-tree. */
	quartz_revision_number_t revision_number;

	/** keeps a count of the number of items in the B-tree. */
	uint4 item_count;

	/** block size of the B tree in bytes */
	unsigned int block_size;

	/** Revision number of the other base, or zero if there is only one
	 *  base file.
	 */
	mutable quartz_revision_number_t latest_revision_number;

	/** set to true if baseA and baseB both exist as valid bases.
	 *
	 *  The unused base is deleted as soon as a write to the Btree takes
	 *  place. */
	mutable bool both_bases;

	/** the value 'A' or 'B' of the current base */
	int base_letter;

	/** true if the root block is faked (not written to disk).
	 * false otherwise.  This is true when the btree hasn't been
	 * modified yet.
	 */
	bool faked_root_block;

	/** true iff the data has been written in a single write in
	 * sequential order.
	 */
	bool sequential;

	/// corresponding file handle
	int handle;

	/// number of levels, counting from 0
	int level;

	/// the root block of the B-tree
	uint4 root;

	/// buffer of size block_size for making up key-tag items
	mutable Item_wr kt;

	/// buffer of size block_size for reforming blocks
	byte * buffer;

	/// For writing back as file baseA or baseB.
	Btree_base base;

	/// The base letter ('B' or 'A') of the next base.
	char other_base_letter;

	/// The path name of the B tree.
	string name;

	/** count of the number of successive instances of purely
	 * sequential addition, starting at SEQ_START_POINT (neg) and
	 * going up to zero. */
	int seq_count;

	/** the last block to be changed by an addition */
	uint4 changed_n;

	/** directory offset corresponding to last block to be changed
	 *  by an addition */
	int changed_c;

	/// maximum size of an item (key-tag pair)
	size_t max_item_size;

	/// Set to true the first time the B-tree is modified.
	mutable bool Btree_modified;

	/// set to true when full compaction is to be achieved
	bool full_compaction;

	/// Set to true when the database is opened to write.
	bool writable;

	/// Set to true if we shouldn't close handle ourselves.
	bool dont_close_handle;

	/* B-tree navigation functions */
	bool prev(Cursor *C_, int j) const { return (this->*prev_ptr)(C_, j); }
	bool next(Cursor *C_, int j) const { return (this->*next_ptr)(C_, j); }

	bool (Btree::* prev_ptr)(Cursor *, int) const;
	bool (Btree::* next_ptr)(Cursor *, int) const;

	bool prev_default(Cursor *C_, int j) const;
	bool next_default(Cursor *C_, int j) const;

	bool prev_for_sequential(Cursor *C_, int dummy) const;
	bool next_for_sequential(Cursor *C_, int dummy) const;

	static int find_in_block(const byte * p, Key key, bool leaf, int c);

	/** block_given_by(p, c) finds the item at block address p, directory
	 *  offset c, and returns its tag value as an integer.
	 */
	static uint4 block_given_by(const byte * p, int c);

	mutable Cursor C[BTREE_CURSOR_LEVELS];

	/** Buffer used when splitting a block.
	 *
	 *  This buffer holds the split off part of the block.  It's only used
	 *  when updating (in Btree::add_item().
	 */
	byte * split_p;

	/* Debugging methods */
//	void report_block_full(int m, int n, const byte * p);
};

#endif /* OM_HGUARD_BTREE_H */
