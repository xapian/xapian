/* quartz_table_entries.cc: Storage of a set of entries from a quartz table
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003 Olly Betts
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

#include <config.h>
#include "omdebug.h"

#include "quartz_table_entries.h"

QuartzTableEntries::QuartzTableEntries()
{
    DEBUGCALL(DB, void, "QuartzTableEntries", "");
    string key;
    entries[key] = 0;
}

QuartzTableEntries::~QuartzTableEntries()
{
    DEBUGCALL(DB, void, "~QuartzTableEntries", "");
    clear();
}

string *
QuartzTableEntries::get_tag(const string &key)
{
    DEBUGCALL(DB, string *, "QuartzTableEntries::get_tag", key);
    Assert(!key.empty());
    items::iterator i = entries.find(key);
    Assert(i != entries.end());
    return i->second; // FIXME can't use RETURN for now
}

const string *
QuartzTableEntries::get_tag(const string &key) const
{
    DEBUGCALL(DB, string *, "QuartzTableEntries::get_tag", key);
    Assert(!key.empty());
    items::const_iterator i = entries.find(key);
    Assert(i != entries.end());
    return i->second; // FIXME can't use RETURN for now
}

bool
QuartzTableEntries::have_entry(const string &key) const
{
    DEBUGCALL(DB, bool, "QuartzTableEntries::have_entry", key);
    Assert(!key.empty());
    RETURN((entries.find(key) != entries.end()));
}

QuartzTableEntries::items::const_iterator
QuartzTableEntries::get_iterator(const string & key) const
{
    DEBUGCALL(DB, QuartzTableEntries::items::const_iterator,
	      "QuartzTableEntries::get_iterator", key);
    // FIXME: think we now allow empty keys (to mean first entry)
    // Assert(!key.empty());
    items::const_iterator result = entries.lower_bound(key);
    if (key.empty()) ++result;

    if (result != entries.end() && result->first == key) {
	// Exact match
	return result;
    }

    // There should always be at least the null entry before this match.
    Assert(result != entries.begin());

    // Point to match _before_ that searched for.
    result--;

    return result;
}

void
QuartzTableEntries::get_item(items::const_iterator iter,
			     const string ** keyptr,
			     const string ** tagptr) const
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
    --iter;
}

void
QuartzTableEntries::next(items::const_iterator & iter) const
{
    DEBUGCALL(DB, void, "QuartzTableEntries::next", "[iter]");
    Assert(iter != entries.end());
    ++iter;
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
    return (entries.size() == 1);
}

void
QuartzTableEntries::set_tag(const string &key, AutoPtr<string> tag)
{
    DEBUGCALL(DB, void, "QuartzTableEntries::set_tag", key << ", " << (tag.get() ? *tag : string("<NULL>")));
    Assert(!key.empty());
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
    string key;
    entries[key] = 0;
}

QuartzTableEntries::items &
QuartzTableEntries::get_all_entries()
{
    return entries;
}
