#include "orpostlist.h"

inline PostList *
OrPostList::maybe_throw_kid(void)
{
    PostList *ret = NULL;
    if (!rhead) {
	ret = l;
	l = NULL;
    } else if (!lhead) {
	ret = r;
	r = NULL;
    }
    return ret;
}

OrPostList::OrPostList(PostList *left, PostList *right)
{
    l = left;
    r = right;
    lhead = rhead = 0;
}

PostList *
OrPostList::next()
{
    bool lnext = false;
    bool rnext = false;

    if (lhead <= rhead) {
        lnext = true;
        if (lhead == rhead) rnext = true;
    } else {
	rnext = true;
    }

    if (lnext) {
	PostList *ret;
	ret = l->next();
        if (ret) {
	    delete l;
	    l = ret;
	}
	lhead = 0;
	if (!l->at_end()) lhead = l->get_docid();
    }

    if (rnext) {
	PostList *ret;
	ret = r->next();
        if (ret) {
	    delete r;
	    r = ret;
	}
        rhead = 0;
        if (!r->at_end()) rhead = r->get_docid();
    }

    PostList *ret = NULL;
    if (!rhead) {
	ret = l;
	l = NULL;
    } else if (!lhead) {
	ret = r;
	r = NULL;
    }
    return ret;
//    return maybe_throw_kid();
}

PostList *
OrPostList::skip_to(docid id)
{
    PostList *ret;
    ret = l->skip_to(id);
    if (ret) {
	delete l;
	l = ret;
    }
    lhead = 0;
    if (!l->at_end()) lhead = l->get_docid();

    ret = r->skip_to(id);
    if (ret) {
	delete r;
	r = ret;
    }
    rhead = 0;
    if (!r->at_end()) rhead = r->get_docid();

    ret = NULL;
    if (!rhead) {
	ret = l;
	l = NULL;
    } else if (!lhead) {
	ret = r;
	r = NULL;
    }
    return ret;
//    return maybe_throw_kid();
}
