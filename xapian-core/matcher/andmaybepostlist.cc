#include "andmaybepostlist.h"
#include "andpostlist.h"

AndMaybePostList::AndMaybePostList(PostList *left, PostList *right, Match *root_)
{
    root = root_;
    l = left;
    r = right;
    lhead = rhead = 0;
}

PostList *
AndMaybePostList::next(weight w_min)
{
    if (w_min > lmax) {
	// we can replace the AND MAYBE with an AND
	PostList *ret;
	cout << "AND MAYBE -> AND\n";
	ret = new AndPostList(l, r, root);
	l = r = NULL;
	PostList *ret2 = ret->next(w_min);
	if (ret2) {
	    delete ret;
	    ret = ret2;
	}
	return ret;
    }

    if (lhead == rhead) {
	handle_prune(r, r->next(w_min));
        if (r->at_end()) {
	    PostList *ret = l;
	    l = NULL;
	    return ret;
	}
	rhead = r->get_docid();
    }
    
    handle_prune(l, l->next(w_min));
    if (!l->at_end()) {
	lhead = l->get_docid();	
    } else {
	// no point returning r - l is required
	lhead = 0;
    }
    return NULL;
}

PostList *
AndMaybePostList::sync_and_skip_to(docid id, weight w_min, docid lh, docid rh)
{
    lhead = lh;
    rhead = rh;
    return skip_to(id, w_min);
}

PostList *
AndMaybePostList::skip_to(docid id, weight w_min)
{
    if (w_min > lmax) {
	// we can replace the AND MAYBE with an AND
	PostList *ret;
	cout << "AND MAYBE -> AND (in skip_to)\n";
	ret = new AndPostList(l, r, root);
	id = max(id, max(lhead, rhead));
	l = r = NULL;
	PostList *ret2 = ret->skip_to(id, w_min);
	if (ret2) {
	    delete ret;
	    ret = ret2;
	}
	return ret;
    }

    // exit if we're already past the skip point
    if (id <= lhead) return NULL;

    handle_prune(l, l->skip_to(id, w_min));
    if (l->at_end()) {
	// once l is over, so is the AND MAYBE
	lhead = 0;
	return NULL;
    }
    
    docid lhead_new = l->get_docid();
    
    if (lhead == rhead) {
	handle_prune(r, r->skip_to(lhead_new, w_min));
        if (r->at_end()) {
	    PostList *ret = l;
	    l = NULL;
	    return ret;
	}
	rhead = r->get_docid();
    }

    lhead = lhead_new;
    return NULL;
}
