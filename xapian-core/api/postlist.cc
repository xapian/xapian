/** @file postlist.cc
 * @brief Abstract base class for postlists.
 */
/* Copyright (C) 2007,2009 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "postlist.h"

#include <xapian/error.h>

#include "omassert.h"

using namespace std;

namespace Xapian {

PostingIterator::Internal::~Internal() { }

TermFreqs
PostingIterator::Internal::get_termfreq_est_using_stats(
	const Xapian::Weight::Internal &) const
{
    throw Xapian::InvalidOperationError(
     "get_termfreq_est_using_stats() not meaningful for this PostingIterator");
}

Xapian::termcount
PostingIterator::Internal::get_wdf() const
{
    throw Xapian::InvalidOperationError("get_wdf() not meaningful for this PostingIterator");
}

const string *
PostingIterator::Internal::get_collapse_key() const
{
    return NULL;
}

PositionList *
PostList::read_position_list()
{
    throw Xapian::InvalidOperationError("read_position_list() not meaningful for this PostingIterator");
}

PositionList *
PostList::open_position_list() const
{
    throw Xapian::InvalidOperationError("open_position_list() not meaningful for this PostingIterator");
}

PostList *
PostList::check(Xapian::docid did, Xapian::weight w_min, bool &valid)
{
    valid = true;
    return skip_to(did, w_min);
}

Xapian::termcount
PostList::count_matching_subqs() const
{
    Assert(false);
    return 0;
}

}
