#include "andpostlist.h"

inline void
AndPostList::advance_to_next_match()
{
    // if both l and r hit the end, lhead and rhead will both be zero
    // if we're already at the end, lhead and rhead will already be zero
    while (lhead != rhead) {
	if (lhead < rhead) {
	    l->skip_to(rhead);
	} else {
	    r->skip_to(lhead);
	}
    }    
}


AndPostList::AndPostList(PostList *left, PostList *right)
{    
    l = left;
    r = right;
    lhead = rhead = 0;
    if (!l->at_end() && !r->at_end()) {
	lhead = l->get_docid();
	rhead = r->get_docid();
	advance_to_next_match();
    }
}

void
AndPostList::next()
{
    l->next();
    r->next();
    advance_to_next_match();
}

void
AndPostList::skip_to(docid id)
{
    // FIXME: scope for optimisation here...
    l->skip_to(id);
    lhead = 0;
    if (!l->at_end()) lhead = l->get_docid();

    rhead = 0;
    if (!r->at_end()) rhead = r->get_docid();

    advance_to_next_match();
}
