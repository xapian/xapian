#ifndef _expand_h_
#define _expand_h_

#include "database.h"
#include "termlist.h"

#include <queue>
#include <stack>
#include <vector>

class ESetItem {
    public:
	weight wt;
	termname tname;
	ESetItem(weight wt_new, termname tname_new)
		: wt(wt_new), tname(tname_new)
		{ return ; }
};

class Expand {
    private:
        IRDatabase *database;
   
        termcount max_esize;

        bool recalculate_maxweight;
	TermList * build_tree(const RSet *rset, const ExpandWeight *ewt);
    public:
        Expand(IRDatabase *);

        void expand(const RSet *);

        vector<ESetItem> eset;
	termcount etotal;
};

inline Expand::Expand(IRDatabase *database_new)
	: database(database_new),
	  max_esize(1000)
{ return; }

#endif /* _expand_h_ */
