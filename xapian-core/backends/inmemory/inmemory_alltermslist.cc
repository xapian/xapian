/* inmemoryalltermslist.cc
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

#include "inmemory_alltermslist.h"

InMemoryAllTermsList::InMemoryAllTermsList(std::map<om_termname, InMemoryTerm>::const_iterator begin,
					   std::map<om_termname, InMemoryTerm>::const_iterator end_,
					   const std::map<om_termname, InMemoryTerm> *tmap_,
					   RefCntPtr<const InMemoryDatabase> database_)
	: it(begin), end(end_), tmap(tmap_), database(database_)
{
}

InMemoryAllTermsList::~InMemoryAllTermsList()
{
}

const om_termname
InMemoryAllTermsList::get_termname() const
{
    return it->first;
}

om_doccount
InMemoryAllTermsList::get_termfreq() const
{
    /* FIXME: this isn't quite right. */
    return it->second.docs.size();
}

om_termcount
InMemoryAllTermsList::get_collection_freq() const
{
    throw OmUnimplementedError("Collection frequency not implemented in InMemory backend");
}

bool
InMemoryAllTermsList::skip_to(const om_termname &tname)
{
    it = tmap->lower_bound(tname);
    return (it->first == tname);
}

bool
InMemoryAllTermsList::next()
{
    it++;
    return (it != end);
}

bool
InMemoryAllTermsList::at_end() const
{
    return (it == end);
}
