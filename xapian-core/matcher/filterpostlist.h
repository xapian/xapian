// apply a boolean posting list as a filter to a probabilistic posting list

#include "database.h"
#include "andpostlist.h"

// FilterPostList(probabilistic, boolean)

class FilterPostList : public virtual AndPostList {
    public:
	weight get_weight() const;
	weight get_maxweight() const;

        FilterPostList(PostList *l, PostList *r) : AndPostList(l, r) {};
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
