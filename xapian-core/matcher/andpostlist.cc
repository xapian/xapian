#include "andpostlist.h"

inline void
AndPostList::advance_to_next_match()
{
    lhead = l->get_docid();
    rhead = r->get_docid();

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
    if (!l->at_end() && !r->at_end()) advance_to_next_match();
}

void
AndPostList::next()
{
    lhead = rhead = 0;

    r->next();
    if (r->at_end()) return;

    l->skip_to(r->get_docid());
    if (l->at_end()) return;

    advance_to_next_match();
}

void
AndPostList::skip_to(docid id)
{
    lhead = rhead = 0;

    r->skip_to(id);
    if (r->at_end()) return;

    l->skip_to(r->get_docid());
    if (l->at_end()) return;

    advance_to_next_match();
}
