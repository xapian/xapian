#include "andpostlist.h"

inline void
AndPostList::advance_to_next_match()
{
    docid lhead = l->get_docid();
    docid rhead = r->get_docid();

    while (lhead != rhead) {
	if (lhead < rhead) {
	    l->skip_to(rhead);
	    if (l->at_end()) {
		lhead = rhead = 0;
		return;
	    }
	    lhead = l->get_docid();	    
	} else {
	    r->skip_to(lhead);
	    if (r->at_end()) {
		lhead = rhead = 0;
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

AndPostList::~AndPostList()
{
    delete l;
    delete r;
}

void
AndPostList::next()
{
    head = 0;

    r->next();
    if (r->at_end()) return;

    l->skip_to(r->get_docid());
    if (l->at_end()) return;

    advance_to_next_match();
}

void
AndPostList::skip_to(docid id)
{
    head = 0;

    r->skip_to(id);
    if (r->at_end()) return;

    l->skip_to(r->get_docid());
    if (l->at_end()) return;

    advance_to_next_match();
}
