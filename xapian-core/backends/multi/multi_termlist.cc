/** @file
 * @brief C++ class definition for multiple database access
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2010,2011,2018 Olly Betts
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

#include "multi_termlist.h"

#include "backends/database.h"
#include "debuglog.h"
#include "expand/expandweight.h"

MultiTermList::MultiTermList(TermList * tl_,
			     const Xapian::Database &db_,
			     size_t db_index_)
	: tl(tl_), db(db_), db_index(db_index_)
{
}

MultiTermList::~MultiTermList()
{
    delete tl;
}

Xapian::termcount
MultiTermList::get_approx_size() const
{
    return tl->get_approx_size();
}

void
MultiTermList::accumulate_stats(Xapian::Internal::ExpandStats & stats) const
{
    tl->accumulate_stats(stats);
}

string
MultiTermList::get_termname() const
{
    return tl->get_termname();
}

Xapian::termcount MultiTermList::get_wdf() const
{
    return tl->get_wdf();
}

Xapian::doccount MultiTermList::get_termfreq() const
{
    return db.get_termfreq(tl->get_termname());
}

TermList * MultiTermList::next()
{
    return tl->next();
}

TermList * MultiTermList::skip_to(const string & term)
{
    return tl->skip_to(term);
}

bool MultiTermList::at_end() const
{
    return tl->at_end();
}

Xapian::termcount
MultiTermList::positionlist_count() const
{
    return tl->positionlist_count();
}

Xapian::PositionIterator
MultiTermList::positionlist_begin() const
{
    return tl->positionlist_begin();
}
