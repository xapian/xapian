/* msetpostlist.cc: MSET of two posting lists
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

// NB don't prune - even with one sublist we still translate docids...

#include "msetpostlist.h"
#include "omdebug.h"

        
MSetPostList::MSetPostList(const OmMSet mset_, const NetworkDatabase *db_)
    : mset(mset_), db(db_), current(-1)
{
    DEBUGCALL(MATCH, void, "MSetPostList::MSetPostList", mset_ << ", " << db_);
}

MSetPostList::~MSetPostList()
{
    DEBUGCALL(MATCH, void, "MSetPostList::~MSetPostList", "");
}

PostList *
MSetPostList::next(om_weight w_min)
{
    Assert(current == -1 || !at_end());
    current++;
    return NULL;
}

PostList *
MSetPostList::skip_to(om_docid did, om_weight w_min)
{
    // MSetPostList doesn't return documents in docid order, so skip_to
    // isn't a meaningful operation.
    throw OmUnimplementedError("MSetPostList doesn't support skip_to");
}

PendingMSetPostList::~PendingMSetPostList()
{
    delete pl;
}
