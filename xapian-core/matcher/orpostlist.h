// OR of two posting lists

#ifndef _orpostlist_h_
#define _orpostlist_h_

#include "database.h"
#include "branchpostlist.h"

class OrPostList : public virtual BranchPostList {
    private:
        docid lhead, rhead;
        weight lmax, rmax, minmax;
    public:
	doccount get_termfreq() const;

	docid  get_docid() const;
	weight get_weight() const;
	weight get_maxweight() const;

        weight recalc_maxweight();

	PostList *next(weight w_min);
	PostList *skip_to(docid, weight w_min);
	bool   at_end() const;

        OrPostList(PostList *, PostList *, Match *);
};

inline doccount
OrPostList::get_termfreq() const
{
    // this is actually the maximum possible frequency for the union of
    // the terms
    return l->get_termfreq() + r->get_termfreq();
}

inline docid
OrPostList::get_docid() const
{
    Assert(lhead != 0 && rhead != 0); // check we've started
    return min(lhead, rhead);
}

// only called if we are doing a probabilistic OR
inline weight
OrPostList::get_weight() const
{
    Assert(lhead != 0 && rhead != 0); // check we've started
    if (lhead < rhead) return l->get_weight();
    if (lhead > rhead) return r->get_weight();
    return l->get_weight() + r->get_weight();
}

// only called if we are doing a probabilistic operation
inline weight
OrPostList::get_maxweight() const
{
    return lmax + rmax;
}

inline weight
OrPostList::recalc_maxweight()
{
    lmax = l->recalc_maxweight();
    rmax = r->recalc_maxweight();
    minmax = min(lmax, rmax);
    return OrPostList::get_maxweight();
}

inline bool
OrPostList::at_end() const
{
    // Can never really happen - OrPostList next/skip_to autoprune
    AssertParanoid(!(l->at_end()) && !(r->at_end()));
    return false;
}

#endif /* _orpostlist_h_ */
