#include "andmaybepostlist.h"
#include "andpostlist.h"
#include <stdio.h>

AndMaybePostList::AndMaybePostList(PostList *left, PostList *right)
{
    l = left;
    r = right;
    lmax = l->get_maxweight();
    rmax = r->get_maxweight();
    lhead = rhead = 0;
}

PostList *
AndMaybePostList::next(weight w_min)
{
    if (w_min > lmax) {
	// we can replace the AND MAYBE with an AND
	PostList *ret;
	printf("AND MAYBE -> AND\n");
	ret = new AndPostList(l, r);
	l = r = NULL;
	PostList *ret2 = ret->next(w_min);
	if (ret2) {
	    delete ret;
	    ret = ret2;
	}
	return ret;
    }

    if (lhead == rhead) {
	PostList *ret;
	ret = r->next(w_min);
        if (ret) {
	    delete r;
	    r = ret;
	}
        if (r->at_end()) {
	    ret = l;
	    l = NULL;
	    return ret;
	}
	rhead = r->get_docid();
    }
    
    PostList *ret;
    ret = l->next(w_min);
    if (ret) {
	delete l;
	l = ret;
    }
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
	ret = new AndPostList(l, r);
	l = r = NULL;
	PostList *ret2 = ret->skip_to(id, w_min);
	if (ret2) {
	    delete ret;
	    ret = ret2;
	}
	return ret;
    }

    PostList *ret;
    ret = l->skip_to(id, w_min);
    if (ret) {
	delete l;
	l = ret;
    }
    if (l->at_end()) {
	// once l is over, so is the AND MAYBE
	lhead = 0;
	return NULL;
    }
    
    bool rskip = false;
    if (lhead == rhead) rskip = true;
    
    lhead = l->get_docid();
    
    if (rskip) {
	ret = r->skip_to(lhead, w_min);
        if (ret) {
	    delete r;
	    r = ret;
	}
        if (r->at_end()) {
	    ret = l;
	    l = NULL;
	    return ret;
	}
	rhead = r->get_docid();
    }
    return NULL;
}
