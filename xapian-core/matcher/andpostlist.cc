#include "andpostlist.h"

inline void
AndPostList::advance_to_next_match()
{
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
    PostList *ret;
    head = 0;

    ret = r->next();
    if (ret) {
	delete r;
	r = ret;
    }
    if (r->at_end()) return NULL;

    ret = l->skip_to(r->get_docid());
    if (ret) {
	delete l;
	l = ret;
    }
    if (l->at_end()) return NULL;

    advance_to_next_match();
    return NULL;
}

PostList *
AndPostList::skip_to(docid id)
{
    PostList *ret;
    head = 0;

    ret = r->skip_to(id);
    if (ret) {
	delete r;
	r = ret;
    }
    if (r->at_end()) return NULL;

    ret = l->skip_to(r->get_docid());
    if (ret) {
	delete l;
	l = ret;
    }
    if (l->at_end()) return NULL;

    advance_to_next_match();
    return NULL;
}
