/* expand.h: class for finding expand terms
 *
 * ----START-LICENCE----
 * -----END-LICENCE-----
 */

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

class ExpandDecider {
    public:
	virtual bool want_term(const termname&) const = 0;
};

class ExpandDeciderAlways : public virtual ExpandDecider {
    public:
	bool want_term(const termname&) const { return true; }
};

class Expand {
    private:
        IRDatabase *database;
   
        termcount max_esize;

        bool recalculate_maxweight;
	TermList * build_tree(const RSet *rset, const ExpandWeight *ewt);
    public:
        Expand(IRDatabase *);

	void expand(const RSet *, const ExpandDecider *);

        vector<ESetItem> eset;
	termcount etotal;
};

inline Expand::Expand(IRDatabase *database_new)
	: database(database_new),
	  max_esize(1000)
{ return; }

#endif /* _expand_h_ */
