#include "ortermlist.h"

OrTermList::OrTermList(TermList *left, TermList *right)
	: lhead(0), rhead(0)
{
    l = left;
    r = right;
}

TermList *
OrTermList::next()
{
    bool ldry = false;
    bool rnext = false;

    if (lhead <= rhead) {
	if (lhead == rhead) rnext = true;
	//handle_prune(l, l->next(w_min - rmax));
	handle_prune(l, l->next());
	if (l->at_end()) ldry = true;
    } else {
	rnext = true;
    }

    if (rnext) {
	//handle_prune(r, r->next(w_min - lmax));
	handle_prune(r, r->next());
	if (r->at_end()) {
	    TermList *ret = l;
	    l = NULL;
	    return ret;
	}
	rhead = r->get_termid();
    }

    if (!ldry) {
	lhead = l->get_termid();
	return NULL;
    }

    TermList *ret = r;
    r = NULL;
    return ret;
}
