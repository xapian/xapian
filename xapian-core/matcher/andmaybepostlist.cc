#include "andmaybepostlist.h"
#include "andpostlist.h"
#include <stdio.h>

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
	printf("AND MAYBE -> AND\n");
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
AndMaybePostList::skip_to(docid id, weight w_min)
{
    if (w_min > lmax) {
	// we can replace the AND MAYBE with an AND
	PostList *ret;
	printf("AND MAYBE -> AND (in skip_to)\n");
	ret = new AndPostList(l, r, root);
	l = r = NULL;
	PostList *ret2 = ret->skip_to(id, w_min);
	if (ret2) {
	    delete ret;
	    ret = ret2;
	}
	return ret;
    }

    handle_prune(l, l->skip_to(id, w_min));
    if (l->at_end()) {
	// once l is over, so is the AND MAYBE
	lhead = 0;
	return NULL;
    }
    
    bool rskip = false;
    if (lhead == rhead) rskip = true;
    
    lhead = l->get_docid();
    
    if (rskip) {
	handle_prune(r, r->skip_to(lhead, w_min));
        if (r->at_end()) {
	    PostList *ret = l;
	    l = NULL;
	    return ret;
	}
	rhead = r->get_docid();
    }
    return NULL;
}
