// OR of two posting lists

#ifndef _orpostlist_h_
#define _orpostlist_h_

#include "database.h"
#include "branchpostlist.h"

class OrPostList : public virtual BranchPostList {
    private:
        docid lhead, rhead;
    public:
	doccount get_termfreq() const;

	docid  get_docid() const;
	weight get_weight() const;

	PostList *next();
	PostList *skip_to(docid);
	bool   at_end() const;

        OrPostList(PostList *, PostList *);
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

inline bool
OrPostList::at_end() const
{
    // Can never really happen - OrPostList next/skip_to autoprune
    return false;
}

#endif /* _orpostlist_h_ */
