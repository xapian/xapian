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

// Allow for BTREE_CURSOR_LEVELS levels in the B-tree.
// With 10, overflow is practically impossible
// FIXME: but we want it to be completely impossible...
#define BTREE_CURSOR_LEVELS 10

class Btree {
    friend class Bcursor; /* Should probably fix this. */
    private:
        // Prevent copying
        Btree(const Btree &);
        Btree & operator=(const Btree &);

    public:
	/** Constructor */
	Btree();

	~Btree();

	/** Open the btree to read at the latest revision
	 */
	void open_to_read(const string &name_);

	/** Open the btree to read at a given revision
	 */
	void open_to_read(const string &name_, uint4 revision_);

	/** Open a read-only version of a given Btree
	 */
	void open_to_read(const Btree &);

	/** Update a read-only version of a given Btree to a just committed
	 *  version.  The btree parameter must be the same as that passed
	 *  to earlier calls to open_to_read().
	 */
	void reopen_to_read(const Btree &btree);

	/** Open the btree to write at the latest revision
	 */
	void open_to_write(const string &name_);

	/** Open the btree to write at a given revision
	 *
	 * @return true if the open succeeded.
	 */
	bool open_to_write(const string &name_, uint4 revision_);

	/** Attempt to commit changes to disk.
	 *  The object should be deleted after this operation.
	 */
	void commit(uint4 revision);

	bool find_key(const string &key);
	bool find_tag(const string &key, string * tag);

	bool add(const string &key, const string &tag);
	bool del(const string &key);

	/** Create an initial btree structure on disk */
	static void create(const string &name_, int blocksize);

	/** Erase the btree structure from disk */
	static void erase(const string & tablename);

	void set_full_compaction(bool parity);

	uint4 get_latest_revision_number() const {
	    if (both_bases && other_revision_number > revision_number)
		return other_revision_number;
	    return revision_number;
	}

	/** revision number of the opened B-tree. */
	uint4 revision_number;

	/** keeps a count of the number of items in the B-tree. */
	uint4 item_count;

	/** the largest possible value of a key_len. */
	static const string::size_type max_key_len = 252;

	/* 'semi-public': the user might be allowed to read this */

	/** block size of the B tree in bytes */
	int block_size;

	/** the last used block of B->bit_map0 */
	/*uint4 last_block; */

    protected:

	/** Perform the opening operation to read.
	 */
	void do_open_to_read(const string &name_,
			     bool revision_supplied,
			     uint4 revision_);

	/** Perform the opening operation to read.
	 *
	 *  Return true iff the open succeeded.
	 */
	bool do_open_to_write(const string &name_,
			     bool revision_supplied,
			     uint4 revision_);
	bool basic_open(const string &name_,
			bool revision_supplied,
			uint4 revision);

	bool find(Cursor *);
	int delete_kt();
	void read_block(uint4 n, byte *p);
	void write_block(uint4 n, const byte *p);
	void set_overwritten();
	void block_to_cursor(Cursor *C_, int j, uint4 n);
	void alter();
	void compress(byte *p);
	void enter_key(int j, byte *kq, byte *kp);
	void split_off(int j, int c, byte *p, byte *q);
	int mid_point(byte *p);
	void add_item_to_block(byte *p, byte *kt, int c);
	void add_item(byte *kt, int j);
	void delete_item(int j, bool repeatedly);
	int add_kt(int found);
	void read_root();
	void split_root(int j);
	void make_index_item(byte * result, unsigned int result_len,
			     const byte * prevkey, const byte * newkey,
			     const uint4 blocknumber, bool truncate) const;
	void form_key(const string & key);

	/** revision number of the other base. */
	uint4 other_revision_number;

	/** set to true if baseA and baseB both exist. The old base
	 *  is deleted as soon as a write to the Btree takes place. */
	bool both_bases;

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
	byte * kt;

	/// buffer of size block_size for reforming blocks
	byte * buffer;

	/// 1 + revision number of the opened B-tree
	uint4 next_revision;

	/// for writing back as file baseA or baseB
	Btree_base base;

	/// - and the value 'B' or 'A' of the next base
	char other_base_letter;

	/** set to true if a parallel overwrite is detected. */
	bool overwritten;

	/// The path name of the B tree
	string name;

	/** count of the number of successive instances of purely
	 * sequential addition, starting at SEQ_START_POINT (neg) and
	 * going up to zero */
	int seq_count;

	/** the last block to be changed by an addition */
	uint4 changed_n;

	/** directory offset corresponding to last block to be changed
	 *  by an addition */
	int changed_c;

	/// maximum size of an item (key-tag pair)
	int max_item_size;

	/// set to true the first time the B-tree is written to
	bool Btree_modified;

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

	Cursor C[BTREE_CURSOR_LEVELS];

	/* Debugging methods */
//	void report_block_full(int m, int n, const byte * p);
};

#endif /* OM_HGUARD_BTREE_H */
