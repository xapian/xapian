/* andnotpostlist.cc: Return items which are in A, unless they're in B
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004 Olly Betts
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
#include "andnotpostlist.h"
#include <algorithm>

PostList *
AndNotPostList::advance_to_next_match(Xapian::weight w_min, PostList *ret)
{
    DEBUGCALL(MATCH, PostList *, "AndNotPostList::advance_to_next_match", w_min << ", " << ret);
    handle_prune(l, ret);
    if (l->at_end()) {
	lhead = 0;
	RETURN(NULL);
    }
    lhead = l->get_docid();

    while (rhead <= lhead) {
	if (lhead == rhead) {
	    next_handling_prune(l, w_min, matcher);
	    if (l->at_end()) {
		lhead = 0;
		RETURN(NULL);
	    }
	    lhead = l->get_docid();
	}
	skip_to_handling_prune(r, lhead, 0, matcher);
	if (r->at_end()) {
	    ret = l;
	    l = NULL;
	    RETURN(ret);
	}
	rhead = r->get_docid();
    }
    RETURN(NULL);
}

AndNotPostList::AndNotPostList(PostList *left_,
			       PostList *right_,
			       MultiMatch *matcher_,
			       Xapian::doccount dbsize_)
	: BranchPostList(left_, right_, matcher_),
	  lhead(0), rhead(0), dbsize(dbsize_)
{
    DEBUGCALL(MATCH, void, "AndNotPostList", left_ << ", " << right_ << ", " << matcher_ << ", " << dbsize_);
}

PostList *
AndNotPostList::next(Xapian::weight w_min)
{
    DEBUGCALL(MATCH, PostList *, "AndNotPostList::next", w_min);
    RETURN(advance_to_next_match(w_min, l->next(w_min)));
}

PostList *
AndNotPostList::sync_and_skip_to(Xapian::docid id,
				 Xapian::weight w_min,
				 Xapian::docid lh,
				 Xapian::docid rh)
{
    DEBUGCALL(MATCH, PostList *, "AndNotPostList::sync_and_skip_to", id << ", " << w_min << ", " << lh << ", " << rh);
    lhead = lh;
    rhead = rh;
    RETURN(skip_to(id, w_min));
}

PostList *
AndNotPostList::skip_to(Xapian::docid did, Xapian::weight w_min)
{
    DEBUGCALL(MATCH, PostList *, "AndNotPostList::skip_to", did << ", " << w_min);
    if (did <= lhead) RETURN(NULL);
    RETURN(advance_to_next_match(w_min, l->skip_to(did, w_min)));
}

Xapian::doccount
AndNotPostList::get_termfreq_max() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "AndNotPostList::get_termfreq_max", "");
    // Max is when as many docs as possible on left, and none excluded.
    RETURN(l->get_termfreq_max());
}

Xapian::doccount
AndNotPostList::get_termfreq_min() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "AndNotPostList::get_termfreq_min", "");
    // Min is when as few docs as possible on left, and maximum are excluded.
    Xapian::doccount l_min = l->get_termfreq_min();
    Xapian::doccount r_max = r->get_termfreq_max();
    if (l_min > r_max) RETURN(l_min - r_max);
    RETURN(0u);
}

Xapian::doccount
AndNotPostList::get_termfreq_est() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "AndNotPostList::get_termfreq_est", "");
    // Estimate assuming independence:
    // P(l and r) = P(l) . P(r)
    // P(l not r) = P(l) - P(l and r) = P(l) . ( 1 - P(r))
    Xapian::doccount est = static_cast<Xapian::doccount>
	    (l->get_termfreq_est() *
	     (1.0 - static_cast<double>(r->get_termfreq_est()) / dbsize));

    RETURN(est);
}

Xapian::docid
AndNotPostList::get_docid() const
{
    DEBUGCALL(MATCH, Xapian::docid, "AndNotPostList::get_docid", "");
    RETURN(lhead);
}

// only called if we are doing a probabilistic AND NOT
Xapian::weight
AndNotPostList::get_weight() const
{
    DEBUGCALL(MATCH, Xapian::weight, "AndNotPostList::get_weight", "");
    RETURN(l->get_weight());
}

// only called if we are doing a probabilistic AND NOT
Xapian::weight
AndNotPostList::get_maxweight() const
{
    DEBUGCALL(MATCH, Xapian::weight, "AndNotPostList::get_maxweight", "");
    RETURN(l->get_maxweight());
}

Xapian::weight
AndNotPostList::recalc_maxweight()
{
    DEBUGCALL(MATCH, Xapian::weight, "AndNotPostList::recalc_maxweight", "");
    RETURN(l->recalc_maxweight());
}

bool
AndNotPostList::at_end() const
{
    DEBUGCALL(MATCH, bool, "AndNotPostList::at_end", "");
    RETURN(lhead == 0);
}

std::string
AndNotPostList::get_description() const
{
    return "(" + l->get_description() + " AndNot " + r->get_description() + ")";
}

Xapian::doclength
AndNotPostList::get_doclength() const
{
    DEBUGCALL(MATCH, Xapian::doclength, "AndNotPostList::get_doclength", "");
    RETURN(l->get_doclength());
}
