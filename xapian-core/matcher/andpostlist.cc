#include "andpostlist.h"

inline void
AndPostList::process_next_or_skip_to(weight w_min, PostList *ret)
{
    head = 0;
    if (ret) {
	delete r;
	r = ret;
    }
    if (r->at_end()) return;

    ret = l->skip_to(r->get_docid(), w_min);
    if (ret) {
	delete l;
	l = ret;
    }
    if (l->at_end()) return;

    docid lhead = l->get_docid();
    docid rhead = r->get_docid();

    while (lhead != rhead) {
	if (lhead < rhead) {
	    PostList *ret = l->skip_to(rhead, w_min);
	    if (ret) {
		delete l;
	        l = ret;
	    }
	    if (l->at_end()) {
		head = 0;
		return;
	    }
	    lhead = l->get_docid();	    
	} else {
	    PostList *ret = r->skip_to(lhead, w_min);
	    if (ret) {
		delete r;
		r = ret;
	    }
	    if (r->at_end()) {
		head = 0;
		return;
	    }
	    rhead = r->get_docid();	    
	}
    }

    head = lhead;
    return;
}

AndPostList::AndPostList(PostList *left, PostList *right)
{    
    l = left;
    r = right;
    lmax = l->get_maxweight();
    rmax = r->get_maxweight();
    head = 0;
}

// left and right are already running so just move them into sync
// FIXME: rename process_next_or_skip_to to this?
PostList *
AndPostList::flying_start(weight w_min)
{    
    process_next_or_skip_to(w_min, NULL);
    return NULL;
}

PostList *
AndPostList::next(weight w_min)
{
    process_next_or_skip_to(w_min, r->next(w_min));
    return NULL;
}

PostList *
AndPostList::skip_to(docid id, weight w_min)
{
    process_next_or_skip_to(w_min, r->skip_to(id, w_min));
    return NULL;
}
