#include "andnotpostlist.h"

inline PostList *
AndNotPostList::advance_to_next_match(PostList *ret)
{
    if (ret) {
	delete l;
	l = ret;
    }    
    if (l->at_end()) {
	lhead = 0;
	return NULL;
    }
    lhead = l->get_docid();

    while (rhead <= lhead) {
	if (lhead == rhead) {
	    ret = l->next();	    
	    if (ret) {
		delete l;
		l = ret;
	    }
	    if (l->at_end()) {
		lhead = 0;
		return NULL;
	    }
	    lhead = l->get_docid();
	}		
	ret = r->skip_to(lhead);
	if (ret) {
	    delete r;
	    r = ret;
	}
	if (r->at_end()) {
	    ret = l;
	    l = NULL;
	    return ret;
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
    return advance_to_next_match(l->next());
}

PostList *
AndNotPostList::skip_to(docid id)
{
    return advance_to_next_match(l->skip_to(id));
}
