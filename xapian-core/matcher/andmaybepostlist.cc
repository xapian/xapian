/* andmaybepostlist.cc: Merged postlist; items from one list, weights from both
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

#include "andmaybepostlist.h"
#include "andpostlist.h"
#include "omassert.h"

inline PostList *
AndMaybePostList::process_next_or_skip_to(om_weight w_min, PostList *ret)
{
    handle_prune(l, ret);
    if (l->at_end()) {
	// once l is over, so is the AND MAYBE
	lhead = 0;
	return NULL;
    }
	
    lhead = l->get_docid();
    if (lhead <= rhead) return NULL;
    
    handle_prune(r, r->skip_to(lhead, w_min - lmax));
    if (r->at_end()) {
	PostList *ret = l;
	l = NULL;
	return ret;
    }
    rhead = r->get_docid();
    return NULL;
}

AndMaybePostList::AndMaybePostList(PostList *left, PostList *right,
				   LocalMatch *matcher_,
				   om_docid lh, om_docid rh)
{
    matcher = matcher_;
    l = left;
    r = right;
    lhead = lh;
    rhead = rh;
    if (lh || rh) {
	// Initialise the maxweights from the kids so we can avoid forcing
	// a full maxweight recalc
	lmax = l->get_maxweight();
	rmax = r->get_maxweight();	
    }
}

PostList *
AndMaybePostList::next(om_weight w_min)
{
    if (w_min > lmax) {
	// we can replace the AND MAYBE with an AND
	PostList *ret;
	DebugMsg("AND MAYBE -> AND" << endl);
	ret = new AndPostList(l, r, matcher, true);
	l = r = NULL;
	PostList *ret2 = ret->skip_to(max(lhead, rhead) + 1, w_min);
	if (ret2) {
	    delete ret;
	    ret = ret2;
	}
	return ret;
    }
    return process_next_or_skip_to(w_min, l->next(w_min - rmax));
}

PostList *
AndMaybePostList::skip_to(om_docid did, om_weight w_min)
{
    if (w_min > lmax) {
	// we can replace the AND MAYBE with an AND
	PostList *ret;
	DebugMsg("AND MAYBE -> AND (in skip_to)" << endl);
	ret = new AndPostList(l, r, matcher, true);
	did = max(did, max(lhead, rhead));
	l = r = NULL;
	PostList *ret2 = ret->skip_to(did, w_min);
	if (ret2) {
	    delete ret;
	    ret = ret2;
	}
	return ret;
    }

    // exit if we're already past the skip point (or at it)
    if (did <= lhead) return NULL;

    return process_next_or_skip_to(w_min, l->skip_to(did, w_min - rmax));
}
