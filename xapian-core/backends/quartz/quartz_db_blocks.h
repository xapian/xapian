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

#include <map>

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
};

class QuartzDbBlocks {
    private:
	/// Copying not allowed
	QuartzDbBlocks(const QuartzDbBlocks &);

	/// Assignment not allowed
	void operator=(const QuartzDbBlocks &);

	/// The blocks stored in this object
	map<QuartzDbKey, QuartzDbBlock *> blocks;

	/// Maximum size allowed for the data in a block
	quartz_blocksize_t max_blocksize;
    public:

	/** Construct the blocks.
	 */
	QuartzDbBlocks(const OmSettings & settings,
		       bool use_transactions,
		       bool readonly);

	/** Delete the blocks.
	 */
	~QuartzDbBlocks();

	/** Get a block.  The block returned is 
	 */
	QuartzDbBlock * get_block(const QuartzDbKey &key);
};

#endif /* OM_HGUARD_QUARTZ_DB_BLOCKS_H */
