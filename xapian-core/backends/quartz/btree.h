/* btree.h: Btree implementation
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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
#include "btree_types.h"
#include "btree_base.h"
#include "bcursor.h"

#define BTREE_CURSOR_LEVELS 10
    /* allow for this many levels in the B-tree. Overflow practically impossible */

class Btree {
    public:
	/** Constructor, to set important elements to 0.
	 */
	Btree();

	~Btree();

	/** Open the btree to read at the latest revision
	 *
	 * @return true if the open succeeded.
	 */
	bool open_to_read(const char *name_);

	/** Open the btree to read at a given revision
	 *
	 * @return true if the open succeeded.
	 */
	bool open_to_read(const char *name_, uint4 revision_);

	/** Open the btree to write at the latest revision
	 *
	 * @return true if the open succeeded.
	 */
	bool open_to_write(const char *name_);

	/** Open the btree to write at a given revision
	 *
	 * @return true if the open succeeded.
	 */
	bool open_to_write(const char *name_, uint4 revision_);

	/** Attempt to commit changes to disk.  Returns
	 *  BTREE_ERRORS_NONE if successful, otherwise an error.
	 *  The object should be deleted after this operation.
	 */
	Btree_errors commit(uint4 revision);

	/** Create an initial btree structure on disk */
	static int create(const char *name_, int blocksize);

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

	/* keeps a count of the number of items in the B-tree. */
	int4 item_count;

	/* the largest possible value of a key_len. */
	int max_key_len;

	/* 'semi-public': the user might be allowed to read this */

	/** block size of the B tree in bytes */
	int block_size;

	/** the value 'A' or 'B' of the current base */
	int base_letter;

	/** the last used block of B->bit_map0 */
	int4 last_block;

	// FIXME: make these private once operations are member functions.
//    private:

	/** Perform the opening operation to read.
	 *
	 * Return true if the open succeeded.
	 */
	bool do_open_to_read(const char *name_,
			     bool revision_supplied,
			     uint4 revision_);

	/** Perform the opening operation to read.
	 *
	 * Return false if the open succeeded.
	 */
	bool do_open_to_write(const char *name_,
			     bool revision_supplied,
			     uint4 revision_);
	bool basic_open(const char *name_,
			bool revision_supplied,
			uint4 revision);
	/** true if the root block is faked (not written to disk).
	 *  false otherwise.  This is true when the btree hasn't been modified yet.
	 */
	bool faked_root_block;

	int handle;           /* corresponding file handle */
	int level;            /* number of levels, counting from 0 */
	int4 root;            /* the root block of the B-tree */
	byte * kt;            /* buffer of size B->block_size for making up key-tag items */
	byte * buffer;        /* buffer of size block_size for reforming blocks */
	uint4 next_revision;  /* 1 + revision number of the opened B-tree */
	int bit_map_size;     /* size of the bit map of blocks, in bytes */
	int bit_map_low;      /* byte offset into the bit map below which there
				 are no free blocks */
	byte * bit_map0;      /* the initial state of the bit map of blocks: 1 means in
				 use, 0 means free */
	byte * bit_map;       /* the current state of the bit map of blocks */
	Btree_base base;          /* for writing back as file baseA or baseB */
	char other_base_letter;/* - and the value 'B' or 'A' of the next base */

	std::string name;     /* The path name of the B tree */

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

	int (* prev)(struct Btree *, struct Cursor *, int);
	int (* next)(struct Btree *, struct Cursor *, int);

	/* B-tree navigation functions */

	struct Cursor C[BTREE_CURSOR_LEVELS];
};

extern int Btree_find_key(struct Btree * B, byte * key, int key_len);
extern struct Btree_item * Btree_item_create();
extern int Btree_find_tag(struct Btree * B, byte * key, int key_len, struct Btree_item * t);
extern void Btree_item_lose(struct Btree_item * kt);
extern int Btree_add(struct Btree * B, byte * key, int key_len,
                                       byte * tag, int tag_len);
extern int Btree_delete(struct Btree * B, byte * key, int key_len);
extern struct Btree * Btree_open_to_write(const char * name);
extern struct Btree * Btree_open_to_write_revision(const char * name, unsigned long revision);
extern void Btree_quit(struct Btree * B);
extern int Btree_close(struct Btree * B, unsigned long revision);
extern int Btree_create(const char * name, int block_size);
extern void Btree_check(const char * name, const char * opt_string);
extern struct Btree * Btree_open_to_read(const char * name);
extern struct Btree * Btree_open_to_read_revision(const char * name, unsigned long revision);
extern struct Bcursor * Bcursor_create(struct Btree * B);
extern int Bcursor_find_key(struct Bcursor * BC, byte * key, int key_len);
extern int Bcursor_next(struct Bcursor * BC);
extern int Bcursor_prev(struct Bcursor * BC);
extern int Bcursor_get_key(struct Bcursor * BC, struct Btree_item * kt);
extern int Bcursor_get_tag(struct Bcursor * BC, struct Btree_item * kt);
extern void Bcursor_lose(struct Bcursor * BC);
extern void Btree_full_compaction(struct Btree * B, int parity);

#endif /* OM_HGUARD_BTREE_H */

