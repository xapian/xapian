// apply a boolean posting list as a filter to a probabilistic posting list

#include "database.h"
#include "andpostlist.h"

// FilterPostList(probabilistic, boolean)

class FilterPostList : public virtual AndPostList {
    public:
	weight get_weight() const;
	weight get_maxweight() const;

        weight recalc_maxweight();

        FilterPostList(PostList *l, PostList *r, Match *root) :
            AndPostList(l, r, root) {};
};

inline weight
FilterPostList::get_weight() const
{
    return l->get_weight();
}

inline weight
FilterPostList::get_maxweight() const
{
    return l->get_maxweight();
}

inline weight
FilterPostList::recalc_maxweight()
{
    return l->recalc_maxweight();    
}
