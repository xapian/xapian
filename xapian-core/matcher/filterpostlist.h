/* filterpostlist.h: apply a boolean posting list as a filter to a
 * probabilistic posting list
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

#ifndef OM_HGUARD_FILTERPOSTLIST_H
#define OM_HGUARD_FILTERPOSTLIST_H

#include "database.h"
#include "andpostlist.h"

// FilterPostList(probabilistic, boolean)
// AntiFilterPostList(probabilistic, boolean)

class FilterPostList : public virtual AndPostList {
    public:
	om_weight get_weight() const;
	om_weight get_maxweight() const;

        om_weight recalc_maxweight();

	string intro_term_description() const;

        FilterPostList(PostList *l, PostList *r, LeafMatch *matcher_) :
            AndPostList(l, r, matcher_) {};
};

inline om_weight
FilterPostList::get_weight() const
{
    return l->get_weight();
}

inline om_weight
FilterPostList::get_maxweight() const
{
    return l->get_maxweight();
}

inline om_weight
FilterPostList::recalc_maxweight()
{
    return l->recalc_maxweight();    
}

inline string
FilterPostList::intro_term_description() const
{
    return "(" + l->intro_term_description() + " Filter " +
	    r->intro_term_description() + ")";
}

#endif /* OM_HGUARD_FILTERPOSTLIST_H */
