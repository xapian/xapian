/* xorpostlist.cc: XOR of two posting lists
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

#include "xorpostlist.h"
#include "andnotpostlist.h"

// for XOR we just pass w_min through unchanged since both branches matching
// doesn't cause a match

inline PostList *
XorPostList::advance_to_next_match(om_weight w_min)
{
    while (rhead == lhead) {
	handle_prune(l, l->next(w_min));
	handle_prune(r, r->next(w_min));
	if (l->at_end()) {
	    if (r->at_end()) {
		lhead = 0;
		return NULL;
	    }
	    PostList *ret = r;
	    r = NULL;
	    return ret;
	}
	if (r->at_end()) {
	    PostList *ret = l;
	    l = NULL;
	    return ret;
	}
	lhead = l->get_docid();
	rhead = r->get_docid();
    }
    return NULL;
}

XorPostList::XorPostList(PostList *left, PostList *right, LocalMatch *matcher_)
{
    matcher = matcher_;
    l = left;
    r = right;
    lhead = rhead = 0;
}

PostList *
XorPostList::next(om_weight w_min)
{
    if (w_min > minmax) {
	// we can replace the XOR with another operator (or run dry)
	PostList *ret;
	if (w_min > lmax) {
	    if (w_min > rmax) {
		DebugMsg("XOR drops below w_min" << endl);
		// neither side is weighty enough, so run dry
		lhead = 0;
		return NULL;
	    }
	    DebugMsg("XOR -> AND NOT (1)" << endl);
	    ret = new AndNotPostList(r, l, matcher);
	} else {
	    // w_min > rmax since w_min > minmax but not (w_min > lmax)
	    Assert(w_min > rmax);
	    DebugMsg("XOR -> AND NOT (2)" << endl);
	    ret = new AndNotPostList(l, r, matcher);
	}
		
	PostList *ret2 = ret->next(w_min);
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
	// lhead == rhead should only happen on first next
        if (lhead == rhead) rnext = true;
        handle_prune(l, l->next(w_min));
	if (l->at_end()) ldry = true;
    } else {
	rnext = true;
    }
    
    if (rnext) {
        handle_prune(r, r->next(w_min));
        if (r->at_end()) {
	    PostList *ret = l;
	    l = NULL;
	    return ret;
	}
	rhead = r->get_docid();
    }

    if (ldry) {
	PostList *ret = r;
	r = NULL;
	return ret;
    }

    lhead = l->get_docid();
    return advance_to_next_match(w_min);
}

PostList *
XorPostList::skip_to(om_docid did, om_weight w_min)
{
    if (w_min > minmax) {
	// we can replace the XOR with another operator (or run dry)
	PostList *ret, *ret2;
	if (w_min > lmax) {
	    if (w_min > rmax) {
		DebugMsg("XOR drops below w_min" << endl);
		// neither side is weighty enough, so run dry
		lhead = 0;
		return NULL;
	    }
	    DebugMsg("XOR -> AND NOT (in skip_to) (1)" << endl);
	    AndNotPostList *ret3 = new AndNotPostList(r, l, matcher);
	    did = max(did, rhead);
	    ret2 = ret3->sync_and_skip_to(did, w_min, rhead, lhead);
	    ret = ret3;
	} else {
	    // w_min > rmax since w_min > minmax but not (w_min > lmax)
	    Assert(w_min > rmax);
	    DebugMsg("XOR -> AND NOT (in skip_to) (2)" << endl);
	    AndNotPostList *ret3 = new AndNotPostList(l, r, matcher);
	    did = max(did, lhead);
	    ret2 = ret3->sync_and_skip_to(did, w_min, lhead, rhead);
	    ret = ret3;
	}
		
	l = r = NULL;
	if (ret2) {
	    delete ret;
	    ret = ret2;
	}
	return ret;
    }

    bool ldry = false;
    if (lhead < did) {
	handle_prune(l, l->skip_to(did, w_min));
	ldry = l->at_end();
    }

    if (rhead < did) {
	handle_prune(r, r->skip_to(did, w_min));

	if (r->at_end()) {
	    PostList *ret = l;
	    l = NULL;
	    return ret;
	}
	rhead = r->get_docid();
    }

    if (ldry) {
	PostList *ret = r;
	r = NULL;
	return ret;
    }
    
    lhead = l->get_docid();
    return advance_to_next_match(w_min);
}
