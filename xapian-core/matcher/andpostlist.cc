/* andpostlist.cc: Return only items which are in both sublists
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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

#include "andpostlist.h"

inline void
AndPostList::process_next_or_skip_to(om_weight w_min, PostList *ret)
{
    head = 0;
    handle_prune(r, ret);
    if (r->at_end()) return;

    // r has just been advanced by next or skip_to so must be > head
    // (and head is the current position of l)
    skip_to_handling_prune(l, r->get_docid(), w_min - rmax, matcher);
    if (l->at_end()) return;

    om_docid lhead = l->get_docid();
    om_docid rhead = r->get_docid();

    while (lhead != rhead) {
	if (lhead < rhead) {
	    // FIXME: CSE these w_min values?
	    // But note that lmax and rmax may change on recalc_maxweight...
	    skip_to_handling_prune(l, rhead, w_min - rmax, matcher);
	    if (l->at_end()) {
		head = 0;
		return;
	    }
	    lhead = l->get_docid();
	} else {
	    skip_to_handling_prune(r, lhead, w_min - lmax, matcher);
	    if (r->at_end()) {
		head = 0;
		return;
	    }
	    rhead = r->get_docid();
	}
    }

    head = lhead;
    return;
}

AndPostList::AndPostList(PostList *left_, PostList *right_,
			 MultiMatch *matcher_,
			 om_doccount dbsize_,
			 bool replacement)
	: BranchPostList(left_, right_, matcher_), head(0), dbsize(dbsize_)
{
    if (replacement) {
	// Initialise the maxweights from the kids so we can avoid forcing
	// a full maxweight recalc
	lmax = l->get_maxweight();
	rmax = r->get_maxweight();
    }
}

PostList *
AndPostList::next(om_weight w_min)
{
    process_next_or_skip_to(w_min, r->next(w_min - lmax));
    return NULL;
}

PostList *
AndPostList::skip_to(om_docid did, om_weight w_min)
{
    if (did > head) process_next_or_skip_to(w_min, r->skip_to(did, w_min - lmax));
    return NULL;
}
