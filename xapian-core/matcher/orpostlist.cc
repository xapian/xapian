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
    bool ldry = false;
    bool rnext = false;

    if (w_min > minmax) {
	cout << "minmax " << minmax << " lmax " << lmax << " rmax " << rmax
	     << " w_min " << w_min << endl;
	// we can replace the OR with another operator
	PostList *ret;
	if (w_min > lmax) {
	    if (w_min > rmax) {
		cout << "OR -> AND\n";
		ret = new AndPostList(l, r, root);
	    } else {
		cout << "OR -> AND MAYBE (1)\n";
		ret = new AndMaybePostList(r, l, root);
	    }
	} else {
	    // w_min > rmax since w_min > minmax but not (w_min > lmax)
	    Assert(w_min > rmax);
	    cout << "OR -> AND MAYBE (2)\n";
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
	PostList *ret2;
	if (w_min > lmax) {
	    if (w_min > rmax) {
		cout << "OR -> AND (in skip_to)\n";
		ret = new AndPostList(l, r, root);
		id = max(id, max(lhead, rhead));
		ret2 = ret->skip_to(id, w_min);
	    } else {
		cout << "OR -> AND MAYBE (in skip_to) (1)\n";
		AndMaybePostList *ret3 = new AndMaybePostList(r, l, root);
		id = max(id, rhead);
		ret2 = ret3->sync_and_skip_to(id, w_min, rhead, lhead);
		ret = ret3;
	    }
	} else {
	    // w_min > rmax since w_min > minmax but not (w_min > lmax)
	    Assert(w_min > rmax);
	    cout << "OR -> AND MAYBE (in skip_to) (2)\n";
	    AndMaybePostList *ret3 = new AndMaybePostList(l, r, root);
	    id = max(id, lhead);
	    ret2 = ret3->sync_and_skip_to(id, w_min, lhead, rhead);
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
    if (lhead < id) {
	handle_prune(l, l->skip_to(id, w_min));
	ldry = l->at_end();
    }

    if (rhead < id) {
	handle_prune(r, r->skip_to(id, w_min));

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
