/* quartz_table_entries.cc: Storage of a set of entries from a quartz table
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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
#include "omdebug.h"

#include "quartz_table_entries.h"

QuartzTableEntries::QuartzTableEntries()
{
    QuartzDbKey key;
    entries[key] = 0;
}

QuartzTableEntries::~QuartzTableEntries()
{
    clear();
}

QuartzDbTag *
QuartzTableEntries::get_tag(const QuartzDbKey &key)
{
    Assert(key.value != "");
    items::iterator i = entries.find(key);
    Assert(i != entries.end());
    return i->second;
}

const QuartzDbTag *
QuartzTableEntries::get_tag(const QuartzDbKey &key) const
{
    Assert(key.value != "");
    return const_cast<QuartzTableEntries *>(this)->get_tag(key);
}

bool
QuartzTableEntries::have_entry(const QuartzDbKey &key) const
{
    Assert(key.value != "");
    return (entries.find(key) != entries.end());
}

QuartzTableEntries::items::const_iterator
QuartzTableEntries::get_iterator(const QuartzDbKey & key) const
{
    Assert(key.value != "");
    items::const_iterator result = entries.lower_bound(key);

    if (result != entries.end() && result->first.value == key.value) {
	// Exact match
	return result;
    }

    // There should always be at least the null entry before this match.
    Assert (result != entries.begin());

    // Point to match _before_ that searched for.
    result--;

    return result;
}

void
QuartzTableEntries::get_item(items::const_iterator iter,
			     const QuartzDbKey ** keyptr,
			     const QuartzDbTag ** tagptr) const
{
    Assert (iter != entries.end());

    *keyptr = &(iter->first);
    *tagptr = iter->second;
}

void
QuartzTableEntries::prev(items::const_iterator & iter) const
{
    Assert(iter != entries.begin());
    iter--;
}

void
QuartzTableEntries::next(items::const_iterator & iter) const
{
    Assert(iter != entries.end());
    iter++;
}

bool
QuartzTableEntries::after_end(items::const_iterator & iter) const
{
    return (iter == entries.end());
}

bool
QuartzTableEntries::empty() const
{
    return (entries.empty());
}

void
QuartzTableEntries::set_tag(const QuartzDbKey &key, AutoPtr<QuartzDbTag> tag)
{
    Assert(key.value != "");
    items::iterator i = entries.find(key);

    if (i == entries.end()) {
	entries[key] = tag.get();
    } else {
	delete (i->second);
	i->second = tag.get();
    }
    tag.release();
}

void
QuartzTableEntries::clear()
{
    items::iterator i;
    for (i = entries.begin(); i != entries.end(); i++) {
	delete (i->second);
	i->second = 0;
    }
    entries.clear();
    QuartzDbKey key;
    entries[key] = 0;
}

QuartzTableEntries::items &
QuartzTableEntries::get_all_entries()
{
    return entries;
}

