/* andnotpostlist.cc: Return items which are in A, unless they're in B
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

#include "andnotpostlist.h"

inline PostList *
AndNotPostList::advance_to_next_match(weight w_min, PostList *ret)
{
    handle_prune(l, ret);
    if (l->at_end()) {
	lhead = 0;
	return NULL;
    }
    lhead = l->get_docid();

    while (rhead <= lhead) {
	if (lhead == rhead) {
	    handle_prune(l, l->next(w_min));
	    if (l->at_end()) {
		lhead = 0;
		return NULL;
	    }
	    lhead = l->get_docid();
	}		
	handle_prune(r, r->skip_to(lhead, 0));
	if (r->at_end()) {
	    ret = l;
	    l = NULL;
	    return ret;
	}
	rhead = r->get_docid();
    }
    return NULL;
}

AndNotPostList::AndNotPostList(PostList *left, PostList *right, OMMatch *root_)
{    
    root = root_;
    l = left;
    r = right;
    lhead = rhead = 0;
}

PostList *
AndNotPostList::next(weight w_min)
{
    return advance_to_next_match(w_min, l->next(w_min));
}

PostList *
AndNotPostList::sync_and_skip_to(docid id, weight w_min, docid lh, docid rh)
{
    lhead = lh;
    rhead = rh;
    return skip_to(id, w_min);
}

PostList *
AndNotPostList::skip_to(docid did, weight w_min)
{
    if (did <= lhead) return NULL;
    return advance_to_next_match(w_min, l->skip_to(did, w_min));
}
