/* filterpostlist.cc: apply a boolean posting list as a filter to a
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

#include "filterpostlist.h"

inline void
FilterPostList::process_next_or_skip_to(om_weight w_min, PostList *ret)
{
    DEBUGCALL(MATCH, void, "FilterPostList::process_next_or_skip_to", w_min << ", " << ret);
    head = 0;
    handle_prune(r, ret);
    if (r->at_end()) return;

    // r has just been advanced by next or skip_to so must be > head
    // (and head is the current position of l)
    skip_to_handling_prune(l, r->get_docid(), w_min, matcher);
    if (l->at_end()) return;

    om_docid lhead = l->get_docid();
    om_docid rhead = r->get_docid();

    while (lhead != rhead) {
	if (lhead < rhead) {
	    skip_to_handling_prune(l, rhead, w_min, matcher);
	    if (l->at_end()) {
		head = 0;
		return;
	    }
	    lhead = l->get_docid();
	} else {
	    skip_to_handling_prune(r, lhead, 0, matcher);
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

PostList *
FilterPostList::next(om_weight w_min)
{
    DEBUGCALL(MATCH, PostList *, "FilterPostList::next", w_min);
    process_next_or_skip_to(w_min, r->next(0));
    return NULL;
}

PostList *
FilterPostList::skip_to(om_docid did, om_weight w_min)
{
    DEBUGCALL(MATCH, PostList *, "FilterPostList::skip_to", did << ", " << w_min);
    if (did > head) process_next_or_skip_to(w_min, r->skip_to(did, 0));
    return NULL;
}

om_weight
FilterPostList::get_weight() const
{
    DEBUGCALL(MATCH, om_weight, "FilterPostList::get_weight", "");
    return l->get_weight();
}

om_weight
FilterPostList::get_maxweight() const
{
    DEBUGCALL(MATCH, om_weight, "FilterPostList::get_maxweight", "");
    return l->get_maxweight();
}

om_weight
FilterPostList::recalc_maxweight()
{
    DEBUGCALL(MATCH, om_weight, "FilterPostList::recalc_maxweight", "");
    lmax = l->recalc_maxweight();
    return lmax;
}

std::string
FilterPostList::get_description() const
{
    return "(" + l->get_description() + " Filter " + r->get_description() + ")";
}
