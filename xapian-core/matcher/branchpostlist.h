// virtual base class for branched types of postlist

#ifndef _branchpostlist_h_
#define _branchpostlist_h_

#include "database.h"

class BranchPostList : public virtual PostList {
    protected:
        PostList *l, *r;
        void catch_kid(PostList *&kid, PostList **p);
    public:
        virtual ~BranchPostList();
};

inline void
BranchPostList::catch_kid(PostList *&kid, PostList **p)
{
    PostList *tmp;
    tmp = kid;
    kid = *p;
    *p = NULL;
    delete tmp;
}

inline
BranchPostList::~BranchPostList()
{
    if (l) delete l;
    if (r) delete r;
}

#endif /* _branchpostlist_h_ */
