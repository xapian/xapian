/* net_termlist.cc
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

#include <stdio.h>

#include "omdebug.h"
#include "net_termlist.h"

#include <vector>

#include "om/omerror.h"

NetworkTermList::NetworkTermList(om_doclength average_length_,
				 om_doccount database_size_,
				 const std::vector<NetClient::TermListItem> &items_)
	: items(),
	  current_position(items.begin()),
	  started(false),
	  database_size(database_size_)
{
    // FIXME: set length
    document_length = 1;

    std::vector<NetClient::TermListItem>::const_iterator i = items_.begin();
    while (i != items_.end()) {
	NetworkTermListItem item;
	item.tname = i->tname;
	item.termfreq = i->termfreq;
	item.wdf = i->wdf;

	items.push_back(item);
	++i;
    }

    current_position = items.begin();

    return;
}

om_termcount
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

const om_termname
NetworkTermList::get_termname() const
{
    Assert(started);
    Assert(!at_end());
    return current_position->tname;
}

om_termcount
NetworkTermList::get_wdf() const
{
    Assert(started);
    Assert(!at_end());
    return current_position->wdf;
}

om_doccount
NetworkTermList::get_termfreq() const
{
    Assert(started);
    Assert(!at_end());
    return current_position->termfreq;
}

TermList *
NetworkTermList::next()
{
    if(started) {
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
    if(current_position != items.end()) return false;
    return true;
}
