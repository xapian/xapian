// apply a boolean posting list as a filter to a probabilistic posting list

#include "database.h"

// FilterPostList(probabilistic, boolean)

class FilterPostList : public virtual AndPostList {
    public:
	weight get_weight() const;

        FilterPostList(PostList *l, PostList *r) : AndPostList(l, r) {};
};

inline weight
FilterPostList::get_weight() const
{
    return l->get_weight();
}
