// virtual base class for branched types of postlist

#ifndef _branchpostlist_h_
#define _branchpostlist_h_

#include "database.h"

class BranchPostList : public virtual PostList {
    protected:
        void handle_prune(PostList *&kid, PostList *ret);
        PostList *l, *r;
    public:
        virtual ~BranchPostList();
};

inline
BranchPostList::~BranchPostList()
{
    if (l) delete l;
    if (r) delete r;
}

inline void
BranchPostList::handle_prune(PostList *&kid, PostList *ret)
{
    if (ret) {
	delete kid;
	kid = ret;
    }
    // FIXME: now get tree to recalculate max weights...
}

#endif /* _branchpostlist_h_ */
