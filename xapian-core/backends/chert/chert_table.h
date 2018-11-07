/** @file chert_table.h
 * @brief Btree implementation
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2012,2015,2016 Olly Betts
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

#ifndef OM_HGUARD_CHERT_TABLE_H
#define OM_HGUARD_CHERT_TABLE_H

#include <xapian/error.h>

#include "chert_types.h"
#include "chert_btreebase.h"
#include "chert_cursor.h"

#include "noreturn.h"
#include "omassert.h"
#include "str.h"
#include "stringutils.h"
#include "wordaccess.h"

#include <algorithm>
#include <string>

#include <zlib.h>

// FIXME: 65536 in Asserts below is the max chert block size.  We should
// abstract this out, and use the current block_size to catch overruns better.
inline int
getint1(const unsigned char *p, int c)
{
    AssertRel(c, >=, 0);
    AssertRel(c, <, 65536);
    return p[c];
}

inline void
setint1(unsigned char *p, int c, int x)
{
    AssertRel(c, >=, 0);
    AssertRel(c, <, 65536);
    p[c] = x;
}

inline int
getint2(const unsigned char *p, int c)
{
    AssertRel(c, >=, 0);
    AssertRel(c, <, 65536 - 1);
    return unaligned_read2(p + c);
}

inline void
setint2(unsigned char *p, int c, int x)
{
    AssertRel(c, >=, 0);
    AssertRel(c, <, 65536 - 1);
    unaligned_write2(p + c, uint16_t(x));
}

inline int
getint4(const unsigned char *p, int c)
{
    AssertRel(c, >=, 0);
    AssertRel(c, <, 65536 - 3);
    return unaligned_read4(p + c);
}

inline void
setint4(unsigned char *p, int c, int x)
{
    AssertRel(c, >=, 0);
    AssertRel(c, <, 65536 - 3);
    unaligned_write4(p + c, uint32_t(x));
}

const int DONT_COMPRESS = -1;

/** The largest possible value of a key_len.
 *
 *  This gives the upper limit of the size of a key that may be stored in the
 *  B-tree (252 bytes with the present implementation).
 */
#define CHERT_BTREE_MAX_KEY_LEN 252

/** Even for items of at maximum size, it must be possible to get this number of
 *  items in a block */
const size_t BLOCK_CAPACITY = 4;

// FIXME: This named constant probably isn't used everywhere it should be...
const int BYTES_PER_BLOCK_NUMBER = 4;

/*  The B-tree blocks have a number of internal lengths and offsets held in 1, 2
    or 4 bytes. To make the coding a little clearer,
       we use  for
       ------  ---
       K1      the 1 byte length of key
       I2      the 2 byte length of an item (key-tag pair)
       D2      the 2 byte offset to the item from the directory
       C2      the 2 byte counter that ends each key and begins each tag
*/

const int K1 = 1;
const int I2 = 2;
const int D2 = 2;
const int C2 = 2;

/*  and when getting K1 or setting D2, we use getK, setD defined as: */

inline int getK(const unsigned char *p, int c) { return getint1(p, c); }
inline void setD(unsigned char *p, int c, int x) { setint2(p, c, x); }

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

inline unsigned REVISION(const uint8_t * b) { return aligned_read4(b); }
inline int GET_LEVEL(const uint8_t * b) { return getint1(b, 4); }
inline int MAX_FREE(const uint8_t * b) { return getint2(b, 5); }
inline int TOTAL_FREE(const uint8_t * b) { return getint2(b, 7); }
inline int DIR_END(const uint8_t * b) { return getint2(b, 9); }
const int DIR_START = 11;

inline void SET_REVISION(uint8_t * b, uint4 rev) { aligned_write4(b, rev); }
inline void SET_LEVEL(uint8_t * b, int x) { setint1(b, 4, x); }
inline void SET_MAX_FREE(uint8_t * b, int x) { setint2(b, 5, x); }
inline void SET_TOTAL_FREE(uint8_t * b, int x) { setint2(b, 7, x); }
inline void SET_DIR_END(uint8_t * b, int x) { setint2(b, 9, x); }

// The item size is stored in 2 bytes, but the top bit is used to store a flag
// for "is the tag data compressed".
const size_t CHERT_MAX_ITEM_SIZE = 0x7fff;

class Key {
    const uint8_t *p;
public:
    explicit Key(const uint8_t * p_) : p(p_) { }
    const uint8_t * get_address() const { return p; }
    void read(std::string * key) const {
	key->assign(reinterpret_cast<const char *>(p + K1), length());
    }
    bool operator==(Key key2) const;
    bool operator!=(Key key2) const { return !(*this == key2); }
    bool operator<(Key key2) const;
    bool operator>=(Key key2) const { return !(*this < key2); }
    bool operator>(Key key2) const { return key2 < *this; }
    bool operator<=(Key key2) const { return !(key2 < *this); }
    int length() const {
	AssertRel(getK(p, 0),>=,3);
	return getK(p, 0) - C2 - K1;
    }
    char operator[](size_t i) const {
	AssertRel(i,<,(size_t)length());
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
    Item_base(T p_, int c) : p(p_ + getint2(p_, c)) { }
    explicit Item_base(T p_) : p(p_) { }
    T get_address() const { return p; }
    /** I in diagram above. */
    int size() const {
	int item_size = getint2(p, 0) & CHERT_MAX_ITEM_SIZE;
	AssertRel(item_size,>=,5);
	return item_size;
    }
    bool get_compressed() const { return *p & 0x80; }
    int component_of() const {
	return getint2(p, getK(p, I2) + I2 - C2);
    }
    int components_of() const {
	return getint2(p, getK(p, I2) + I2);
    }
    Key key() const { return Key(p + I2); }
    void append_chunk(std::string * tag) const {
	/* number of bytes to extract from current component */
	int cd = getK(p, I2) + I2 + C2;
	int l = size() - cd;
	tag->append(reinterpret_cast<const char *>(p + cd), l);
    }
    /** Get this item's tag as a block number (this block should not be at
     *  level 0).
     */
    uint4 block_given_by() const {
	AssertRel(size(),>=,BYTES_PER_BLOCK_NUMBER);
	return getint4(p, size() - BYTES_PER_BLOCK_NUMBER);
    }
};

class Item : public Item_base<const uint8_t *> {
public:
    /* Item from block address and offset to item pointer */
    Item(const uint8_t * p_, int c) : Item_base<const uint8_t *>(p_, c) { }
    explicit Item(const uint8_t * p_) : Item_base<const uint8_t *>(p_) { }
};

class Item_wr : public Item_base<uint8_t *> {
    void set_key_len(int x) { setint1(p, I2, x); }
public:
    /* Item_wr from block address and offset to item pointer */
    Item_wr(uint8_t * p_, int c) : Item_base<uint8_t *>(p_, c) { }
    explicit Item_wr(uint8_t * p_) : Item_base<uint8_t *>(p_) { }
    void set_component_of(int i) {
	setint2(p, getK(p, I2) + I2 - C2, i);
    }
    void set_components_of(int m) {
	setint2(p, getK(p, I2) + I2, m);
    }
    // Takes size as we may be truncating newkey.
    void set_key_and_block(Key newkey, int truncate_size, uint4 n) {
	int i = truncate_size;
	// Read the length now because we may be copying the key over itself.
	// FIXME that's stupid!  sort this out
	int newkey_len = newkey.length();
	AssertRel(i,<=,newkey_len);
	int newsize = I2 + K1 + i + C2;
	// Item size (BYTES_PER_BLOCK_NUMBER since tag contains block number)
	setint2(p, 0, newsize + BYTES_PER_BLOCK_NUMBER);
	// Key size
	setint1(p, I2, newsize - I2);
	// Copy the main part of the key, possibly truncating.
	std::memmove(p + I2 + K1, newkey.get_address() + K1, i);
	// Copy the count part.
	std::memmove(p + I2 + K1 + i, newkey.get_address() + K1 + newkey_len, C2);
	// Set tag contents to block number
//	set_block_given_by(n);
	setint4(p, newsize, n);
    }

    /** Set this item's tag to point to block n (this block should not be at
     *  level 0).
     */
    void set_block_given_by(uint4 n) {
	setint4(p, size() - BYTES_PER_BLOCK_NUMBER, n);
    }
    void set_size(int l) {
	AssertRel(l,>=,5);
	// We should never be able to pass too large a size here, but don't
	// corrupt the database if this somehow happens.
	if (rare(l &~ CHERT_MAX_ITEM_SIZE))
	    throw Xapian::DatabaseError("item too large!");
	setint2(p, 0, l);
    }
    /** Form an item with a null key and with block number n in the tag.
     */
    void form_null_key(uint4 n) {
	setint4(p, I2 + K1, n);
	set_key_len(K1);        /* null key */
	set_size(I2 + K1 + BYTES_PER_BLOCK_NUMBER);  /* total length */
    }
    void form_key(const std::string & key_) {
	std::string::size_type key_len = key_.length();
	if (key_len > CHERT_BTREE_MAX_KEY_LEN) {
	    // We check term length when a term is added to a document but
	    // chert doubles zero bytes, so this can still happen for terms
	    // which contain one or more zero bytes.
	    std::string msg("Key too long: length was ");
	    msg += str(key_len);
	    msg += " bytes, maximum length of a key is "
		   STRINGIZE(CHERT_BTREE_MAX_KEY_LEN) " bytes";
	    throw Xapian::InvalidArgumentError(msg);
	}

	set_key_len(key_len + K1 + C2);
	std::memmove(p + I2 + K1, key_.data(), key_len);
	set_component_of(1);
    }
    // FIXME passing cd here is icky
    void set_tag(int cd, const char *start, int len, bool compressed) {
	std::memmove(p + cd, start, len);
	set_size(cd + len);
	if (compressed) *p |= 0x80;
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
const int BTREE_CURSOR_LEVELS = 10;

/** Class managing a Btree table in a Chert database.
 *
 *  A table is a store holding a set of key/tag pairs.
 *
 *  A key is used to access a block of data in a chert table.
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
class ChertTable {
    friend class ChertCursor; /* Should probably fix this. */
    private:
	/// Copying not allowed
	ChertTable(const ChertTable &);

	/// Assignment not allowed
	ChertTable & operator=(const ChertTable &);

	/// Return true if there are no entries in the table.
	bool really_empty() const;

    public:
	/** Create a new Btree object.
	 *
	 *  This does not create the table on disk - the create_and_open()
	 *  method must be called to create the table on disk.
	 *
	 *  This also does not open the table - either the create_and_open()
	 *  or open() methods must be called before use is made of the table.
	 *
	 *  @param tablename_   The name of the table (used in changesets).
	 *  @param path_	Path at which the table is stored.
	 *  @param readonly_	whether to open the table for read only access.
	 *  @param compress_strategy_	DONT_COMPRESS, Z_DEFAULT_STRATEGY,
	 *				Z_FILTERED, Z_HUFFMAN_ONLY, or Z_RLE.
	 *  @param lazy		If true, don't create the table until it's
	 *			needed.
	 */
	ChertTable(const char * tablename_, const std::string & path_,
		   bool readonly_, int compress_strategy_ = DONT_COMPRESS,
		   bool lazy = false);

	/** Close the Btree.
	 *
	 *  Any outstanding changes (ie, changes made without commit() having
	 *  subsequently been called) will be lost.
	 */
	~ChertTable();

	/** Close the Btree.  This closes and frees any of the btree
	 *  structures which have been created and opened.
	 *
	 *  @param permanent If true, the Btree will not reopen on demand.
	 */
	void close(bool permanent = false);

	bool readahead_key(const string &key) const;

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
	bool open(chert_revision_number_t revision_);

	/** Return true if this table is open.
	 *
	 *  NB If the table is lazy and doesn't yet exist, returns false.
	 */
	bool is_open() const { return handle >= 0; }

	/** Flush any outstanding changes to the DB file of the table.
	 *
	 *  This must be called before commit, to ensure that the DB file is
	 *  ready to be switched to a new version by the commit.
	 */
	void flush_db();

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
	 *
	 *  @param changes_fd  The file descriptor to write changes to.
	 *	    Defaults to -1, meaning no changes will be written.
	 */
	void commit(chert_revision_number_t revision, int changes_fd = -1,
		    const std::string * changes_tail = NULL);

	/** Append the list of blocks changed to a changeset file.
	 *
	 *  @param changes_fd  The file descriptor to write changes to.
	 */
	void write_changed_blocks(int changes_fd);

	/** Cancel any outstanding changes.
	 *
	 *  This will discard any modifications which haven't been committed
	 *  by calling commit().
	 */
	void cancel();

	/** Read an entry from the table, if and only if it is exactly that
	 *  being asked for.
	 *
	 *  If the key is found in the table, then the tag is copied to @a
	 *  tag.  If the key is not found tag is left unchanged.
	 *
	 *  The result is true iff the specified key is found in the Btree.
	 *
	 *  @param key  The key to look for in the table.
	 *  @param tag  A tag object to fill with the value if found.
	 *
	 *  @return true if key is found in table,
	 *          false if key is not found in table.
	 */
	bool get_exact_entry(const std::string & key, std::string & tag) const;

	/** Check if a key exists in the Btree.
	 *
	 *  This is just like get_exact_entry() except it doesn't read the tag
	 *  value so is more efficient if you only want to check that the key
	 *  exists.
	 *
	 *  @param key  The key to look for in the table.
	 *
	 *  @return true if key is found in table,
	 *          false if key is not found in table.
	 */
	bool key_exists(const std::string &key) const;

	/** Read the tag value for the key pointed to by cursor C_.
	 *
	 *  @param keep_compressed  Don't uncompress the tag - e.g. useful
	 *			    if it's just being opaquely copied.
	 *
	 *  @return	true if current_tag holds compressed data (always
	 *		false if keep_compressed was false).
	 */
	bool read_tag(Cursor * C_, std::string *tag, bool keep_compressed) const;

	/** Add a key/tag pair to the table, replacing any existing pair with
	 *  the same key.
	 *
	 *  If an error occurs during the operation, an exception will be
	 *  thrown.
	 *
	 *  If key is empty, then the null item is replaced.
	 *
	 *  e.g.    btree.add("TODAY", "Mon 9 Oct 2000");
	 *
	 *  @param key   The key to store in the table.
	 *  @param tag   The tag to store in the table.
	 *  @param already_compressed	true if tag is already compressed,
	 *		for example because it is being opaquely copied
	 *		(default: false).
	 */
	void add(const std::string &key, std::string tag, bool already_compressed = false);

	/** Delete an entry from the table.
	 *
	 *  The entry will be removed from the table, if it exists.  If
	 *  it does not exist, no action will be taken.  The item with
	 *  an empty key can't be removed, and false is returned.
	 *
	 *  If an error occurs during the operation, this will be signalled
	 *  by an exception.
	 *
	 *  e.g.    bool deleted = btree.del("TODAY")
	 *
	 *  @param key   The key to remove from the table.
	 *
	 *  @return true if an entry was removed; false if it did not exist.
	 */
	bool del(const std::string &key);

	/// Erase this table from disk.
	void erase();

	/** Set the block size.
	 *
	 *  It's only safe to do this before the table is created.
	 */
	void set_block_size(unsigned int block_size_);

	/** Get the block size.
	 */
	unsigned int get_block_size() const { return block_size; }

	/** Create a new empty btree structure on disk and open it at the
	 *  initial revision.
	 *
	 *  The table must be writable - it doesn't make sense to create
	 *  a table that is read-only!
	 *
	 *  The block size must be less than 64K, where K = 1024. It is unwise
	 *  to use a small block size (less than 1024 perhaps), so we enforce a
	 *  minimum block size of 2K.
	 *
	 *  Example:
	 *
	 *    Btree btree("X-");
	 *    btree.create_and_open(8192);
	 *    // Files will be X-DB, X-baseA (and X-baseB).
	 *
	 *  @param blocksize     - Size of blocks to use.
	 *
	 *  @exception Xapian::DatabaseCreateError if the table can't be
	 *	created.
	 *  @exception Xapian::InvalidArgumentError if the requested blocksize
	 *	is unsuitable.
	 */
	void create_and_open(unsigned int blocksize);

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
	chert_revision_number_t get_latest_revision_number() const {
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
	chert_revision_number_t get_open_revision_number() const {
	    return revision_number;
	}

	/** Return a count of the number of entries in the table.
	 *
	 *  The count does not include the ever-present item with null key.
	 *
	 *  Use @a empty() if you only want to know if the table is empty or
	 *  not.
	 *
	 *  @return The number of entries in the table.
	 */
	chert_tablesize_t get_entry_count() const {
	    return item_count;
	}

	/// Return true if there are no entries in the table.
	bool empty() const {
	    // Prior to 1.1.4/1.0.18, item_count was stored in 32 bits, so we
	    // can't trust it as there could be more than 1<<32 entries.
	    //
	    // In theory it should wrap, so if non-zero the table isn't empty,
	    // but the table this was first noticed in wasn't off by a multiple
	    // of 1<<32.

	    // An empty table will always have level == 0, and most non-empty
	    // tables will have more levels, so use that as a short-cut.
	    return (level == 0) && really_empty();
	}

	/** Get a cursor for reading from the table.
	 *
	 *  The cursor is owned by the caller - it is the caller's
	 *  responsibility to ensure that it is deleted.
	 */
	ChertCursor * cursor_get() const;

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
	    if (block_capacity > BLOCK_CAPACITY) block_capacity = BLOCK_CAPACITY;
	    max_item_size = (block_size - DIR_START - block_capacity * D2)
		/ block_capacity;
	    // Make sure we don't exceed the limit imposed by the format.
	    if (max_item_size > CHERT_MAX_ITEM_SIZE)
		max_item_size = CHERT_MAX_ITEM_SIZE;
	}

	/// Throw an exception indicating that the database is closed.
	XAPIAN_NORETURN(static void throw_database_closed());

	string get_path() const {
	    return name;
	}

    protected:

	/** Perform the opening operation to read.
	 *
	 *  Return true iff the open succeeded.
	 */
	bool do_open_to_read(bool revision_supplied, chert_revision_number_t revision_);

	/** Perform the opening operation to write.
	 *
	 *  Return true iff the open succeeded.
	 */
	bool do_open_to_write(bool revision_supplied,
			      chert_revision_number_t revision_,
			      bool create_db = false);
	bool basic_open(bool revision_supplied, chert_revision_number_t revision);

	bool find(Cursor *) const;
	int delete_kt();
	void read_block(uint4 n, uint8_t *p) const;
	void write_block(uint4 n, const uint8_t *p) const;
	XAPIAN_NORETURN(void set_overwritten() const);
	void block_to_cursor(Cursor *C_, int j, uint4 n) const;
	void alter();
	void compact(uint8_t *p);
	void enter_key(int j, Key prevkey, Key newkey);
	int mid_point(uint8_t *p) const;
	void add_item_to_block(uint8_t *p, Item_wr kt, int c);
	void add_item(Item_wr kt, int j);
	void delete_item(int j, bool repeatedly);
	int add_kt(bool found);
	void read_root();
	void split_root(uint4 split_n);
	void form_key(const std::string & key) const;

	char other_base_letter() const {
	   return (base_letter == 'A') ? 'B' : 'A';
	}

	/// The name of the table (used when writing changesets).
	const char * tablename;

	/// Allocate the zstream for deflating, if not already allocated.
	void lazy_alloc_deflate_zstream() const;

	/// Allocate the zstream for inflating, if not already allocated.
	void lazy_alloc_inflate_zstream() const;

	/** revision number of the opened B-tree. */
	chert_revision_number_t revision_number;

	/** keeps a count of the number of items in the B-tree. */
	chert_tablesize_t item_count;

	/** block size of the B tree in bytes */
	unsigned int block_size;

	/** Revision number of the other base, or zero if there is only one
	 *  base file.
	 */
	mutable chert_revision_number_t latest_revision_number;

	/** set to true if baseA and baseB both exist as valid bases.
	 *
	 *  The unused base is deleted as soon as a write to the Btree takes
	 *  place. */
	mutable bool both_bases;

	/** the value 'A' or 'B' of the current base */
	char base_letter;

	/** true if the root block is faked (not written to disk).
	 * false otherwise.  This is true when the btree hasn't been
	 * modified yet.
	 */
	bool faked_root_block;

	/** true iff the data has been written in a single write in
	 * sequential order.
	 */
	bool sequential;

	/** File descriptor of the table.
	 *
	 *  If the table is lazily created and doesn't yet exist, this will be
	 *  -1.
	 *
	 *  If close() has been called, this will be -2.
	 */
	int handle;

	/// number of levels, counting from 0
	int level;

	/// the root block of the B-tree
	uint4 root;

	/// buffer of size block_size for making up key-tag items
	mutable Item_wr kt;

	/// buffer of size block_size for reforming blocks
	uint8_t * buffer;

	/// For writing back as file baseA or baseB.
	ChertTable_base base;

	/// The path name of the B tree.
	std::string name;

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

	/// Flag for tracking when cursors need to rebuild.
	mutable bool cursor_created_since_last_modification;

	/// Version count for tracking when cursors need to rebuild.
	unsigned long cursor_version;

	/* B-tree navigation functions */
	bool prev(Cursor *C_, int j) const {
	    if (sequential) return prev_for_sequential(C_, j);
	    return prev_default(C_, j);
	}

	bool next(Cursor *C_, int j) const {
	    if (sequential) return next_for_sequential(C_, j);
	    return next_default(C_, j);
	}

	/* Default implementations. */
	bool prev_default(Cursor *C_, int j) const;
	bool next_default(Cursor *C_, int j) const;

	/* Implementations for sequential mode. */
	bool prev_for_sequential(Cursor *C_, int dummy) const;
	bool next_for_sequential(Cursor *C_, int dummy) const;

	static int find_in_block(const uint8_t * p, Key key, bool leaf, int c);

	/** block_given_by(p, c) finds the item at block address p, directory
	 *  offset c, and returns its tag value as an integer.
	 */
	static uint4 block_given_by(const uint8_t * p, int c);

	mutable Cursor C[BTREE_CURSOR_LEVELS];

	/** Buffer used when splitting a block.
	 *
	 *  This buffer holds the split off part of the block.  It's only used
	 *  when updating (in ChertTable::add_item().
	 */
	uint8_t * split_p;

	/** DONT_COMPRESS or Z_DEFAULT_STRATEGY, Z_FILTERED, Z_HUFFMAN_ONLY,
	 *  Z_RLE. */
	int compress_strategy;

	/// Zlib state object for deflating
	mutable z_stream *deflate_zstream;

	/// Zlib state object for inflating
	mutable z_stream *inflate_zstream;

	/// If true, don't create the table until it's needed.
	bool lazy;

	/// Last block readahead_key() preread.
	mutable uint4 last_readahead;

	/* Debugging methods */
//	void report_block_full(int m, int n, const uint8_t * p);
};

#endif /* OM_HGUARD_CHERT_TABLE_H */
