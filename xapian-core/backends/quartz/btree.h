/* btree.h: Btree implementation
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004 Olly Betts
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#ifndef OM_HGUARD_BTREE_H
#define OM_HGUARD_BTREE_H

#include <string>
using std::string;

#include "btree_types.h"
#include "btree_base.h"
#include "bcursor.h"
#include "quartz_types.h"

// Allow for BTREE_CURSOR_LEVELS levels in the B-tree.
// With 10, overflow is practically impossible
// FIXME: but we want it to be completely impossible...
#define BTREE_CURSOR_LEVELS 10

class QuartzCursor;

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

	/** Open the btree at a given revision
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

	bool find_key(const string &key) const;
	bool find_tag(const string &key, string * tag) const;

	/** Add an entry to the table.
	 *
	 *  If the key already exists in the table, the existing tag
	 *  is replaced by the supplied one.  If not, a new entry is
	 *  created.
	 *
	 *  If an error occurs during the operation, this will be signalled
	 *  by a return value of false.  All modifications since the
	 *  previous commit() will be lost.
	 *
	 *  @param key   The key to store in the table.
	 *  @param tag   The tag to store in the table.
	 *
	 *  @return true if the operation completed successfully, false
	 *          otherwise.
	 */
	bool add(const string &key, const string &tag);

	/** Delete an entry from the table.
	 *
	 *  The entry will be removed from the table, if it exists.  If
	 *  it does not exist, no action will be taken.
	 *
	 *  If an error occurs during the operation, this will be signalled
	 *  by a return value of false.  All modifications since the
	 *  previous commit() will be lost.
	 *
	 *  @param key   The key to store in the table.
	 *
	 *  @return true if the operation completed successfully, false
	 *          otherwise.
	 */
	bool del(const string &key);

	/** Create an empty btree structure on disk.
	 *
	 *  @param blocksize     - Size of blocks to use.
	 *
	 *  @exception Xapian::DatabaseCreateError if the table can't be created.
	 */
	void create(unsigned int blocksize);

	void set_full_compaction(bool parity);

	/** Get the latest revision number stored in this table.
	 *
	 *  It is possible that there are other, older, revisions of this
	 *  table available, and indeed that the revision currently open
	 *  is one of these older revisions.
	 */
	quartz_revision_number_t get_latest_revision_number() const {
	    if (both_bases && other_revision_number > revision_number)
		return other_revision_number;
	    return revision_number;
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
	QuartzCursor * cursor_get() const;

	/** Determine whether the object contains uncommitted modifications.
	 *
	 *  @return true if there have been modifications since the last
	 *          the last call to commit().
	 */
	bool is_modified() const { return Btree_modified; }

	/** revision number of the opened B-tree. */
	quartz_revision_number_t revision_number;

	/** keeps a count of the number of items in the B-tree. */
	uint4 item_count;

	/** the largest possible value of a key_len. */
	static const string::size_type max_key_len = 252;

	/* 'semi-public': the user might be allowed to read this */

	/** block size of the B tree in bytes */
	unsigned int block_size;

    protected:

	/** Perform the opening operation to read.
	 */
	void do_open_to_read(bool revision_supplied, quartz_revision_number_t revision_);

	/** Perform the opening operation to read.
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
	void compress(byte *p);
	void enter_key(int j, byte *kq, byte *kp);
	int mid_point(byte *p);
	void add_item_to_block(byte *p, byte *kt, int c);
	void add_item(byte *kt, int j);
	void delete_item(int j, bool repeatedly);
	int add_kt(bool found);
	void read_root();
	void split_root(uint4 split_n);
	void make_index_item(byte * result, unsigned int result_len,
			     const byte * prevkey, const byte * newkey,
			     const uint4 blocknumber, bool truncate) const;
	void form_key(const string & key) const;

	/** revision number of the other base. */
	quartz_revision_number_t other_revision_number;

	/** set to true if baseA and baseB both exist. The old base
	 *  is deleted as soon as a write to the Btree takes place. */
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
	mutable byte * kt;

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
	int max_item_size;

	/// Set to true the first time the B-tree is modified.
	mutable bool Btree_modified;

	/// set to true when full compaction is to be achieved
	bool full_compaction;

	/// Set to true when the database is opened to write.
	bool writable;

	/// Set to true if we shouldn't close handle ourselves.
	bool dont_close_handle;

	/* B-tree navigation functions */
	bool prev(Cursor *C_, int j) { return (this->*prev_ptr)(C_, j); }
	bool next(Cursor *C_, int j) { return (this->*next_ptr)(C_, j); }

	bool (Btree::* prev_ptr)(Cursor *, int);
	bool (Btree::* next_ptr)(Cursor *, int);

	bool prev_default(Cursor *C_, int j);
	bool next_default(Cursor *C_, int j);

	bool prev_for_sequential(Cursor *C_, int dummy);
	bool next_for_sequential(Cursor *C_, int dummy);

	static int find_in_block(const byte * p, const byte * key, int offset, int c);

	static int compare_keys(const byte * key1, const byte * key2);

	/** block_given_by(p, c) finds the item at block address p, directory
	 *  offset c, and returns its tag value as an integer.
	 */
	static uint4 block_given_by(const byte * p, int c);

	mutable Cursor C[BTREE_CURSOR_LEVELS];

	/** Buffer used when splitting a block.
	 *
	 *  This buffer holds the split off part of the block.  It's only used
	 *  when updating.
	 */
	byte * split_p;

	/* Debugging methods */
//	void report_block_full(int m, int n, const byte * p);
};

#endif /* OM_HGUARD_BTREE_H */
