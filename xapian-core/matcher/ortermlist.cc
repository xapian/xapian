#include "ortermlist.h"

OrTermList::OrTermList(TermList *left, TermList *right)
{
    l = left;
    r = right;
    lhead = rhead = 0;
}

TermList *
OrTermList::next()
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
	if (!l->at_end()) lhead = l->get_termid();
    }
    
    if (rnext) {
	r->next();
	rhead = 0;
	if (!r->at_end()) rhead = r->get_termid();
    }
    return NULL;
}
