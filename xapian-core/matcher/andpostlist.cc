#include "andpostlist.h"

inline void
AndPostList::process_next_or_skip_to(weight w_min, PostList *ret)
{
    head = 0;
    handle_prune(r, ret);
    if (r->at_end()) return;

    // r has just been advanced by next or skip_to so must be > head
    // (and head is the current position of l)
    handle_prune(l, l->skip_to(r->get_docid(), w_min));
    if (l->at_end()) return;

    docid lhead = l->get_docid();
    docid rhead = r->get_docid();

    while (lhead != rhead) {
	if (lhead < rhead) {
	    handle_prune(l, l->skip_to(rhead, w_min));
	    if (l->at_end()) {
		head = 0;
		return;
	    }
	    lhead = l->get_docid();	    
	} else {
	    handle_prune(r, r->skip_to(lhead, w_min));
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

AndPostList::AndPostList(PostList *left, PostList *right, Match *root_,
			 bool replacement)
{
    root = root_;
    l = left;
    r = right;
    head = 0;
    if (replacement) {
	// Initialise the maxweights from the kids so we can avoid forcing
	// a full maxweight recalc
	lmax = l->get_maxweight();
	rmax = r->get_maxweight();
    }
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
    if (id > head) process_next_or_skip_to(w_min, r->skip_to(id, w_min));
    return NULL;
}
