/* net_termlist.cc
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2006,2007,2008,2009,2010 Olly Betts
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

#include "net_termlist.h"

#include "omassert.h"

#include <xapian/error.h>

using Xapian::Internal::intrusive_ptr;

NetworkTermList::NetworkTermList(Xapian::termcount document_length_,
				 Xapian::doccount database_size_,
				 intrusive_ptr<const RemoteDatabase> this_db_,
				 Xapian::docid did_)
	: items(),
	  current_position(items.begin()),
	  started(false),
	  document_length(document_length_),
	  database_size(database_size_),
	  this_db(this_db_),
	  did(did_)
{
}

Xapian::termcount
NetworkTermList::get_approx_size() const
{
    return items.size();
}

void
NetworkTermList::accumulate_stats(Xapian::Internal::ExpandStats & stats) const
{
    Assert(started);
    Assert(!at_end());

    stats.accumulate(current_position->wdf,
		     document_length,
		     current_position->termfreq,
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
	++current_position;
    } else {
	started = true;
    }
    return NULL;
}

TermList *
NetworkTermList::skip_to(const string & term)
{
    while (current_position != items.end() && current_position->tname < term) {
	++current_position;
    }
    started = true;
    return NULL;
}

bool
NetworkTermList::at_end() const
{
    Assert(started);
    return (current_position == items.end());
}

Xapian::termcount
NetworkTermList::positionlist_count() const
{
    throw Xapian::UnimplementedError("NetworkTermList::positionlist_count() not implemented");
}

Xapian::PositionIterator
NetworkTermList::positionlist_begin() const
{
    return Xapian::PositionIterator(this_db->open_position_list(did, get_termname()));
}
