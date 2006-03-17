/* net_termlist.cc
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2006 Olly Betts
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
#include <stdio.h>

#include "omdebug.h"
#include "net_termlist.h"

#include <vector>

#include <xapian/error.h>

NetworkTermList::NetworkTermList(Xapian::doclength /*average_length*/,
				 Xapian::doccount database_size_,
				 const std::vector<NetClient::TermListItem> &items_,
				 Xapian::Internal::RefCntPtr<const NetworkDatabase> this_db_)
	: items(),
	  current_position(items.begin()),
	  started(false),
	  database_size(database_size_),
	  this_db(this_db_)
{
    // FIXME: set length
    document_length = 1;

    std::vector<NetClient::TermListItem>::const_iterator i;
    for (i = items_.begin(); i != items_.end(); ++i) {
	NetworkTermListItem item;
	item.tname = i->tname;
	item.termfreq = i->termfreq;
	item.wdf = i->wdf;

	items.push_back(item);
    }

    current_position = items.begin();
}

Xapian::termcount
NetworkTermList::get_approx_size() const
{
    return items.size();
}

OmExpandBits
NetworkTermList::get_weighting() const
// FIXME: change this to get_weighting_info, which returns the info needed
// to call get_bits.
{
    Assert(started);
    Assert(!at_end());
    Assert(wt != NULL);

    return wt->get_bits(NetworkTermList::get_wdf(),
			document_length,
			NetworkTermList::get_termfreq(),
			database_size);
}

string
NetworkTermList::get_termname() const
{
    Assert(started);
    Assert(!at_end());
    return current_position->tname;
}

Xapian::termcount
NetworkTermList::get_wdf() const
{
    Assert(started);
    Assert(!at_end());
    return current_position->wdf;
}

Xapian::doccount
NetworkTermList::get_termfreq() const
{
    Assert(started);
    Assert(!at_end());
    return current_position->termfreq;
}

TermList *
NetworkTermList::next()
{
    if (started) {
	Assert(!at_end());
	current_position++;
    } else {
	started = true;
    }
    return NULL;
}

bool
NetworkTermList::at_end() const
{
    Assert(started);
    return (current_position == items.end());
}

Xapian::PositionIterator
NetworkTermList::positionlist_begin() const
{
    /* FIXME: NetworkDatabase doesn't support open_position_list() yet. */
    throw Xapian::UnimplementedError("positionlist_begin not supported by remote backend");
    /*return this_db->open_position_list(did, get_termname());*/
}
