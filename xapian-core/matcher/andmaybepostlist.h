// AND MAYBE of two posting lists
// A AND MAYBE B is logically just A, but we keep B around for weight purposes

#ifndef _andmaybepostlist_h_
#define _andmaybepostlist_h_

#include "database.h"
#include "branchpostlist.h"

class AndMaybePostList : public virtual BranchPostList {
    private:
        docid lhead, rhead;
        weight lmax, rmax;

    public:
	doccount get_termfreq() const;

	docid  get_docid() const;
	weight get_weight() const;
	weight get_maxweight() const;

        weight recalc_maxweight();

	PostList *next(weight w_min);
	PostList *skip_to(docid, weight w_min);
	bool   at_end() const;

        AndMaybePostList(PostList *, PostList *, Match *root_);

        PostList *sync_and_skip_to(docid id, weight w_min, docid lh, docid rh);
};

inline doccount
AndMaybePostList::get_termfreq() const
{
    // this is exactly correct
    return l->get_termfreq();
}

inline docid
AndMaybePostList::get_docid() const
{
    Assert(lhead != 0 && rhead != 0); // check we've started
    return lhead;
}

// only called if we are doing a probabilistic AND MAYBE
inline weight
AndMaybePostList::get_weight() const
{
    Assert(lhead != 0 && rhead != 0); // check we've started
    if (lhead == rhead) return l->get_weight() + r->get_weight();
    return l->get_weight();
}

// only called if we are doing a probabilistic operation
inline weight
AndMaybePostList::get_maxweight() const
{
    return lmax + rmax;
}

inline weight
AndMaybePostList::recalc_maxweight()
{
    lmax = l->recalc_maxweight();
    rmax = r->recalc_maxweight();
    return AndMaybePostList::get_maxweight();
}

inline bool
AndMaybePostList::at_end() const
{
    return lhead == 0;
}

#endif /* _andmaybepostlist_h_ */
