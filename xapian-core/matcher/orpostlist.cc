#include "orpostlist.h"

inline void
OrPostList::maybe_throw_kid(void)
{
    if (!rhead) {	
	throw &l;
    } else if (!lhead) {
	throw &r;
    }
}

OrPostList::OrPostList(PostList *left, PostList *right)
{
    l = left;
    r = right;
    lhead = rhead = 0;
}

void
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
	try { l->next(); } catch (PostList **p) { catch_kid(l, p); }
	lhead = 0;
	if (!l->at_end()) lhead = l->get_docid();
    }

    if (rnext) {
	try { r->next(); } catch (PostList **p) { catch_kid(r, p); }
        rhead = 0;
        if (!r->at_end()) rhead = r->get_docid();
    }

    maybe_throw_kid();
}

void
OrPostList::skip_to(docid id)
{
    try { l->skip_to(id); } catch (PostList **p) { catch_kid(l, p); }
    lhead = 0;
    if (!l->at_end()) lhead = l->get_docid();

    try { r->skip_to(id); } catch (PostList **p) { catch_kid(r, p); }
    rhead = 0;
    if (!r->at_end()) rhead = r->get_docid();

    maybe_throw_kid();
}
