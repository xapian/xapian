/* orpostlist.cc: OR of two posting lists
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#include "orpostlist.h"
#include "andpostlist.h"
#include "andmaybepostlist.h"
#include "omdebug.h"

OrPostList::OrPostList(PostList *left, PostList *right, LocalMatch *matcher_)
{
    matcher = matcher_;
    l = left;
    r = right;
    lhead = rhead = 0;
}

PostList *
OrPostList::next(om_weight w_min)
{
    if (w_min > minmax) {
	// we can replace the OR with another operator
	PostList *ret, *ret2;
	if (w_min > lmax) {
	    if (w_min > rmax) {
		DebugMsg("OR -> AND" << endl);
		ret = new AndPostList(l, r, matcher, true);
		ret2 = ret->skip_to(std::max(lhead, rhead) + 1, w_min);
	    } else {
		DebugMsg("OR -> AND MAYBE (1)" << endl);
		ret = new AndMaybePostList(r, l, matcher, rhead, lhead);
		ret2 = ret->next(w_min);
	    }
	} else {
	    // w_min > rmax since w_min > minmax but not (w_min > lmax)
	    Assert(w_min > rmax);
	    DebugMsg("OR -> AND MAYBE (2)" << endl);
	    ret = new AndMaybePostList(l, r, matcher, lhead, rhead);
	    ret2 = ret->next(w_min);
	}
		
	l = r = NULL;
	if (ret2) {
	    delete ret;
	    ret = ret2;
	}
	return ret;
    }

    bool ldry = false;
    bool rnext = false;

    if (lhead <= rhead) {
        if (lhead == rhead) rnext = true;
        handle_prune(l, l->next(w_min - rmax));
	if (l->at_end()) ldry = true;
    } else {
	rnext = true;
    }

    if (rnext) {
        handle_prune(r, r->next(w_min - lmax));
        if (r->at_end()) {
	    PostList *ret = l;
	    l = NULL;
	    return ret;
	}
	rhead = r->get_docid();
    }

    if (!ldry) {
	lhead = l->get_docid();
	return NULL;
    }
    
    PostList *ret = r;
    r = NULL;
    return ret;
}

PostList *
OrPostList::skip_to(om_docid did, om_weight w_min)
{
    if (w_min > minmax) {
	// we can replace the OR with another operator
	PostList *ret;
	if (w_min > lmax) {
	    if (w_min > rmax) {
		DebugMsg("OR -> AND (in skip_to)" << endl);
		ret = new AndPostList(l, r, matcher, true);
		did = std::max(did, std::max(lhead, rhead));
	    } else {
		DebugMsg("OR -> AND MAYBE (in skip_to) (1)" << endl);
		ret = new AndMaybePostList(r, l, matcher, rhead, lhead);
		did = std::max(did, rhead);
	    }
	} else {
	    // w_min > rmax since w_min > minmax but not (w_min > lmax)
	    Assert(w_min > rmax);
	    DebugMsg("OR -> AND MAYBE (in skip_to) (2)" << endl);
	    ret = new AndMaybePostList(l, r, matcher, lhead, rhead);
	    did = std::max(did, lhead);
	}

	PostList *ret2 = ret->skip_to(did, w_min);	
	l = r = NULL;
	if (ret2) {
	    delete ret;
	    ret = ret2;
	}
	return ret;
    }

    bool ldry = false;
    if (lhead < did) {
	handle_prune(l, l->skip_to(did, w_min - rmax));
	ldry = l->at_end();
    }

    if (rhead < did) {
	handle_prune(r, r->skip_to(did, w_min - lmax));

	if (r->at_end()) {
	    PostList *ret = l;
	    l = NULL;
	    return ret;
	}
	rhead = r->get_docid();
    }

    if (!ldry) {
	lhead = l->get_docid();
	return NULL;
    }
    
    PostList *ret = r;
    r = NULL;
    return ret;
}
