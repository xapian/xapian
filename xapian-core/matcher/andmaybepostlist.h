/* andmaybepostlist.h: Merged postlist: items from one list, weights from both
 *
 * AND MAYBE of two posting lists
 * A AND MAYBE B is logically just A, but we keep B around for weight purposes
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


#ifndef OM_HGUARD_ANDMAYBEPOSTLIST_H
#define OM_HGUARD_ANDMAYBEPOSTLIST_H

#include "database.h"
#include "branchpostlist.h"

/** A postlist with weights modified by another postlist.
 *
 *  This postlist returns a posting if and only if it is in the left
 *  sub-postlist.
 *
 *  If the posting does not occur in the right postlist, the weight for the
 *  posting is simply that in the left postlist.  If the posting occurs in
 *  both postlists, the weight for the posting is the sum of the weights in
 *  the sub-postlists.
 */
class AndMaybePostList : public BranchPostList { private: om_docid lhead,
    rhead; om_weight lmax, rmax;

        PostList * process_next_or_skip_to(om_weight w_min, PostList *ret);
    public:
	om_doccount get_termfreq() const;

	om_docid  get_docid() const;
	om_weight get_weight() const;
	om_weight get_maxweight() const;

        om_weight init_maxweight();
        om_weight recalc_maxweight();

	PostList *next(om_weight w_min);
	PostList *skip_to(om_docid did, om_weight w_min);
	bool   at_end() const;

	string intro_term_description() const;

	/** Return the document length of the document the current term
	 *  comes from.
	 */
	virtual om_doclength get_doclength() const;

        AndMaybePostList(PostList *left, PostList *right, LocalMatch *matcher_,
			 om_docid lh = 0, om_docid rh = 0);
};

inline om_doccount
AndMaybePostList::get_termfreq() const
{
    // this is exactly correct
    return l->get_termfreq();
}

inline om_docid
AndMaybePostList::get_docid() const
{
    Assert(lhead != 0 && rhead != 0); // check we've started
    return lhead;
}

// only called if we are doing a probabilistic AND MAYBE
inline om_weight
AndMaybePostList::get_weight() const
{
    Assert(lhead != 0 && rhead != 0); // check we've started
    if (lhead == rhead) return l->get_weight() + r->get_weight();
    return l->get_weight();
}

// only called if we are doing a probabilistic operation
inline om_weight
AndMaybePostList::get_maxweight() const
{
    return lmax + rmax;
}

inline om_weight
AndMaybePostList::recalc_maxweight()
{
    lmax = l->recalc_maxweight();
    rmax = r->recalc_maxweight();
    return AndMaybePostList::get_maxweight();
}

inline bool
AndMaybePostList::at_end() const
{
    return lhead == 0;
}

inline string
AndMaybePostList::intro_term_description() const
{
    return "(" + l->intro_term_description() + " AndMaybe " +
	    r->intro_term_description() + ")";
}

inline om_doclength
AndMaybePostList::get_doclength() const
{
    Assert(lhead != 0 && rhead != 0); // check we've started
    if (lhead == rhead) AssertEqDouble(l->get_doclength(), r->get_doclength());
    return l->get_doclength();
}

#endif /* OM_HGUARD_ANDMAYBEPOSTLIST_H */
