// empty posting list (for zero frequency terms)

#ifndef _emptypostlist_h_
#define _emptypostlist_h_

#include "database.h"

class EmptyPostList : public virtual PostList {
    public:
	doccount get_termfreq() const;

	docid  get_docid() const;
	weight get_weight() const;
	weight get_maxweight() const;

        weight recalc_maxweight();

	PostList *next(weight w_min);
	bool   at_end() const;
};

inline doccount
EmptyPostList::get_termfreq() const
{
    return 0;
}

inline docid
EmptyPostList::get_docid() const
{
    Assert(0); // no documents
    return 0;
}

inline weight
EmptyPostList::get_weight() const
{
    Assert(0); // no documents
    return 0;
}

inline weight
EmptyPostList::get_maxweight() const
{
    return 0;
}

inline weight
EmptyPostList::recalc_maxweight()
{
    return 0;
}

inline PostList *
EmptyPostList::next(weight w_min)
{
    Assert(0); // no documents
    return NULL;
}

inline bool
EmptyPostList::at_end() const
{
    return true;
}

#endif /* _emptypostlist_h_ */
