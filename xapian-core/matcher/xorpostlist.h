/* xorpostlist.h: XOR of two posting lists
 *
 * ----START-LICENCE----
 * Copyright 1999 Dialog Corporation
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

#ifndef _xorpostlist_h_
#define _xorpostlist_h_

#include "database.h"
#include "branchpostlist.h"

class XorPostList : public virtual BranchPostList {
    private:
        docid lhead, rhead;
        weight lmax, rmax, minmax;

        PostList *advance_to_next_match(weight w_min);
    public:
	doccount get_termfreq() const;

	docid  get_docid() const;
	weight get_weight() const;
	weight get_maxweight() const;

        weight recalc_maxweight();

	PostList *next(weight w_min);
	PostList *skip_to(docid, weight w_min);
	bool   at_end() const;

	string intro_term_description() const;

        XorPostList(PostList *, PostList *, Match *);
};

inline doccount
XorPostList::get_termfreq() const
{
    // this is actually the maximum possible frequency for the union of
    // the terms
    return l->get_termfreq() + r->get_termfreq();
}

inline docid
XorPostList::get_docid() const
{
    Assert(lhead != 0 && rhead != 0); // check we've started
    return min(lhead, rhead);
}

// only called if we are doing a probabilistic XOR
inline weight
XorPostList::get_weight() const
{
    Assert(lhead != 0 && rhead != 0); // check we've started
    if (lhead < rhead) return l->get_weight();
    Assert(lhead > rhead);
    return r->get_weight();
}

// only called if we are doing a probabilistic operation
inline weight
XorPostList::get_maxweight() const
{
    return max(lmax, rmax);
}

inline weight
XorPostList::recalc_maxweight()
{
    lmax = l->recalc_maxweight();
    rmax = r->recalc_maxweight();
    minmax = min(lmax, rmax);
    return XorPostList::get_maxweight();
}

inline bool
XorPostList::at_end() const
{
    return lhead == 0;
}

inline string
XorPostList::intro_term_description() const
{
    return "(" + l->intro_term_description() + " Xor " +
	    r->intro_term_description() + ")";
}

#endif /* _xorpostlist_h_ */
