/* filterpostlist.cc: apply a boolean posting list as a filter to a
 * probabilistic posting list
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003 Olly Betts
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
#include "filterpostlist.h"

inline void
FilterPostList::process_next_or_skip_to(Xapian::weight w_min, PostList *ret)
{
    DEBUGLINE(MATCH, "ret = " << (void*)ret);
    head = 0;
    // Don't call handle_prune - since r contributes no weight, we don't
    // need to recalc_maxweight
    if (ret) {
	delete r;
	r = ret;
    }
    DEBUGLINE(MATCH, "r at_end = " << r->at_end());
    if (r->at_end()) return;

    // r has just been advanced by next or skip_to so must be > head
    // (and head is the current position of l)
    Xapian::docid rhead = r->get_docid();
    DEBUGLINE(MATCH, "rhead " << rhead);
    DEBUGLINE(MATCH, "w_min " << w_min);
    skip_to_handling_prune(l, rhead, w_min, matcher);
    DEBUGLINE(MATCH, "l at_end = " << l->at_end());
    if (l->at_end()) return;

    Xapian::docid lhead = l->get_docid();
    DEBUGLINE(MATCH, "lhead " << lhead);

    while (lhead != rhead) {
	if (lhead < rhead) {
	    skip_to_handling_prune(l, rhead, w_min, matcher);
	    DEBUGLINE(MATCH, "l at_end = " << l->at_end());
	    if (l->at_end()) return;
	    lhead = l->get_docid();
	    DEBUGLINE(MATCH, "lhead " << lhead);
	} else {
	    PostList *p = r->skip_to(lhead, 0);
	    if (p) {
		delete r;
		r = p;
	    }
	    DEBUGLINE(MATCH, "r at_end = " << r->at_end());
	    if (r->at_end()) return;
	    rhead = r->get_docid();
	    DEBUGLINE(MATCH, "rhead " << rhead);
	}
    }

    head = lhead;
    return;
}

PostList *
FilterPostList::next(Xapian::weight w_min)
{
    DEBUGCALL(MATCH, PostList *, "FilterPostList::next", w_min);
    process_next_or_skip_to(w_min, r->next(0));
    RETURN(NULL);
}

PostList *
FilterPostList::skip_to(Xapian::docid did, Xapian::weight w_min)
{
    DEBUGCALL(MATCH, PostList *, "FilterPostList::skip_to", did << ", " << w_min);
    if (did > head) process_next_or_skip_to(w_min, r->skip_to(did, 0));
    RETURN(NULL);
}

Xapian::weight
FilterPostList::get_weight() const
{
    DEBUGCALL(MATCH, Xapian::weight, "FilterPostList::get_weight", "");
    RETURN(l->get_weight());
}

Xapian::weight
FilterPostList::get_maxweight() const
{
    DEBUGCALL(MATCH, Xapian::weight, "FilterPostList::get_maxweight", "");
    RETURN(lmax);
}

Xapian::weight
FilterPostList::recalc_maxweight()
{
    DEBUGCALL(MATCH, Xapian::weight, "FilterPostList::recalc_maxweight", "");
    lmax = l->recalc_maxweight();
    RETURN(lmax);
}

std::string
FilterPostList::get_description() const
{
    return "(" + l->get_description() + " Filter " + r->get_description() + ")";
}
