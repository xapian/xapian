/* quartz_db_blocks.h: Storage of a set of database blocks from a quartz db
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

#ifndef OM_HGUARD_QUARTZ_DB_BLOCKS_H
#define OM_HGUARD_QUARTZ_DB_BLOCKS_H

#include "config.h"
#ifdef MUS_BUILD_BACKEND_QUARTZ

#include <map>
#include <memory>
#include <string>

/** A block of data in a quartz database.
 */
struct QuartzDbBlock {
    public:
	string value;
};

/** The key used to access a block of data in a quartz database.
 */
struct QuartzDbKey {
    public:
	string value;

	bool operator < (const QuartzDbKey & a) const {return (value<a.value);}
};

class QuartzDbBlocks {
    private:
	/// Copying not allowed
	QuartzDbBlocks(const QuartzDbBlocks &);

	/// Assignment not allowed
	void operator=(const QuartzDbBlocks &);

	/// The blocks stored in this object
	std::map<QuartzDbKey, QuartzDbBlock *> blocks;
    public:

	/** Initialise the cache of blocks.
	 */
	QuartzDbBlocks();

	/** Delete the blocks.
	 */
	~QuartzDbBlocks();

	/** Get a pointer to a block.
	 *  If the block isn't currently in the list of blocks, a null pointer
	 *  will be returned.
	 *
	 *  @param key The key that the block is stored under.
	 *
	 *  @return A pointer to the block.  This is guaranteed not to be
	 *          a null pointer.  The block pointed to is still owned by
	 *          the QuartzDbBlocks object - it may be modifed if
	 *          desired, but the user should not try to free the
	 *          pointer.
	 */
	QuartzDbBlock * get_block(const QuartzDbKey &key);

	/** Set the block associated with a given key.
	 *  If a block is already associated with the key, it is freed and
	 *  replaced.  Any pointers to the old block previously returned by
	 *  get_block will become invalid.
	 *
	 *  To remove a block, the data should have size zero, or be a
	 *  null pointer.
	 *
	 *  @param key   The key to store the block under.
	 *
	 *  @param block The block to store.
	 */
	void set_block(const QuartzDbKey &key,
		       auto_ptr<QuartzDbBlock> data);
};

#endif /* MUS_BUILD_BACKEND_QUARTZ */

#endif /* OM_HGUARD_QUARTZ_DB_BLOCKS_H */
