// OR of two term lists

#ifndef _ortermlist_h_
#define _ortermlist_h_

#include "database.h"
#include "branchtermlist.h"

class OrTermList : public virtual BranchTermList {
    private:
        termname lhead, rhead;
//        weight lmax, rmax, minmax;
	bool started;
    public:
	termcount get_approx_size() const;

	ExpandBits get_weighting() const;
	termname get_termname() const;
        termcount get_wdf() const;
        doccount get_termfreq() const;

//	weight recalc_maxweight();

	TermList *next();
//	TermList *next(weight w_min);
//	TermList *skip_to(docid, weight w_min);
	bool   at_end() const;

        OrTermList(TermList *, TermList *);
//        OrTermList(TermList *, TermList *, Match *);
};

inline ExpandBits
OrTermList::get_weighting() const
{
    Assert(started); // check we've started
    if (lhead < rhead) return l->get_weighting();
    if (lhead > rhead) return r->get_weighting();
    return l->get_weighting() + r->get_weighting();
}

inline doccount
OrTermList::get_termfreq() const
{
    Assert(started); // check we've started
    if (lhead < rhead) return l->get_termfreq();
    return r->get_termfreq();
}

inline termname
OrTermList::get_termname() const
{
    Assert(started); // check we've started
    return min(lhead, rhead);
}

inline termcount
OrTermList::get_wdf() const
{
    Assert(started); // check we've started
    if (lhead < rhead) return l->get_wdf();
    if (lhead > rhead) return r->get_wdf();
    return l->get_wdf() + r->get_wdf();
}

inline bool
OrTermList::at_end() const
{
    AssertParanoid(!(l->at_end()) && !(r->at_end()));
    return false; // Should have thrown a sub-tree, rather than got to end
}

inline termcount
OrTermList::get_approx_size() const
{
    return l->get_approx_size() + r->get_approx_size();
}

#endif /* _ortermlist_h_ */
