#include "orpostlist.h"

OrPostList::OrPostList(PostList *left, PostList *right)
{
    l = left;
    r = right;
    lhead = rhead = 0;
    if (!l->at_end()) lhead = l->get_docid();
    if (!r->at_end()) rhead = r->get_docid();
}

OrPostList::~OrPostList()
{
    delete l;
    delete r;
}

void
OrPostList::next()
{
    bool lnext = false;
    bool rnext = false;

    if (!rhead) {
        if (lhead) lnext = true;
    } else if (!lhead) {
        rnext = true;
    } else if (lhead <= rhead) {
        lnext = true;
        if (lhead == rhead) rnext = true;
    } else {
	rnext = true;
    }

    if (lnext) {
        l->next();
        lhead = 0;
        if (!l->at_end()) lhead = l->get_docid();
    }

    if (rnext) {
        r->next();
        rhead = 0;
        if (!r->at_end()) rhead = r->get_docid();
    }
}

void
OrPostList::skip_to(docid id)
{
    if (lhead) {
        l->skip_to(id);
        lhead = 0;
        if (!l->at_end()) lhead = l->get_docid();
    }

    if (rhead) {
        r->skip_to(id);
        rhead = 0;
        if (!r->at_end()) rhead = r->get_docid();
    }
}
