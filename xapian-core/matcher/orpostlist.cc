#include "orpostlist.h"
#include "andpostlist.h"
#include "andmaybepostlist.h"

OrPostList::OrPostList(PostList *left, PostList *right, Match *root_)
{
    root = root_;
    l = left;
    r = right;
    lhead = rhead = 0;
}

PostList *
OrPostList::next(weight w_min)
{
    if (w_min > minmax) {
	// we can replace the OR with another operator
	PostList *ret, *ret2;
	if (w_min > lmax) {
	    if (w_min > rmax) {
		DebugMsg("OR -> AND" << endl);
		ret = new AndPostList(l, r, root, true);
		ret2 = ret->skip_to(max(lhead, rhead) + 1, w_min);
	    } else {
		DebugMsg("OR -> AND MAYBE (1)" << endl);
		ret = new AndMaybePostList(r, l, root, rhead, lhead);
		ret2 = ret->next(w_min);
	    }
	} else {
	    // w_min > rmax since w_min > minmax but not (w_min > lmax)
	    Assert(w_min > rmax);
	    DebugMsg("OR -> AND MAYBE (2)" << endl);
	    ret = new AndMaybePostList(l, r, root, lhead, rhead);
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
OrPostList::skip_to(docid id, weight w_min)
{
    if (w_min > minmax) {
	// we can replace the OR with another operator
	PostList *ret;
	if (w_min > lmax) {
	    if (w_min > rmax) {
		DebugMsg("OR -> AND (in skip_to)" << endl);
		ret = new AndPostList(l, r, root, true);
		id = max(id, max(lhead, rhead));
	    } else {
		DebugMsg("OR -> AND MAYBE (in skip_to) (1)" << endl);
		ret = new AndMaybePostList(r, l, root, rhead, lhead);
		id = max(id, rhead);
	    }
	} else {
	    // w_min > rmax since w_min > minmax but not (w_min > lmax)
	    Assert(w_min > rmax);
	    DebugMsg("OR -> AND MAYBE (in skip_to) (2)" << endl);
	    ret = new AndMaybePostList(l, r, root, lhead, rhead);
	    id = max(id, lhead);
	}

	PostList *ret2 = ret->skip_to(id, w_min);	
	l = r = NULL;
	if (ret2) {
	    delete ret;
	    ret = ret2;
	}
	return ret;
    }

    bool ldry = false;
    if (lhead < id) {
	handle_prune(l, l->skip_to(id, w_min - rmax));
	ldry = l->at_end();
    }

    if (rhead < id) {
	handle_prune(r, r->skip_to(id, w_min - lmax));

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
