/* branchpostlist.h: virtual base class for branched types of postlist
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#ifndef OM_HGUARD_BRANCHPOSTLIST_H
#define OM_HGUARD_BRANCHPOSTLIST_H

#include "match.h"
#include "postlist.h"

class BranchPostList : public virtual PostList {
    protected:
        void handle_prune(PostList *&kid, PostList *ret);
        PostList *l, *r;
        OmMatch *root;
    public:
        virtual ~BranchPostList();
};

inline
BranchPostList::~BranchPostList()
{
    if (l) delete l;
    if (r) delete r;
}

inline void
BranchPostList::handle_prune(PostList *&kid, PostList *ret)
{
    if (ret) {
	delete kid;
	kid = ret;
	// now get tree to recalculate max weights...
	root->recalc_maxweight();
    }
}

#endif /* OM_HGUARD_BRANCHPOSTLIST_H */
