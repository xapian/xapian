// virtual base class for branched types of termlist

#ifndef _branchtermlist_h_
#define _branchtermlist_h_

#include "database.h"
#include "match.h"
#include "termlist.h"

class BranchTermList : public virtual TermList {
    protected:
        void handle_prune(TermList *&kid, TermList *ret);
        TermList *l, *r;
    public:
        virtual ~BranchTermList();
};

inline
BranchTermList::~BranchTermList()
{
    if (l) delete l;
    if (r) delete r;
}

inline void
BranchTermList::handle_prune(TermList *&kid, TermList *ret)
{
    if (ret) {
	delete kid;
	kid = ret;
    }
}

#endif /* _branchtermlist_h_ */
