/** @file msetpostlist.cc
 *  @brief PostList returning entries from an MSet
 */
/* Copyright (C) 2006,2007,2009 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include "msetpostlist.h"
#include "omassert.h"
#include "omdebug.h"

Xapian::doccount
MSetPostList::get_termfreq_min() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "MSetPostList::get_termfreq_min", "");
    RETURN(mset_internal->matches_lower_bound);
}

Xapian::doccount
MSetPostList::get_termfreq_est() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "MSetPostList::get_termfreq_est", "");
    RETURN(mset_internal->matches_estimated);
}

Xapian::doccount
MSetPostList::get_termfreq_max() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "MSetPostList::get_termfreq_max", "");
    RETURN(mset_internal->matches_upper_bound);
}

Xapian::weight
MSetPostList::get_maxweight() const
{
    DEBUGCALL(MATCH, Xapian::weight, "MSetPostList::get_maxweight", "");
    // If we've not started, return max_possible from our MSet so that this
    // value gets used to set max_possible in the combined MSet.
    if (cursor == -1) RETURN(mset_internal->max_possible);

    // If the MSet is sorted in descending weight order, then the maxweight we
    // can return from now on is the weight of the current item.
    if (decreasing_relevance) {
	// FIXME: This is actually a reduction in the maxweight...
	if (at_end()) RETURN(0);
	RETURN(mset_internal->items[cursor].wt);
    }

    // Otherwise max_attained is the best answer we can give.
    RETURN(mset_internal->max_attained);
}

Xapian::docid
MSetPostList::get_docid() const
{
    DEBUGCALL(MATCH, Xapian::docid, "MSetPostList::get_docid", "");
    Assert(cursor != -1);
    RETURN(mset_internal->items[cursor].did);
}

Xapian::weight
MSetPostList::get_weight() const
{
    DEBUGCALL(MATCH, Xapian::weight, "MSetPostList::get_weight", "");
    Assert(cursor != -1);
    RETURN(mset_internal->items[cursor].wt);
}

const string *
MSetPostList::get_collapse_key() const
{
    DEBUGCALL(MATCH, string *, "MSetPostList::get_collapse_key", "");
    Assert(cursor != -1);
    RETURN(&mset_internal->items[cursor].collapse_key);
}

Xapian::termcount
MSetPostList::get_doclength() const
{
    throw Xapian::UnimplementedError("MSetPostList::get_doclength() unimplemented");
}

Xapian::weight
MSetPostList::recalc_maxweight()
{
    DEBUGCALL(MATCH, Xapian::weight, "MSetPostList::recalc_maxweight", "");
    RETURN(MSetPostList::get_maxweight());
}

PostList *
MSetPostList::next(Xapian::weight w_min)
{
    DEBUGCALL(MATCH, PostList *, "MSetPostList::next", w_min);
    Assert(cursor == -1 || !at_end());
    ++cursor;
    if (decreasing_relevance) {
	// MSet items are in decreasing weight order, so if the current item
	// doesn't have enough weight, none of the remaining items will, so
	// skip straight to the end.
	if (!at_end() && mset_internal->items[cursor].wt < w_min)
	    cursor = mset_internal->items.size();
    } else {
	// Otherwise, skip to the next entry with enough weight.
	while (!at_end() && mset_internal->items[cursor].wt < w_min)
	    ++cursor;
    }
    RETURN(NULL);
}

PostList *
MSetPostList::skip_to(Xapian::docid, Xapian::weight)
{
    // The usual semantics of skip_to don't make sense since MSetPostList
    // returns documents in MSet order rather than docid order like other
    // PostLists do.
    throw Xapian::InvalidOperationError("MSetPostList::skip_to not meaningful");
}

bool
MSetPostList::at_end() const
{
    DEBUGCALL(MATCH, bool, "MSetPostList::at_end", "");
    Assert(cursor != -1);
    RETURN(size_t(cursor) >= mset_internal->items.size());
}

string
MSetPostList::get_description() const
{
    string desc("(MSet ");
    desc += mset_internal->get_description();
    desc += ')';
    return desc;
}
