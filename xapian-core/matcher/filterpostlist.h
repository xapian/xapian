/* filterpostlist.h: apply a boolean posting list as a filter to a
 * probabilistic posting list
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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

/** A postlist comprising two postlists ANDed together, but with weights
 *  taken only from one.
 *
 *  This postlist returns a posting if and only if it is in both of the
 *  sub-postlists.  The weight for a posting is the weight of the left-hand
 *  sub-postings.  The weight of the right-hand posting is ignored (and
 *  indeed, should never get calculated).
 *
 *  This is useful for restricting the search to a subset of the
 *  database.  Note that AndNotPostList provides an "AntiFilterPostList".
 */
class FilterPostList : public AndPostList {
    private:
        void process_next_or_skip_to(om_weight w_min, PostList *ret);
    public:
	om_weight get_weight() const;
	om_weight get_maxweight() const;

        om_weight recalc_maxweight();

	PostList *next(om_weight w_min);
	PostList *skip_to(om_docid did, om_weight w_min);

	std::string get_description() const;

        FilterPostList(PostList *left_, PostList *right_, MultiMatch *matcher_,
		       om_doccount dbsize_)
		: AndPostList(left_, right_, matcher_, dbsize_) {}
};

#endif /* OM_HGUARD_FILTERPOSTLIST_H */
