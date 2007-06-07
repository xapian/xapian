/* inmemoryalltermslist.cc
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003,2004,2007 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>
#include "inmemory_alltermslist.h"

#include "stringutils.h"

InMemoryAllTermsList::InMemoryAllTermsList(const std::map<string, InMemoryTerm> *tmap_,
					   Xapian::Internal::RefCntPtr<const InMemoryDatabase> database_,
					   const string & prefix_)
	: tmap(tmap_), database(database_), started(false), prefix(prefix_)
{
}

InMemoryAllTermsList::~InMemoryAllTermsList()
{
}

Xapian::termcount
InMemoryAllTermsList::get_approx_size() const
{
    return tmap->size();
}

string
InMemoryAllTermsList::get_termname() const
{
    Assert(started);
    Assert(!at_end());
    return it->first;
}

Xapian::doccount
InMemoryAllTermsList::get_termfreq() const
{
    Assert(started);
    Assert(!at_end());
    /* FIXME: this isn't quite right. */
    return it->second.docs.size();
}

Xapian::termcount
InMemoryAllTermsList::get_collection_freq() const
{
    Assert(started);
    Assert(!at_end());
    throw Xapian::UnimplementedError("Collection frequency not implemented in InMemory backend");
}

TermList *
InMemoryAllTermsList::skip_to(const string &tname_)
{
    string tname(tname_);
    if (started) {
	Assert(!at_end());
	// Don't skip backwards.
	if (tname <= get_termname()) return NULL;
    } else {
	started = true;
	// Don't skip to before where we're supposed to start.
	if (tname < prefix) tname = prefix;
    }
    it = tmap->lower_bound(tname);
    while (it != tmap->end() && it->second.term_freq == 0) ++it;
    if (it != tmap->end() && !begins_with(it->first, prefix))
	it = tmap->end();
    return NULL;
}

TermList *
InMemoryAllTermsList::next()
{
    if (!started) {
	started = true;
	it = tmap->lower_bound(prefix);
    } else {
	Assert(!at_end());
	++it;
    }
    while (it != tmap->end() && it->second.term_freq == 0) ++it;
    if (it != tmap->end() && !begins_with(it->first, prefix))
	it = tmap->end();
    return NULL;
}

bool
InMemoryAllTermsList::at_end() const
{
    Assert(started);
    return (it == tmap->end());
}
