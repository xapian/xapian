#include "orpostlist.h"
#include "andpostlist.h"
#include "andmaybepostlist.h"
#include <stdio.h>

OrPostList::OrPostList(PostList *left, PostList *right)
{
    l = left;
    r = right;
    lmax = l->get_maxweight();
    rmax = r->get_maxweight();
    minmax = min(lmax, rmax);
    lhead = rhead = 0;
}

PostList *
OrPostList::next(weight w_min)
{
    bool ldry = false;
    bool rnext = false;

    if (w_min > minmax) {
	printf("minmax %f lmax %f rmax %f w_min %f\n",
	       minmax, lmax, rmax, w_min);
	// we can replace the OR with another operator
	PostList *ret;
	if (w_min > lmax) {
	    if (w_min > rmax) {
		printf("OR -> AND\n");
		ret = new AndPostList(l, r);
	    } else {
		printf("OR -> AND MAYBE (1)\n");
		ret = new AndMaybePostList(r, l);
	    }
	} else {
	    // w_min > rmax since w_min > minmax but not (w_min > lmax)
	    Assert(w_min > rmax);
	    printf("OR -> AND MAYBE (2)\n");
	    ret = new AndMaybePostList(l, r);
	}
		
	l = r = NULL;
	PostList *ret2 = ret->next(w_min);
	if (ret2) {
	    delete ret;
	    ret = ret2;
	}
	return ret;
    }

    if (lhead <= rhead) {
        if (lhead == rhead) rnext = true;

	PostList *ret;
	ret = l->next(w_min);
        if (ret) {
	    delete l;
	    l = ret;
	}
	if (l->at_end()) ldry = true;
    } else {
	rnext = true;
    }

    if (rnext) {
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
		printf("OR -> AND (in skip_to)\n");
		ret = new AndPostList(l, r);
	    } else {
		printf("OR -> AND MAYBE (in skip_to) (1)\n");
		ret = new AndMaybePostList(r, l);
	    }
	} else {
	    // w_min > rmax since w_min > minmax but not (w_min > lmax)
	    Assert(w_min > rmax);
	    printf("OR -> AND MAYBE (in skip_to) (2)\n");
	    ret = new AndMaybePostList(l, r);
	}
		
	l = r = NULL;
	PostList *ret2 = ret->skip_to(id, w_min);
	if (ret2) {
	    delete ret;
	    ret = ret2;
	}
	return ret;
    }

    bool ldry;
    PostList *ret;

    ret = l->skip_to(id, w_min);
    if (ret) {
	delete l;
	l = ret;
    }
    ldry = l->at_end();

    ret = r->skip_to(id, w_min);
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

    if (!ldry) {
	lhead = l->get_docid();
	return NULL;
    }
    
    ret = r;
    r = NULL;
    return ret;
}
