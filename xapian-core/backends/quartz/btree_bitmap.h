/* btree_bitmap.h: Btree bitmap implementation
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

#ifndef OM_HGUARD_BTREE_BITMAP_H
#define OM_HGUARD_BTREE_BITMAP_H

#include "btree_types.h"
#include "btree_util.h"

class Btree_bitmap {
    public:
	/** Constructor */
	Btree_bitmap(Btree *B_);

	/** Destructor - frees resources. */
	~Btree_bitmap();

	/** Read the bitmap from the file. */
	void read(const std::string &name, char ch, int size);

	/** true iff block n was free at the start of the transaction on
	 *  the B-tree.
	 */
	bool block_free_at_start(int4 n) const;

	void free_block(int4 n);

	int next_free_block();

	bool block_free_now(int4 n);

	/** Returns the current size of the bitmap */
	int get_size() const;

	int write_to_file(const std::string &name);

	int4 get_last_block();
	
	/* Only used with fake root blocks */
	void clear();

	/* Used by Btree::check() */
	bool is_empty() const;
    private:
	void extend();

	/** Read a bitmap from a file
	 * 
	 *  @return	true if the bitmap was successfully read from
	 *  		the file.
	 */
	void read_from_file(const std::string &name, char ch, int size);

	/** size of the bit map of blocks, in bytes */
	int size;

	/** byte offset into the bit map below which there
	   are no free blocks */
	int low;

	/** the initial state of the bit map of blocks: 1 means in
	   use, 0 means free */
	byte *bit_map0;

	/** the current state of the bit map of blocks */
	byte *bit_map;

	/** A pointer back to the btree */
	// FIXME: is this really necessary?
	Btree *B;
};

#endif /* OM_HGUARD_BTREE_BITMAP_H */

