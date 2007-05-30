/* inmemoryalltermslist.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003,2004 Olly Betts
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
#include "inmemory_alltermslist.h"

InMemoryAllTermsList::InMemoryAllTermsList(const std::map<string, InMemoryTerm> *tmap_,
					   Xapian::Internal::RefCntPtr<const InMemoryDatabase> database_,
					   const string & prefix_)
	: tmap(tmap_), it(tmap->begin()), database(database_), started(false), prefix(prefix_)
{
    while (it != tmap->end() &&
	   (it->second.term_freq == 0 ||
	    it->first.substr(0, prefix.size()) != prefix))
	++it;
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
InMemoryAllTermsList::skip_to(const string &tname)
{
    started = true;
    // FIXME: might skip backwards - is this a problem?
    it = tmap->lower_bound(tname);
    while (it != tmap->end() && it->second.term_freq == 0) ++it;
    if (it != tmap->end() && it->first.substr(0, prefix.size()) != prefix)
	it = tmap->end();
    return NULL;
}

TermList *
InMemoryAllTermsList::next()
{
    if (!started) {
	started = true;
    } else {
	Assert(!at_end());
	++it;
	while (it != tmap->end() && it->second.term_freq == 0) ++it;
    }
    if (it != tmap->end() && it->first.substr(0, prefix.size()) != prefix)
	it = tmap->end();
    return NULL;
}

bool
InMemoryAllTermsList::at_end() const
{
    Assert(started);
    return (it == tmap->end());
}
