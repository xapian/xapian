/* btree.h: Btree implementation
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Olly Betts
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

#include "autoptr.h"
#include "btree_types.h"
#include "btree_base.h"
#include "bcursor.h"

/* allow for this many levels in the B-tree. Overflow practically impossible */
/* FIXME: but we want it to be completely impossible... */
#define BTREE_CURSOR_LEVELS 10

class Btree {
    friend class Bcursor; /* Should probably fix this. */
    public:
	/** Constructor, to set important elements to 0.
	 */
	Btree();

	~Btree();

	/** Open the btree to read at the latest revision
	 *
	 * @return true if the open succeeded.
	 */
	bool open_to_read(const string &name_);

	/** Open the btree to read at a given revision
	 *
	 * @return true if the open succeeded.
	 */
	bool open_to_read(const string &name_, uint4 revision_);

	/** Open the btree to write at the latest revision
	 *
	 * @return true if the open succeeded.
	 */
	bool open_to_write(const string &name_);

	/** Open the btree to write at a given revision
	 *
	 * @return true if the open succeeded.
	 */
	bool open_to_write(const string &name_, uint4 revision_);

	/** Attempt to commit changes to disk.  Returns
	 *  BTREE_ERRORS_NONE if successful, otherwise an error.
	 *  The object should be deleted after this operation.
	 */
	Btree_errors commit(uint4 revision);

	bool find_key(const string &key);
	bool find_tag(const string &key, struct Btree_item * t);

	bool add(const string &key, const string &tag);
	bool del(const string &key);

	/** Create an initial btree structure on disk */
	static void create(const string &name_, int blocksize);

	/** Erase the btree structure from disk */
	static void erase(const string & tablename);

	void set_full_compaction(int parity);

	static void check(const string & name, const string & opt_string);

	AutoPtr<Bcursor> Bcursor_create();

	/** error number setting */
	Btree_errors error;

	/** set to true if a parallel overwrite is detected. */
	bool overwritten;

	/** revision number of the opened B-tree. */
	uint4 revision_number;

	/** revision number of the other base. */
	uint4 other_revision_number;

	/** set to true if baseA and baseB both exist. The old base
	 *  is deleted as soon as a write to the Btree takes place. */
	bool both_bases;

	/** keeps a count of the number of items in the B-tree. */
	int4 item_count;

	/** the largest possible value of a key_len. */
	string::size_type max_key_len;

	/* 'semi-public': the user might be allowed to read this */

	/** block size of the B tree in bytes */
	int block_size;

	/** the value 'A' or 'B' of the current base */
	int base_letter;

	/** the last used block of B->bit_map0 */
	/*int4 last_block; */

    private:

	/** Perform the opening operation to read.
	 *
	 * Return true if the open succeeded.
	 */
	bool do_open_to_read(const string &name_,
			     bool revision_supplied,
			     uint4 revision_);

	/** Perform the opening operation to read.
	 *
	 * Return false if the open succeeded.
	 */
	bool do_open_to_write(const string &name_,
			     bool revision_supplied,
			     uint4 revision_);
	bool basic_open(const string &name_,
			bool revision_supplied,
			uint4 revision);

	bool find(Cursor *C_);
	int delete_kt();
	void read_block(int4 n, byte *p);
	void write_block(int4 n, const byte *p);
	void set_overwritten();
	void block_to_cursor(struct Cursor *C_, int j, int4 n);
	void alter(struct Cursor *C);
	void compress(byte *p);
	void enter_key(Cursor *C_, int j, byte *kq, byte *kp);
	void split_off(Cursor *C_, int j, int c, byte *p, byte *q);
	int mid_point(byte *p);
	void add_item_to_block(byte *p, byte *kt, int c);
	void add_item(Cursor *C, byte *kt, int j);
	void delete_item(Cursor *C, int j, int repeatedly);
	int add_kt(int found, Cursor *C);
	int write_base();
	void read_root();
	void force_block_to_cursor(Cursor *C_, int j);
	void block_check(Cursor *C, int j, int opts);
	void split_root(struct Cursor * C_, int j);
	void make_index_item(byte * result, unsigned int result_len,
			     const byte * prevkey, const byte * newkey,
			     const int4 blocknumber, bool truncate) const;
	void form_key(const string & key);

	/** true if the root block is faked (not written to disk).  false
	 * otherwise.  This is true when the btree hasn't been modified yet.
	 */
	bool faked_root_block;

	/** true iff the data has been written in a single write in sequential
	 *  order.
	 */
	bool sequential;

	int handle;           /* corresponding file handle */
	int level;            /* number of levels, counting from 0 */
	int4 root;            /* the root block of the B-tree */
	byte * kt;            /* buffer of size B->block_size for making up key-tag items */
	byte * buffer;        /* buffer of size block_size for reforming blocks */
	uint4 next_revision;  /* 1 + revision number of the opened B-tree */
	Btree_base base;      /* for writing back as file baseA or baseB */
	char other_base_letter;/* - and the value 'B' or 'A' of the next base */

	string name;     /* The path name of the B tree */

	/** count of the number of successive instances of purely sequential
	 *  addition, starting at SEQ_START_POINT (neg) and going up to zero */
	int seq_count;

	/** the last block to be changed by an addition */
	int4 changed_n;

	/* - and the corresponding directory offset */
	int changed_c;

	int max_item_size;    /* maximum size of an item (key-tag pair) */
	int shared_level;     /* in B-tree read mode, cursors share blocks in
				 BC->C for levels at or above B->shared_level */
	char Btree_modified;  /* set to true the first time the B-tree is written to */
	char full_compaction; /* set to true when full compaction is to be achieved */

	bool writable; 	/* Set to true when the database is opened to write. */

	int (* prev)(struct Btree *, struct Cursor *, int);
	int (* next)(struct Btree *, struct Cursor *, int);

	static int prev_default(Btree *, Cursor *C, int j);
	static int next_default(Btree *, Cursor *C, int j);

	static int prev_for_sequential(Btree *, Cursor *C, int dummy);
	static int next_for_sequential(Btree *, Cursor *C, int dummy);

	/* B-tree navigation functions */

	struct Cursor C[BTREE_CURSOR_LEVELS];

	/* Debugging methods */
    private:
	void report_block_full(int m, int n, byte * p);
};

extern int Btree_close(struct Btree * B, unsigned long revision);
extern void Btree_full_compaction(struct Btree * B, int parity);

#endif /* OM_HGUARD_BTREE_H */
