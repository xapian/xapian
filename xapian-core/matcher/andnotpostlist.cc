#include "andnotpostlist.h"

inline PostList *
AndNotPostList::advance_to_next_match(weight w_min, PostList *ret)
{
    handle_prune(l, ret);
    if (l->at_end()) {
	lhead = 0;
	return NULL;
    }
    lhead = l->get_docid();

    while (rhead <= lhead) {
	if (lhead == rhead) {
	    handle_prune(l, l->next(w_min));
	    if (l->at_end()) {
		lhead = 0;
		return NULL;
	    }
	    lhead = l->get_docid();
	}		
	handle_prune(r, r->skip_to(lhead, w_min));
	if (r->at_end()) {
	    ret = l;
	    l = NULL;
	    return ret;
	}
	rhead = r->get_docid();
    }
    return NULL;
}

AndNotPostList::AndNotPostList(PostList *left, PostList *right, Match *root_)
{    
    root = root_;
    l = left;
    r = right;
    lhead = rhead = 0;
}

PostList *
AndNotPostList::next(weight w_min)
{
    return advance_to_next_match(w_min, l->next(w_min));
}

PostList *
AndNotPostList::skip_to(docid id, weight w_min)
{
    return advance_to_next_match(w_min, l->skip_to(id, w_min));
}
