#include "andpostlist.h"

inline void
AndPostList::process_next_or_skip_to(PostList *ret)
{
    head = 0;
    if (ret) {
	delete r;
	r = ret;
    }
    if (r->at_end()) return;

    ret = l->skip_to(r->get_docid());
    if (ret) {
	delete l;
	l = ret;
    }
    if (l->at_end()) return;

    docid lhead = l->get_docid();
    docid rhead = r->get_docid();

    while (lhead != rhead) {
	if (lhead < rhead) {
	    PostList *ret = l->skip_to(rhead);
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
	    PostList *ret = r->skip_to(lhead);
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
    head = 0;
}

PostList *
AndPostList::next()
{
    process_next_or_skip_to(r->next());
    return NULL;
}

PostList *
AndPostList::skip_to(docid id)
{
    process_next_or_skip_to(r->skip_to(id));
    return NULL;
}
