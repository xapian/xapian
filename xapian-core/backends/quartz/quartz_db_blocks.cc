/* quartz_db_blocks.cc: Storage of a set of database blocks from a quartz db
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

#include "config.h"

#include "quartz_db_blocks.h"

QuartzDbBlocks::QuartzDbBlocks()
{
}

QuartzDbBlocks::~QuartzDbBlocks()
{
    map<QuartzDbKey, QuartzDbBlock *>::iterator i;
    for (i = blocks.begin(); i != blocks.end(); i++) {
	delete (i->second);
	i->second = 0;
    }
}

QuartzDbBlock *
QuartzDbBlocks::get_block(const QuartzDbKey &key)
{
    map<QuartzDbKey, QuartzDbBlock *>::iterator i = blocks.find(key);
    if (i == blocks.end()) return 0;
    return i->second;
}

void
QuartzDbBlocks::set_block(const QuartzDbKey &key,
			  auto_ptr<QuartzDbBlock> data)
{
    map<QuartzDbKey, QuartzDbBlock *>::iterator i = blocks.find(key);

    if (data.get() == 0) {
	// Don't allow null pointers, for convenience.  (Makes get_block()
	// easier, for a start).
	auto_ptr<QuartzDbBlock> temp(new QuartzDbBlock());
	data = temp;
    }

    if (i == blocks.end()) {
	blocks[key] = data.get();
    } else {
	delete (i->second);
	i->second = data.get();
    }
    data.release();
}


