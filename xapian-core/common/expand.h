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
	termid tid;
	ESetItem(weight wt_new, docid tid_new)
		: wt(wt_new), tid(tid_new)
		{ return ; }
};

class Expand {
    private:
        IRDatabase *database;
   
        termcount max_esize;

        bool recalculate_maxweight;
    public:
        Expand(IRDatabase *);

        void expand(RSet *);

        vector<ESetItem> eset;
	termcount etotal;
};

Expand::Expand(IRDatabase *database_new)
	: database(database_new),
	  max_esize(1000)
{ return; }

#endif /* _expand_h_ */
