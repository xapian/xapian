#include "andnotpostlist.h"

inline PostList *
AndNotPostList::advance_to_next_match()
{
    if (l->at_end()) {
	lhead = 0;
	return NULL;
    }
    lhead = l->get_docid();

    while (rhead <= lhead) {
	if (lhead == rhead) {
	    l->next();
	    if (l->at_end()) {
		lhead = 0;
		return NULL;
	    }
	    lhead = l->get_docid();
	}		
	r->skip_to(lhead);
	if (r->at_end()) {
	    l = NULL;
	    return l;
	}
	rhead = r->get_docid();
    }
    return NULL;
}

AndNotPostList::AndNotPostList(PostList *left, PostList *right)
{    
    l = left;
    r = right;
    lhead = rhead = 0;
}

PostList *
AndNotPostList::next()
{
    l->next();
    return advance_to_next_match();    
}

PostList *
AndNotPostList::skip_to(docid id)
{
    l->skip_to(id);
    return advance_to_next_match();
}
