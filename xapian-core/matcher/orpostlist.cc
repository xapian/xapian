#include "orpostlist.h"

OrPostList::OrPostList(PostList *left, PostList *right)
{
    l = left;
    r = right;
    lhead = rhead = 0;
}

PostList *
OrPostList::next()
{
    bool ldry = false;
    bool rnext = false;

    if (lhead <= rhead) {
        if (lhead == rhead) rnext = true;

	PostList *ret;
	ret = l->next();
        if (ret) {
	    delete l;
	    l = ret;
	}
	if (l->at_end()) ldry = true;
    } else {
	rnext = true;
    }


    if (rnext) {
	PostList *ret;
	ret = r->next();
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

    if (!ldry) {
	lhead = l->get_docid();
	return NULL;
    }
    
    PostList *ret = r;
    r = NULL;
    return ret;
}

PostList *
OrPostList::skip_to(docid id)
{
    bool ldry;
    PostList *ret;

    ret = l->skip_to(id);
    if (ret) {
	delete l;
	l = ret;
    }
    ldry = l->at_end();

    ret = r->skip_to(id);
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

    if (!ldry) {
	lhead = l->get_docid();
	return NULL;
    }
    
    ret = r;
    r = NULL;
    return ret;
}
