#include "andpostlist.h"

inline void
AndPostList::advance_to_next_match()
{
    docid lhead = l->get_docid();
    docid rhead = r->get_docid();

    while (lhead != rhead) {
	if (lhead < rhead) {
 	    try { l->skip_to(rhead); } catch (PostList **p) { catch_kid(l, p); }
	    if (l->at_end()) {
		head = 0;
		return;
	    }
	    lhead = l->get_docid();	    
	} else {
 	    try { r->skip_to(lhead); } catch (PostList **p) { catch_kid(r, p); }
	    if (r->at_end()) {
		head = 0;
		return;
	    }
	    rhead = r->get_docid();	    
	}
    }

    head = lhead;
}

AndPostList::AndPostList(PostList *left, PostList *right)
{    
    l = left;
    r = right;
    head = 0;
    if (!l->at_end() && !r->at_end()) advance_to_next_match();
}

void
AndPostList::next()
{
    head = 0;

    try { r->next(); } catch (PostList **p) { catch_kid(r, p); }
    if (r->at_end()) return;

    try { l->skip_to(r->get_docid()); } catch (PostList **p) { catch_kid(l, p); }
    if (l->at_end()) return;

    advance_to_next_match();
}

void
AndPostList::skip_to(docid id)
{
    head = 0;

    try { r->skip_to(id); } catch (PostList **p) { catch_kid(r, p); }
    if (r->at_end()) return;

    try { l->skip_to(r->get_docid()); } catch (PostList **p) { catch_kid(l, p); }
    if (l->at_end()) return;

    advance_to_next_match();
}
