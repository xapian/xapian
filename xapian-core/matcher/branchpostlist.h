// virtual base class for branched types of postlist

#ifndef _branchpostlist_h_
#define _branchpostlist_h_

#include "database.h"
#include "match.h"
#include "postlist.h"

class BranchPostList : public virtual PostList {
    protected:
        void handle_prune(PostList *&kid, PostList *ret);
        PostList *l, *r;
        Match *root;
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
	// now get tree to recalculate max weights...
	root->recalc_maxweight();
    }
}

#endif /* _branchpostlist_h_ */
