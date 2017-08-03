/** @file glass_table.h
 * @brief Btree implementation
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2012,2013,2014,2015,2016 Olly Betts
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

#ifndef OM_HGUARD_GLASS_TABLE_H
#define OM_HGUARD_GLASS_TABLE_H

#include <xapian/constants.h>
#include <xapian/error.h>

#include "glass_freelist.h"
#include "glass_cursor.h"
#include "glass_defs.h"

#include "io_utils.h"
#include "noreturn.h"
#include "omassert.h"
#include "str.h"
#include "stringutils.h"
#include "wordaccess.h"

#include "common/compression_stream.h"

#include <algorithm>
#include <string>

namespace Glass {

/** Even for items of at maximum size, it must be possible to get this number of
 *  items in a block */
const size_t BLOCK_CAPACITY = 4;

/** The largest possible value of a key_len.
 *
 *  This gives the upper limit of the size of a key that may be stored in the
 *  B-tree.
 */
#define GLASS_BTREE_MAX_KEY_LEN 255

// FIXME: This named constant probably isn't used everywhere it should be...
const int BYTES_PER_BLOCK_NUMBER = 4;

/*  The B-tree blocks have a number of internal lengths and offsets held in 1, 2
    or 4 bytes. To make the coding a little clearer,
       we use  for
       ------  ---
       K1      the 1 byte length of key
       I2      the 2 byte length of an item (key-tag pair)
       D2      the 2 byte offset to the item from the directory
       X2      the 2 byte component counter that ends each key
*/

const int K1 = 1;
const int I2 = 2;
const int D2 = 2;
const int X2 = 2;

/*  and when getting or setting them, we use these methods of the various
 *  *Item* classes: */

// getD(p, c)
// setD(p, c, x)
// getI()
// setI(x)
// getX(p, c)
// setX(p, c, x)

/* if you've been reading the comments from the top, the next four procedures
   will not cause any headaches.

   Recall that a leaf item has this form:

	   i k     x
	   | |     |
	   I K key X tag
	       ←K→
	   <---SIZE---->
	       <---I--->

   Except that X is omitted for the first component of a tag (there is a flag
   bit in the upper bits of I which indicates these).

   item_of(p, c) returns i, the address of the item at block address p,
   directory offset c,

   component_of(p, c) returns the number marked 'x' above,

   last_component(p, c) returns true if this is a final component.
*/

inline uint4 REVISION(const byte * b) { return aligned_read4(b); }
inline int GET_LEVEL(const byte * b) { return b[4]; }
inline int MAX_FREE(const byte * b) { return unaligned_read2(b + 5); }
inline int TOTAL_FREE(const byte * b) { return unaligned_read2(b + 7); }
inline int DIR_END(const byte * b) { return unaligned_read2(b + 9); }
const int DIR_START = 11;

inline void SET_REVISION(byte * b, uint4 rev) { aligned_write4(b, rev); }
inline void SET_LEVEL(byte * b, int x) { AssertRel(x,<,256); b[4] = x; }
inline void SET_MAX_FREE(byte * b, int x) { unaligned_write2(b + 5, x); }
inline void SET_TOTAL_FREE(byte * b, int x) { unaligned_write2(b + 7, x); }
inline void SET_DIR_END(byte * b, int x) { unaligned_write2(b + 9, x); }

// The item size is stored in 2 bytes, but the top bit is used to store a flag for
// "is the tag data compressed" and the next two bits are used to flag if this is the
// first and/or last item for this tag.
const int I_COMPRESSED_BIT = 0x80;
const int I_LAST_BIT = 0x40;
const int I_FIRST_BIT = 0x20;

const int I_MASK = (I_COMPRESSED_BIT|I_LAST_BIT|I_FIRST_BIT);

const int ITEM_SIZE_MASK = (0xffff &~ (I_MASK << 8));
const size_t MAX_ITEM_SIZE = (ITEM_SIZE_MASK + 3);

/** Freelist blocks have their level set to LEVEL_FREELIST. */
const int LEVEL_FREELIST = 254;

class RootInfo;

class Key {
    const byte *p;
public:
    explicit Key(const byte * p_) : p(p_) { }
    const byte * get_address() const { return p; }
    const byte * data() const { return p + K1; }
    void read(std::string * key) const {
	key->assign(reinterpret_cast<const char *>(p + K1), length());
    }
    int length() const {
	return p[0];
    }
    char operator[](size_t i) const {
	AssertRel(i,<,(size_t)length());
	return p[i + K1];
    }
};

// LeafItem_wr wants to be "LeafItem with non-const p and more methods" - we can't
// achieve that nicely with inheritance, so we use a template base class.
template <class T> class LeafItem_base {
protected:
    T p;
    int get_key_len() const { return p[I2]; }
    static int getD(const byte * q, int c) {
	AssertRel(c, >=, DIR_START);
	AssertRel(c, <, 65535);
	Assert((c & 1) == 1);
	return unaligned_read2(q + c);
    }
    int getI() const { return unaligned_read2(p); }
    static int getX(const byte * q, int c) { return unaligned_read2(q + c); }
public:
    /* LeafItem from block address and offset to item pointer */
    LeafItem_base(T p_, int c) : p(p_ + getD(p_, c)) { }
    explicit LeafItem_base(T p_) : p(p_) { }
    T get_address() const { return p; }
    /** SIZE in diagram above. */
    int size() const {
	return (getI() & ITEM_SIZE_MASK) + 3;
    }
    bool get_compressed() const { return *p & I_COMPRESSED_BIT; }
    bool first_component() const { return *p & I_FIRST_BIT; }
    bool last_component() const { return *p & I_LAST_BIT; }
    int component_of() const {
	if (first_component()) return 1;
	return getX(p, get_key_len() + I2 + K1);
    }
    Key key() const { return Key(p + I2); }
    void append_chunk(std::string * tag) const {
	// Offset to the start of the tag data.
	int cd = get_key_len() + I2 + K1;
	if (!first_component()) cd += X2;
	// Number of bytes to extract from current component.
	int l = size() - cd;
	const char * chunk = reinterpret_cast<const char *>(p + cd);
	tag->append(chunk, l);
    }
    bool decompress_chunk(CompressionStream& comp_stream, string& tag) const {
	// Offset to the start of the tag data.
	int cd = get_key_len() + I2 + K1;
	if (!first_component()) cd += X2;
	// Number of bytes to extract from current component.
	int l = size() - cd;
	const char * chunk = reinterpret_cast<const char *>(p + cd);
	return comp_stream.decompress_chunk(chunk, l, tag);
    }
};

class LeafItem : public LeafItem_base<const byte *> {
public:
    /* LeafItem from block address and offset to item pointer */
    LeafItem(const byte * p_, int c) : LeafItem_base<const byte *>(p_, c) { }
    explicit LeafItem(const byte * p_) : LeafItem_base<const byte *>(p_) { }
};

class LeafItem_wr : public LeafItem_base<byte *> {
    void set_key_len(int x) {
	AssertRel(x, >=, 0);
	AssertRel(x, <=, GLASS_BTREE_MAX_KEY_LEN);
	p[I2] = x;
    }
    void setI(int x) { unaligned_write2(p, x); }
    static void setX(byte * q, int c, int x) { unaligned_write2(q + c, x); }
public:
    /* LeafItem_wr from block address and offset to item pointer */
    LeafItem_wr(byte * p_, int c) : LeafItem_base<byte *>(p_, c) { }
    explicit LeafItem_wr(byte * p_) : LeafItem_base<byte *>(p_) { }
    void set_component_of(int i) {
	AssertRel(i,>,1);
	*p &=~ I_FIRST_BIT;
	setX(p, get_key_len() + I2 + K1, i);
    }
    void set_size(int new_size) {
	AssertRel(new_size,>=,3);
	int I = new_size - 3;
	// We should never be able to pass too large a size here, but don't
	// corrupt the database if this somehow happens.
	if (rare(I &~ ITEM_SIZE_MASK)) throw Xapian::DatabaseError("item too large!");
	setI(I);
    }
    void form_key(const std::string & key_) {
	std::string::size_type key_len = key_.length();
	if (key_len > GLASS_BTREE_MAX_KEY_LEN) {
	    // We check term length when a term is added to a document but
	    // glass doubles zero bytes, so this can still happen for terms
	    // which contain one or more zero bytes.
	    std::string msg("Key too long: length was ");
	    msg += str(key_len);
	    msg += " bytes, maximum length of a key is "
		   STRINGIZE(GLASS_BTREE_MAX_KEY_LEN) " bytes";
	    throw Xapian::InvalidArgumentError(msg);
	}

	set_key_len(key_len);
	std::memmove(p + I2 + K1, key_.data(), key_len);
	*p |= I_FIRST_BIT;
    }
    // FIXME passing cd here is icky
    void set_tag(int cd, const char *start, int len, bool compressed, int i, int m) {
	std::memmove(p + cd, start, len);
	set_size(cd + len);
	if (compressed) *p |= I_COMPRESSED_BIT;
	if (i == m) *p |= I_LAST_BIT;
	if (i == 1) {
	    *p |= I_FIRST_BIT;
	} else {
	    set_component_of(i);
	}
    }
    void fake_root_item() {
	set_key_len(0);   // null key length
	set_size(I2 + K1);   // length of the item
	*p |= I_FIRST_BIT|I_LAST_BIT;
    }
    operator const LeafItem() const { return LeafItem(p); }
    static void setD(byte * q, int c, int x) {
	AssertRel(c, >=, DIR_START);
	AssertRel(c, <, 65535);
	Assert((c & 1) == 1);
	unaligned_write2(q + c, x);
    }
};

/* A branch item has this form:

		 k     x
		 |     |
	     tag K key X
	     ←B→   ←K→
	     <--SIZE--->

	     B = BYTES_PER_BLOCK_NUMBER

   We can't omit X here, as we've nowhere to store the first and last bit
   flags which we have in leaf items.
*/

// BItem_wr wants to be "BItem with non-const p and more methods" - we can't
// achieve that nicely with inheritance, so we use a template base class.
template <class T> class BItem_base {
protected:
    T p;
    int get_key_len() const { return p[BYTES_PER_BLOCK_NUMBER]; }
    static int getD(const byte * q, int c) {
	AssertRel(c, >=, DIR_START);
	AssertRel(c, <, 65535);
	Assert((c & 1) == 1);
	return unaligned_read2(q + c);
    }
    static int getX(const byte * q, int c) { return unaligned_read2(q + c); }
public:
    /* BItem from block address and offset to item pointer */
    BItem_base(T p_, int c) : p(p_ + getD(p_, c)) { }
    explicit BItem_base(T p_) : p(p_) { }
    T get_address() const { return p; }
    /** SIZE in diagram above. */
    int size() const {
	return get_key_len() + K1 + X2 + BYTES_PER_BLOCK_NUMBER;
    }
    Key key() const { return Key(p + BYTES_PER_BLOCK_NUMBER); }
    /** Get this item's tag as a block number (this block should not be at
     *  level 0).
     */
    uint4 block_given_by() const {
	return unaligned_read4(p);
    }
    int component_of() const {
	return getX(p, get_key_len() + BYTES_PER_BLOCK_NUMBER + K1);
    }
};

class BItem : public BItem_base<const byte *> {
public:
    /* BItem from block address and offset to item pointer */
    BItem(const byte * p_, int c) : BItem_base<const byte *>(p_, c) { }
    explicit BItem(const byte * p_) : BItem_base<const byte *>(p_) { }
};

class BItem_wr : public BItem_base<byte *> {
    void set_key_len(int x) {
	AssertRel(x, >=, 0);
	AssertRel(x, <, GLASS_BTREE_MAX_KEY_LEN);
	p[BYTES_PER_BLOCK_NUMBER] = x;
    }
    static void setX(byte * q, int c, int x) { unaligned_write2(q + c, x); }
public:
    /* BItem_wr from block address and offset to item pointer */
    BItem_wr(byte * p_, int c) : BItem_base<byte *>(p_, c) { }
    explicit BItem_wr(byte * p_) : BItem_base<byte *>(p_) { }
    void set_component_of(int i) {
	setX(p, get_key_len() + BYTES_PER_BLOCK_NUMBER + K1, i);
    }
    void set_key_and_block(Key newkey, uint4 n) {
	int len = newkey.length() + K1 + X2;
	// Copy the key size, main part of the key and the count part.
	std::memcpy(p + BYTES_PER_BLOCK_NUMBER, newkey.get_address(), len);
	// Set tag contents to block number
	set_block_given_by(n);
    }
    // Takes size as we may be truncating newkey.
    void set_truncated_key_and_block(Key newkey, int new_comp,
				     int truncate_size, uint4 n) {
	int i = truncate_size;
	AssertRel(i,<=,newkey.length());
	// Key size
	set_key_len(i);
	// Copy the main part of the key, possibly truncating.
	std::memcpy(p + BYTES_PER_BLOCK_NUMBER + K1, newkey.get_address() + K1, i);
	// Set the component count.
	setX(p, BYTES_PER_BLOCK_NUMBER + K1 + i, new_comp);
	// Set tag contents to block number
	set_block_given_by(n);
    }

    /** Set this item's tag to point to block n (this block should not be at
     *  level 0).
     */
    void set_block_given_by(uint4 n) {
	unaligned_write4(p, n);
    }
    /** Form an item with a null key and with block number n in the tag.
     */
    void form_null_key(uint4 n) {
	set_block_given_by(n);
	set_key_len(0);        /* null key */
	set_component_of(0);
    }
    operator const BItem() const { return BItem(p); }
    static void setD(byte * q, int c, int x) {
	AssertRel(c, >=, DIR_START);
	AssertRel(c, <, 65535);
	Assert((c & 1) == 1);
	unaligned_write2(q + c, x);
    }
};

// Allow for BTREE_CURSOR_LEVELS levels in the B-tree.
// With 10, overflow is practically impossible
// FIXME: but we want it to be completely impossible...
const int BTREE_CURSOR_LEVELS = 10;

}

using Glass::RootInfo;

class GlassChanges;

/** Class managing a Btree table in a Glass database.
 *
 *  A table is a store holding a set of key/tag pairs.
 *
 *  A key is used to access a block of data in a glass table.
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
class GlassTable {
    friend class GlassCursor; /* Should probably fix this. */
    friend class GlassFreeList;
    private:
	/// Copying not allowed
	GlassTable(const GlassTable &);

	/// Assignment not allowed
	GlassTable & operator=(const GlassTable &);

	void basic_open(const RootInfo * root_info,
			glass_revision_number_t rev);

	/** Perform the opening operation to read. */
	void do_open_to_read(const RootInfo * root_info,
			     glass_revision_number_t rev);

	/** Perform the opening operation to write. */
	void do_open_to_write(const RootInfo * root_info,
			      glass_revision_number_t rev = 0);

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
	 *  @param lazy		If true, don't create the table until it's
	 *			needed.
	 */
	GlassTable(const char * tablename_, const std::string & path_,
		   bool readonly_, bool lazy = false);

	GlassTable(const char * tablename_, int fd, off_t offset_,
		   bool readonly_, bool lazy = false);

	/** Close the Btree.
	 *
	 *  Any outstanding changes (ie, changes made without commit() having
	 *  subsequently been called) will be lost.
	 */
	~GlassTable();

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

	/** Open the btree.
	 *
	 *  @param flags_	flags for opening
	 *  @param root_info	root block info
	 *
	 *  @exception Xapian::DatabaseCorruptError will be thrown if the table
	 *	is in a corrupt state.
	 *  @exception Xapian::DatabaseOpeningError will be thrown if the table
	 *	cannot be opened (but is not corrupt - eg, permission problems,
	 *	not present, etc).
	 */
	void open(int flags_, const RootInfo & root_info,
		  glass_revision_number_t rev);

	/** Return true if this table is open.
	 *
	 *  NB If the table is lazy and doesn't yet exist, returns false.
	 */
	bool is_open() const { return handle >= 0; }

	/** Return true if this table is writable. */
	bool is_writable() const { return writable; }

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
	 *          be greater than the current revision number.  FIXME: If
	 *          we support rewinding to a previous revision, maybe this
	 *          needs to be greater than any previously used revision.
	 *
	 *  @param root_info  Information about the root is returned in this.
	 */
	void commit(glass_revision_number_t revision, RootInfo * root_info);

	bool sync() {
	    return (flags & Xapian::DB_NO_SYNC) ||
		   handle < 0 ||
		   io_sync(handle);
	}

	/** Cancel any outstanding changes.
	 *
	 *  This will discard any modifications which haven't been committed
	 *  by calling commit().
	 */
	void cancel(const RootInfo & root_info, glass_revision_number_t rev);

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
	bool read_tag(Glass::Cursor * C_, std::string *tag, bool keep_compressed) const;

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

	int get_flags() const { return flags; }

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
	 *    // File will be "X." + GLASS_TABLE_EXTENSION (i.e. "X.glass")
	 *    Btree btree("X.");
	 *    btree.create_and_open(0, root_info);
	 *
	 *  @param root_info     RootInfo object
	 *
	 *  @exception Xapian::DatabaseCreateError if the table can't be
	 *	created.
	 *  @exception Xapian::InvalidArgumentError if the requested blocksize
	 *	is unsuitable.
	 */
	void create_and_open(int flags_, const RootInfo & root_info);

	void set_full_compaction(bool parity);

	/** Get the revision number at which this table
	 *  is currently open.
	 *
	 *  It is possible that there are other, more recent or older
	 *  revisions available.
	 *
	 *  @return the current revision number.
	 */
	glass_revision_number_t get_open_revision_number() const {
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
	glass_tablesize_t get_entry_count() const {
	    return item_count;
	}

	/// Return true if there are no entries in the table.
	bool empty() const {
	    return (item_count == 0);
	}

	/** Get a cursor for reading from the table.
	 *
	 *  The cursor is owned by the caller - it is the caller's
	 *  responsibility to ensure that it is deleted.
	 */
	GlassCursor * cursor_get() const;

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
	    if (block_capacity > Glass::BLOCK_CAPACITY)
		block_capacity = Glass::BLOCK_CAPACITY;
	    using Glass::DIR_START;
	    using Glass::D2;
	    max_item_size =
		(block_size - DIR_START - block_capacity * D2) / block_capacity;
	    // Make sure we don't exceed the limit imposed by the format.
	    if (max_item_size > Glass::MAX_ITEM_SIZE)
		max_item_size = Glass::MAX_ITEM_SIZE;
	}

	/** Set the GlassChanges object to write changed blocks to.
	 *
	 *  The GlassChanges object is not owned by the table, so the table
	 *  must not delete it.
	 */
	void set_changes(GlassChanges * changes) {
	    changes_obj = changes;
	}

	/// Throw an exception indicating that the database is closed.
	XAPIAN_NORETURN(static void throw_database_closed());

	string get_path() const {
	    return name + GLASS_TABLE_EXTENSION;
	}

    protected:

	bool find(Glass::Cursor *) const;
	int delete_kt();
	void read_block(uint4 n, byte *p) const;
	void write_block(uint4 n, const byte *p, bool appending = false) const;
	XAPIAN_NORETURN(void set_overwritten() const);
	void block_to_cursor(Glass::Cursor *C_, int j, uint4 n) const;
	void alter();
	void compact(byte *p);
	void enter_key_above_leaf(Glass::LeafItem previtem, Glass::LeafItem newitem);
	void enter_key_above_branch(int j, Glass::BItem newitem);
	int mid_point(byte *p) const;
	void add_item_to_leaf(byte *p, Glass::LeafItem kt, int c);
	void add_item_to_branch(byte *p, Glass::BItem kt, int c);
	void add_leaf_item(Glass::LeafItem kt);
	void add_branch_item(Glass::BItem kt, int j);
	void delete_leaf_item(bool repeatedly);
	void delete_branch_item(int j);
	int add_kt(bool found);
	void read_root();
	void split_root(uint4 split_n);
	void form_key(const std::string & key) const;

	/// The name of the table (used when writing changesets).
	const char * tablename;

	/** revision number of the opened B-tree. */
	glass_revision_number_t revision_number;

	/** keeps a count of the number of items in the B-tree. */
	glass_tablesize_t item_count;

	/** block size of the B tree in bytes */
	unsigned int block_size;

	/** Flags like DB_NO_SYNC and DB_DANGEROUS. */
	int flags;

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
	 *  If close() has been called, this will be -2.
	 *
	 *  If the table is lazily created and doesn't yet exist, this will be
	 *  -1 (for a multi-file database) or -3-fd (for a single-file database).
	 */
	int handle;

	/// number of levels, counting from 0
	int level;

	/// the root block of the B-tree
	uint4 root;

	/// buffer of size block_size for making up key-tag items
	mutable Glass::LeafItem_wr kt;

	/// buffer of size block_size for reforming blocks
	byte * buffer;

	/// List of free blocks.
	GlassFreeList free_list;

	/** The path name of the B tree.
	 *
	 *  For a single-file database, this will be empty.
	 */
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

	/** The GlassChanges object to write block changes to.
	 *
	 *  If NULL, no changes will be written.
	 */
	GlassChanges * changes_obj;

	bool single_file() const {
	    return name.empty();
	}

	/* B-tree navigation functions */
	bool prev(Glass::Cursor *C_, int j) const {
	    if (sequential && !single_file())
		return prev_for_sequential(C_, j);
	    return prev_default(C_, j);
	}

	bool next(Glass::Cursor *C_, int j) const {
	    if (sequential) return next_for_sequential(C_, j);
	    return next_default(C_, j);
	}

	/* Default implementations. */
	bool prev_default(Glass::Cursor *C_, int j) const;
	bool next_default(Glass::Cursor *C_, int j) const;

	/* Implementations for sequential mode. */
	bool prev_for_sequential(Glass::Cursor *C_, int dummy) const;
	bool next_for_sequential(Glass::Cursor *C_, int dummy) const;

	static int find_in_leaf(const byte * p, Glass::LeafItem item, int c, bool& exact);
	static int find_in_branch(const byte * p, Glass::LeafItem item, int c);
	static int find_in_branch(const byte * p, Glass::BItem item, int c);

	/** block_given_by(p, c) finds the item at block address p, directory
	 *  offset c, and returns its tag value as an integer.
	 */
	static uint4 block_given_by(const byte * p, int c);

	mutable Glass::Cursor C[Glass::BTREE_CURSOR_LEVELS];

	/** Buffer used when splitting a block.
	 *
	 *  This buffer holds the split off part of the block.  It's only used
	 *  when updating (in GlassTable::add_item().
	 */
	byte * split_p;

	/** Minimum size tag to try compressing (0 for no compression). */
	uint4 compress_min;

	mutable CompressionStream comp_stream;

	/// If true, don't create the table until it's needed.
	bool lazy;

	/// Last block readahead_key() preread.
	mutable uint4 last_readahead;

	/// offset to start of table in file.
	off_t offset;

	/* Debugging methods */
//	void report_block_full(int m, int n, const byte * p);
};

namespace Glass {

/** Compare two items by their keys.

   The result is negative if a precedes b, positive is b precedes a, and
   0 if a and b are equal.  The comparison is for byte sequence collating
   order, taking lengths into account. So if the keys are made up of lower case
   ASCII letters we get alphabetical ordering.

   Now remember that items are added into the B-tree in fastest time
   when they are preordered by their keys. This is therefore the piece
   of code that needs to be followed to arrange for the preordering.

   Note that keys have two parts - a string value and a "component_of" count.
   If the string values are equal, the comparison is made based on
   "component_of".
*/

template<typename ITEM1, typename ITEM2>
int compare(ITEM1 a, ITEM2 b)
{
    Key key1 = a.key();
    Key key2 = b.key();
    const byte* p1 = key1.data();
    const byte* p2 = key2.data();
    int key1_len = key1.length();
    int key2_len = key2.length();
    int k_smaller = (key2_len < key1_len ? key2_len : key1_len);

    // Compare the common part of the keys.
    int diff = std::memcmp(p1, p2, k_smaller);
    if (diff == 0) {
	// If the common part matches, compare the lengths.
	diff = key1_len - key2_len;
	if (diff == 0) {
	    // If the strings match, compare component_of().
	    diff = a.component_of() - b.component_of();
	}
    }
    return diff;
}

/** Compare two BItem objects by their keys.
 *
 *  Specialisation for comparing two BItems, where component_of is always
 *  explicitly stored.
 */
inline int
compare(BItem a, BItem b)
{
    Key key1 = a.key();
    Key key2 = b.key();
    const byte* p1 = key1.data();
    const byte* p2 = key2.data();
    int key1_len = key1.length();
    int key2_len = key2.length();
    if (key1_len == key2_len) {
	// The keys are the same length, so we can compare the counts in the
	// same operation since they're stored as 2 byte bigendian numbers.
	int len = key1_len + X2;
	return std::memcmp(p1, p2, len);
    }

    int k_smaller = (key2_len < key1_len ? key2_len : key1_len);

    // Compare the common part of the keys.
    int diff = std::memcmp(p1, p2, k_smaller);
    if (diff == 0) {
	// If the common part matches, compare the lengths.
	diff = key1_len - key2_len;
	// We dealt with the "same length" case above so we never need to check
	// component_of here.
    }
    return diff;
}

}

#endif /* OM_HGUARD_GLASS_TABLE_H */
