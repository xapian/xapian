#include "orpostlist.h"
#include "andpostlist.h"
#include "andmaybepostlist.h"
#include <stdio.h>

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
		ret = new AndPostList(l, r, root);
	    } else {
		printf("OR -> AND MAYBE (1)\n");
		ret = new AndMaybePostList(r, l, root);
	    }
	} else {
	    // w_min > rmax since w_min > minmax but not (w_min > lmax)
	    Assert(w_min > rmax);
	    printf("OR -> AND MAYBE (2)\n");
	    ret = new AndMaybePostList(l, r, root);
	}
		
	PostList *ret2 = ret->next(w_min);
	l = r = NULL;
	if (ret2) {
	    delete ret;
	    ret = ret2;
	}
	return ret;
    }

    if (lhead <= rhead) {
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
		ret = new AndPostList(l, r, root);
	    } else {
		printf("OR -> AND MAYBE (in skip_to) (1)\n");
		ret = new AndMaybePostList(r, l, root);
	    }
	} else {
	    // w_min > rmax since w_min > minmax but not (w_min > lmax)
	    Assert(w_min > rmax);
	    printf("OR -> AND MAYBE (in skip_to) (2)\n");
	    ret = new AndMaybePostList(l, r, root);
	}
		
	PostList *ret2 = ret->skip_to(id, w_min);
	l = r = NULL;
	if (ret2) {
	    delete ret;
	    ret = ret2;
	}
	return ret;
    }

    handle_prune(l, l->skip_to(id, w_min));
    bool ldry = l->at_end();

    handle_prune(r, r->skip_to(id, w_min));

    if (r->at_end()) {
	PostList *ret = l;
	l = NULL;
	return ret;
    }
    rhead = r->get_docid();

    if (!ldry) {
	lhead = l->get_docid();
	return NULL;
    }
    
    PostList *ret = r;
    r = NULL;
    return ret;
}
