#include "andmaybepostlist.h"
#include "andpostlist.h"
#include "omassert.h"

inline PostList *
AndMaybePostList::process_next_or_skip_to(weight w_min, PostList *ret)
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
				   Match *root_, docid lh, docid rh)
{
    root = root_;
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
AndMaybePostList::next(weight w_min)
{
    if (w_min > lmax) {
	// we can replace the AND MAYBE with an AND
	PostList *ret;
	DebugMsg("AND MAYBE -> AND" << endl);
	ret = new AndPostList(l, r, root, true);
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
AndMaybePostList::skip_to(docid id, weight w_min)
{
    if (w_min > lmax) {
	// we can replace the AND MAYBE with an AND
	PostList *ret;
	DebugMsg("AND MAYBE -> AND (in skip_to)" << endl);
	ret = new AndPostList(l, r, root, true);
	id = max(id, max(lhead, rhead));
	l = r = NULL;
	PostList *ret2 = ret->skip_to(id, w_min);
	if (ret2) {
	    delete ret;
	    ret = ret2;
	}
	return ret;
    }

    // exit if we're already past the skip point (or at it)
    if (id <= lhead) return NULL;

    return process_next_or_skip_to(w_min, l->skip_to(id, w_min - rmax));
}
