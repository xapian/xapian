/* msetpostlist.cc: MSET of two posting lists
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003 Olly Betts
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
#include "msetpostlist.h"
#include "omdebug.h"

MSetPostList::MSetPostList(const Xapian::MSet mset_, const NetworkDatabase *db_)
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

PendingMSetPostList::~PendingMSetPostList()
{
    DEBUGCALL(MATCH, void, "~PendingMSetPostList", "");
    delete pl;
}
