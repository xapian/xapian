/* quartz_table_entries.cc: Storage of a set of entries from a quartz table
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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
    DEBUGCALL(DB, void, "QuartzTableEntries", "");
    QuartzDbKey key;
    entries[key] = 0;
}

QuartzTableEntries::~QuartzTableEntries()
{
    DEBUGCALL(DB, void, "~QuartzTableEntries", "");
    clear();
}

QuartzDbTag *
QuartzTableEntries::get_tag(const QuartzDbKey &key)
{
    DEBUGCALL(DB, QuartzDbTag *, "QuartzTableEntries::get_tag", key.value);
    Assert(key.value != "");
    items::iterator i = entries.find(key);
    Assert(i != entries.end());
    return i->second; // FIXME can't use RETURN for now
}

const QuartzDbTag *
QuartzTableEntries::get_tag(const QuartzDbKey &key) const
{
    DEBUGCALL(DB, QuartzDbTag *, "QuartzTableEntries::get_tag", key.value);
    Assert(key.value != "");
    return const_cast<QuartzTableEntries *>(this)->get_tag(key); // FIXME RETURN
}

bool
QuartzTableEntries::have_entry(const QuartzDbKey &key) const
{
    DEBUGCALL(DB, bool, "QuartzTableEntries::have_entry", key.value);
    Assert(key.value != "");
    RETURN((entries.find(key) != entries.end()));
}

QuartzTableEntries::items::const_iterator
QuartzTableEntries::get_iterator(const QuartzDbKey & key) const
{
    DEBUGCALL(DB, QuartzTableEntries::items::const_iterator,
	      "QuartzTableEntries::get_iterator", key.value);
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
    DEBUGCALL(DB, void, "QuartzTableEntries::get_item", "[iter], " << keyptr << ", " << tagptr);
    Assert (iter != entries.end());

    *keyptr = &(iter->first);
    *tagptr = iter->second;
}

void
QuartzTableEntries::prev(items::const_iterator & iter) const
{
    DEBUGCALL(DB, void, "QuartzTableEntries::prev", "[iter]");
    Assert(iter != entries.begin());
    iter--;
}

void
QuartzTableEntries::next(items::const_iterator & iter) const
{
    DEBUGCALL(DB, void, "QuartzTableEntries::next", "[iter]");
    Assert(iter != entries.end());
    iter++;
}

bool
QuartzTableEntries::after_end(items::const_iterator & iter) const
{
    DEBUGCALL(DB, bool, "QuartzTableEntries::after_end", "[iter]");
    return (iter == entries.end());
}

bool
QuartzTableEntries::empty() const
{
    DEBUGCALL(DB, bool, "QuartzTableEntries::empty", "");
    return (entries.empty());
}

void
QuartzTableEntries::set_tag(const QuartzDbKey &key, AutoPtr<QuartzDbTag> tag)
{
    DEBUGCALL(DB, void, "QuartzTableEntries::set_tag", "[key], [tag]");
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
    DEBUGCALL(DB, void, "QuartzTableEntries::clear", "");
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

