#include "andnotpostlist.h"

inline void
AndNotPostList::advance_to_next_match()
{
    if (l->at_end()) {
	lhead = 0;
	return;
    }
    lhead = l->get_docid();

    while (rhead <= lhead) {
	if (lhead == rhead) {
	    l->next();
	    if (l->at_end()) {
		lhead = 0;
		return;
	    }
	    lhead = l->get_docid();
	}		
	r->skip_to(lhead);
	if (r->at_end()) throw &l;
	rhead = r->get_docid();
    }
}

AndNotPostList::AndNotPostList(PostList *left, PostList *right)
{    
    l = left;
    r = right;
    lhead = rhead = 0;
}

void
AndNotPostList::next()
{
    l->next();
    advance_to_next_match();
}

void
AndNotPostList::skip_to(docid id)
{
    l->skip_to(id);
    advance_to_next_match();
}
