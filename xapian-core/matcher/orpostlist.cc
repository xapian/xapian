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

OrPostList::OrPostList(PostList *left_,
		       PostList *right_,
		       MultiMatch *matcher_,
		       om_doccount dbsize_)
	: BranchPostList(left_, right_, matcher_),
	  lhead(0), rhead(0), lmax(0), rmax(0), minmax(0), dbsize(dbsize_)
{
}

PostList *
OrPostList::next(om_weight w_min)
{
    if (w_min > minmax) {
	// we can replace the OR with another operator
	PostList *ret;
	if (w_min > lmax) {
	    if (w_min > rmax) {
		DEBUGLINE(MATCH, "OR -> AND");
		ret = new AndPostList(l, r, matcher, true);
		skip_to_handling_prune(ret, std::max(lhead, rhead) + 1, w_min,
				       matcher);
	    } else {
		DEBUGLINE(MATCH, "OR -> AND MAYBE (1)");
		ret = new AndMaybePostList(r, l, matcher, rhead, lhead);
		next_handling_prune(ret, w_min, matcher);
	    }
	} else {
	    // w_min > rmax since w_min > minmax but not (w_min > lmax)
	    Assert(w_min > rmax);
	    DEBUGLINE(MATCH, "OR -> AND MAYBE (2)");
	    ret = new AndMaybePostList(l, r, matcher, lhead, rhead);
	    next_handling_prune(ret, w_min, matcher);
	}

	l = r = NULL;
	return ret;
    }

    bool ldry = false;
    bool rnext = false;

    if (lhead <= rhead) {
        if (lhead == rhead) rnext = true;
        next_handling_prune(l, w_min - rmax, matcher);
	if (l->at_end()) ldry = true;
    } else {
	rnext = true;
    }

    if (rnext) {
        next_handling_prune(r, w_min - lmax, matcher);
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
		DEBUGLINE(MATCH, "OR -> AND (in skip_to)");
		ret = new AndPostList(l, r, matcher, true);
		did = std::max(did, std::max(lhead, rhead));
	    } else {
		DEBUGLINE(MATCH, "OR -> AND MAYBE (in skip_to) (1)");
		ret = new AndMaybePostList(r, l, matcher, rhead, lhead);
		did = std::max(did, rhead);
	    }
	} else {
	    // w_min > rmax since w_min > minmax but not (w_min > lmax)
	    Assert(w_min > rmax);
	    DEBUGLINE(MATCH, "OR -> AND MAYBE (in skip_to) (2)");
	    ret = new AndMaybePostList(l, r, matcher, lhead, rhead);
	    did = std::max(did, lhead);
	}

	l = r = NULL;
	skip_to_handling_prune(ret, did, w_min, matcher);
	return ret;
    }

    bool ldry = false;
    if (lhead < did) {
	skip_to_handling_prune(l, did, w_min - rmax, matcher);
	ldry = l->at_end();
    }

    if (rhead < did) {
	skip_to_handling_prune(r, did, w_min - lmax, matcher);

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
