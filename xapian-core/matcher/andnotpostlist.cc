#include "andnotpostlist.h"

inline void
AndNotPostList::advance_to_next_match()
{
    while (rhead <= lhead) {
	if (lhead == rhead) {
	    l->next();
	    if (l->at_end()) {
		lhead = 0;
		return;
	    }
	    lhead = r->get_docid();
	}		
	r->skip_to(lhead);
	if (r->at_end()) {
	    rhead = 0;
	    return;
	}
	rhead = r->get_docid();
    }
}

AndNotPostList::AndNotPostList(PostList *left, PostList *right)
{    
    l = left;
    r = right;
    lhead = rhead = 0;
    if (!l->at_end()) {
	lhead = l->get_docid();
	if (!r->at_end()) {
	    rhead = r->get_docid();
	    advance_to_next_match();
	}
    }
}

void
AndNotPostList::next()
{
    l->next();
    if (l->at_end()) {
	lhead = 0;
	return;
    }

    lhead = l->get_docid();
    if (rhead) advance_to_next_match();
}

void
AndNotPostList::skip_to(docid id)
{
    l->skip_to(id);
    if (l->at_end()) {
	lhead = 0;
	return;
    }
    
    lhead = l->get_docid();
    if (rhead) advance_to_next_match();
}
