/* quartz_db_entries.cc: Storage of a set of entries from a quartz db table
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

#include "quartz_db_entries.h"

QuartzDbEntries::QuartzDbEntries()
{
}

QuartzDbEntries::~QuartzDbEntries()
{
    std::map<QuartzDbKey, QuartzDbTag *>::iterator i;
    for (i = entries.begin(); i != entries.end(); i++) {
	delete (i->second);
	i->second = 0;
    }
}

QuartzDbTag *
QuartzDbEntries::get_tag(const QuartzDbKey &key)
{
    std::map<QuartzDbKey, QuartzDbTag *>::iterator i = entries.find(key);
    if (i == entries.end()) return 0;
    return i->second;
}

void
QuartzDbEntries::set_tag(const QuartzDbKey &key,
			 auto_ptr<QuartzDbTag> data)
{
    std::map<QuartzDbKey, QuartzDbTag *>::iterator i = entries.find(key);

    if (data.get() == 0) {
	// Don't allow null pointers, for convenience.  (Makes get_tag()
	// easier, for a start).
	auto_ptr<QuartzDbTag> temp(new QuartzDbTag());
	data = temp;
    }

    if (i == entries.end()) {
	entries[key] = data.get();
    } else {
	delete (i->second);
	i->second = data.get();
    }
    data.release();
}


