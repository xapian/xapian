// virtual base class for branched types of termlist

#ifndef _branchtermlist_h_
#define _branchtermlist_h_

#include "database.h"
#include "match.h"
#include "termlist.h"

class BranchTermList : public virtual TermList {
    protected:
//        void handle_prune(TermList *&kid, TermList *ret);
        TermList *l, *r;
//        Match *root;
    public:
        virtual ~BranchTermList();
};

inline
BranchTermList::~BranchTermList()
{
    if (l) delete l;
    if (r) delete r;
}

#if 0
inline void
BranchtermList::handle_prune(termList *&kid, termList *ret)
{
    if (ret) {
	delete kid;
	kid = ret;
	// now get tree to recalculate max weights...
	root->recalc_maxweight();
    }
}
#endif

#endif /* _branchtermlist_h_ */
