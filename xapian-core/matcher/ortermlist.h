// OR of two term lists

#ifndef _ortermlist_h_
#define _ortermlist_h_

#include "database.h"
#include "branchtermlist.h"

class OrTermList : public virtual BranchTermList {
    private:
        termid lhead, rhead;
//        weight lmax, rmax, minmax;
    public:
	termcount get_approx_size() const;

	weight get_weight() const;
	termid get_termid() const;
        termcount get_wdf() const;
        doccount get_termfreq() const;

//        weight recalc_maxweight();

	TermList *next();
//	TermList *next(weight w_min);
//	TermList *skip_to(docid, weight w_min);
	bool   at_end() const;

        OrTermList(TermList *, TermList *);
//        OrTermList(TermList *, TermList *, Match *);
};

inline weight
OrTermList::get_weight() const
{
    Assert(lhead != 0 && rhead != 0); // check we've started
    if (lhead < rhead) return l->get_weight();
    if (lhead > rhead) return r->get_weight();
    return l->get_weight() + r->get_weight();
}

inline doccount
OrTermList::get_termfreq() const
{
    Assert(lhead != 0 && rhead != 0); // check we've started
    if (lhead < rhead) return l->get_termfreq();
    return r->get_termfreq();
}

inline termid
OrTermList::get_termid() const
{
    Assert(lhead != 0 && rhead != 0); // check we've started
    return min(lhead, rhead);
}

inline termcount
OrTermList::get_wdf() const
{
    Assert(lhead != 0 && rhead != 0); // check we've started
    if (lhead < rhead) return l->get_wdf();
    if (lhead > rhead) return r->get_wdf();
    return l->get_wdf() + r->get_wdf();
}

inline bool
OrTermList::at_end() const
{
    return lhead == 0 && rhead == 0;
}

inline termcount
OrTermList::get_approx_size() const
{
    return l->get_approx_size() + r->get_approx_size();
}

#endif /* _ortermlist_h_ */
