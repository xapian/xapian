/* andpostlist.h: Return only items which are in both sublists
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

#ifndef OM_HGUARD_ANDPOSTLIST_H
#define OM_HGUARD_ANDPOSTLIST_H

#include "database.h"
#include "branchpostlist.h"

/** A postlist comprising two postlists ANDed together.
 *
 *  This postlist returns a posting if and only if it is in both of the
 *  sub-postlists.  The weight for a posting is the sum of the weights of
 *  the sub-postings.
 */
class AndPostList : public BranchPostList {
    private:
        om_docid head;
        om_weight lmax, rmax;

        void process_next_or_skip_to(om_weight w_min, PostList *ret);
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

        AndPostList(PostList *left,
		    PostList *right,
		    LocalMatch *matcher_,
		    bool replacement = false);
};

inline om_doccount
AndPostList::get_termfreq() const
{
    // this is actually the maximum possible frequency for the intersection of
    // the terms
    return min(l->get_termfreq(), r->get_termfreq());
}

inline om_docid
AndPostList::get_docid() const
{
    return head;
}

// only called if we are doing a probabilistic AND
inline om_weight
AndPostList::get_weight() const
{
    return l->get_weight() + r->get_weight();
}

// only called if we are doing a probabilistic operation
inline om_weight
AndPostList::get_maxweight() const
{
    return lmax + rmax;
}

inline om_weight
AndPostList::recalc_maxweight()
{
    lmax = l->recalc_maxweight();
    rmax = r->recalc_maxweight();
    return AndPostList::get_maxweight();
}

inline bool
AndPostList::at_end() const
{
    return head == 0;
}

inline string
AndPostList::intro_term_description() const
{
    return "(" + l->intro_term_description() + " And " +
	    r->intro_term_description() + ")";
}

#endif /* OM_HGUARD_ANDPOSTLIST_H */
