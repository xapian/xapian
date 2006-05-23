/* msetpostlist.cc: MSET of two posting lists
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2006 Olly Betts
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
#include "msetpostlist.h"
#include "net_database.h"
#include "omdebug.h"

MSetPostList::MSetPostList(const Xapian::MSet mset_, NetworkDatabase *db_)
    : mset(mset_), db(db_), current(-1)
{
    DEBUGCALL(MATCH, void, "MSetPostList::MSetPostList", mset_ << ", " << db_);
}

MSetPostList::~MSetPostList()
{
    DEBUGCALL(MATCH, void, "MSetPostList::~MSetPostList", "");
}

PostList *
MSetPostList::next(Xapian::weight w_min)
{
    DEBUGCALL(MATCH, PostList *, "MSetPostList::next", w_min);
    (void)w_min;
    Assert(current == -1 || !at_end());
    current++;
    RETURN(NULL);
}

PostList *
MSetPostList::skip_to(Xapian::docid did, Xapian::weight w_min)
{
    DEBUGCALL(MATCH, PostList *, "MSetPostList::skip_to", did << ", " << w_min);
    (void)did;
    (void)w_min;
    // MSetPostList doesn't return documents in docid order, so skip_to
    // isn't a meaningful operation.
    throw Xapian::InvalidOperationError("MSetPostList doesn't support skip_to");
}

void
PendingMSetPostList::make_pl()
{
    if (pl) return;
    Xapian::MSet mset;
    while (!db->get_mset(0, maxitems, mset)) {
	db->wait_for_input();
    }
    pl = new MSetPostList(mset, db);
}

PendingMSetPostList::~PendingMSetPostList()
{
    DEBUGCALL(MATCH, void, "~PendingMSetPostList", "");
    delete pl;
}

Xapian::doccount
MSetPostList::get_termfreq_max() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "MSetPostList::get_termfreq_max", "");
    return mset.get_matches_upper_bound();
}

Xapian::doccount
MSetPostList::get_termfreq_min() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "MSetPostList::get_termfreq_min", "");
    return mset.get_matches_lower_bound();
}

Xapian::doccount
MSetPostList::get_termfreq_est() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "MSetPostList::get_termfreq_est", "");
    return mset.get_matches_estimated();
}

Xapian::docid
MSetPostList::get_docid() const
{
    DEBUGCALL(MATCH, Xapian::docid, "MSetPostList::get_docid", "");
    Assert(current != -1);
    RETURN(mset.internal->items[current].did);
}

Xapian::weight
MSetPostList::get_weight() const
{
    DEBUGCALL(MATCH, Xapian::weight, "MSetPostList::get_weight", "");
    Assert(current != -1);
    return mset.internal->items[current].wt;
}

const string *
MSetPostList::get_collapse_key() const
{
    DEBUGCALL(MATCH, string *, "MSetPostList::get_collapse_key", "");
    Assert(current != -1);
    return &(mset.internal->items[current].collapse_key);
}

Xapian::weight
MSetPostList::get_maxweight() const
{
    DEBUGCALL(MATCH, Xapian::weight, "MSetPostList::get_maxweight", "");
    // Before we've started, return max_possible...
    // FIXME: when current advances from -1 to 0, we should probably call
    // recalc_maxweight on the matcher...
    if (current == -1) return mset.get_max_possible();
    if (mset.empty()) return 0;
    // mset.max_attained is bigger than this if firstitem != 0
    return mset.internal->items[current].wt;
}

Xapian::weight
MSetPostList::recalc_maxweight()
{
    DEBUGCALL(MATCH, Xapian::weight, "MSetPostList::recalc_maxweight", "");
    return get_maxweight();
}

bool
MSetPostList::at_end() const
{
    DEBUGCALL(MATCH, bool, "MSetPostList::at_end", "");
    Assert(current != -1);
    return unsigned(current) >= mset.size();
}

string
MSetPostList::get_description() const
{
    return "( MSet " + mset.get_description() + " )";
}

Xapian::doclength
MSetPostList::get_doclength() const
{
    DEBUGCALL(MATCH, Xapian::doclength, "MSetPostList::get_doclength", "");
    Assert(current != -1);
    return 1; // FIXME: this info is unused with present weights
//    return db->get_doclength(mset.internal->items[current].did);
}

PositionList *
MSetPostList::read_position_list()
{
    DEBUGCALL(MATCH, PositionList *, "MSetPostList::read_position_list", "");
    throw Xapian::UnimplementedError("MSetPostList::read_position_list() unimplemented");
}

PositionList *
MSetPostList::open_position_list() const
{
    throw Xapian::UnimplementedError("MSetPostList::open_position_list() unimplemented");
}
