// AND of two posting lists

#ifndef _andpostlist_h_
#define _andpostlist_h_

#include "database.h"
#include "branchpostlist.h"

class AndPostList : public virtual BranchPostList {
    private:
        docid head;
        weight lmax, rmax;

        void process_next_or_skip_to(weight w_min, PostList *);
    public:
	doccount get_termfreq() const;

	docid  get_docid() const;
	weight get_weight() const;
	weight get_maxweight() const;

        weight recalc_maxweight();
    
	PostList *next(weight w_min);
	PostList *skip_to(docid, weight w_min);
	bool   at_end() const;

        AndPostList(PostList *l, PostList *r, Match *root_);
};

inline doccount
AndPostList::get_termfreq() const
{
    // this is actually the maximum possible frequency for the intersection of
    // the terms
    return min(l->get_termfreq(), r->get_termfreq());
}

inline docid
AndPostList::get_docid() const
{
    return head;
}

// only called if we are doing a probabilistic AND
inline weight
AndPostList::get_weight() const
{
    return l->get_weight() + r->get_weight();
}

// only called if we are doing a probabilistic operation
inline weight
AndPostList::get_maxweight() const
{
    return lmax + rmax;
}

inline weight
AndPostList::recalc_maxweight()
{
    lmax = l->recalc_maxweight();
    rmax = r->recalc_maxweight();
    return AndPostList::get_maxweight();
}

inline bool
AndPostList::at_end() const
{
    return head == 0;
}

#endif /* _andpostlist_h_ */
