/* andnotpostlist.h: Return items which are in A, unless they're in B
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

#ifndef _andnotpostlist_h_
#define _andnotpostlist_h_

#include "database.h"
#include "branchpostlist.h"

class AndNotPostList : public virtual BranchPostList {
    private:
        docid lhead, rhead;

        PostList *advance_to_next_match(weight w_min, PostList *ret);
    public:
	doccount get_termfreq() const;

	docid  get_docid() const;
	weight get_weight() const;
	weight get_maxweight() const;

        weight recalc_maxweight();

	PostList *next(weight w_min);
	PostList *skip_to(docid did, weight w_min);
	bool   at_end() const;

	string intro_term_description() const;

        AndNotPostList(PostList *left,
		       PostList *right,
		       OMMatch *root_);

        PostList *sync_and_skip_to(docid id, weight w_min, docid lh, docid rh);
};

inline doccount
AndNotPostList::get_termfreq() const
{
    // this is actually the maximum possible frequency
    return l->get_termfreq();
}

inline docid
AndNotPostList::get_docid() const
{
    return lhead;
}

// only called if we are doing a probabilistic AND NOT
inline weight
AndNotPostList::get_weight() const
{
    return l->get_weight();
}

// only called if we are doing a probabilistic AND NOT
inline weight
AndNotPostList::get_maxweight() const
{
    return l->get_maxweight();
}

inline weight
AndNotPostList::recalc_maxweight()
{
    return l->recalc_maxweight();
}

inline bool
AndNotPostList::at_end() const
{
    return lhead == 0;
}

inline string
AndNotPostList::intro_term_description() const
{
    return "(" + l->intro_term_description() + " AndNot " +
	    r->intro_term_description() + ")";
}

#endif /* _andnotpostlist_h_ */
