/* expand.cc
 */

#include "expand.h"
#include "rset.h"
#include "database.h"

#include <algorithm>

class ESetCmp {
    public:
        bool operator()(const ESetItem &a, const ESetItem &b) {
            return a.wt > b.wt;
        }
};

void
Expand::expand(RSet *rset)
{    
    eset.clear();
    etotal = 0;

    if (rset->get_rsize() == 0) return; // No query

    TermList *merger = NULL;

    weight w_min = 0;
    vector<TermList *> termlists;

#if 0
    // FIXME: option to specify default operator? e.g. combine all remaining
    // terms with AND
    while (q.size() > 1) add_op(OR);

    merger = q.top();

    recalculate_maxweight = false;

    while (1) {
	if (recalculate_maxweight) {
	    recalculate_maxweight = false;
	    w_max = merger->recalc_maxweight();
	    cout << "max possible doc weight = " << w_max << endl;
	    if (w_max < w_min) {
		cout << "*** TERMINATING EARLY (1)" << endl;
		break;
	    }
	}    

	PostList *ret = merger->next(w_min);
        if (ret) {
	    delete merger;
	    merger = ret;

	    cout << "*** REPLACING ROOT\n";
	    // no need for a full recalc (unless we've got to do one because
	    // of a prune elsewhere) - we're just switching to a subtree
	    w_max = merger->get_maxweight();
	    cout << "max possible doc weight = " << w_max << endl;
            AssertParanoid(recalculate_maxweight || fabs(w_max - merger->recalc_maxweight()) < 1e-9);

	    if (w_max < w_min) {
		cout << "*** TERMINATING EARLY (2)" << endl;
		break;
	    }
	}

	if (merger->at_end()) break;

        etotal++;
	
        weight w = merger->get_weight();
        
        if (w > w_min) {
	    docid id = merger->get_docid();
	    mset.push_back(MSetItem(w, id));

	    // FIXME: find balance between larger size for more efficient
	    // nth_element and smaller size for better w_min optimisations
	    if (mset.size() == max_msize * 2) {
		// find last element we care about
		cout << "finding nth\n";		
		nth_element(mset.begin(), mset.begin() + max_msize, mset.end(), MSetCmp());
		// erase elements which don't make the grade
	        mset.erase(mset.begin() + max_msize, mset.end());
	        w_min = mset.back().w;
	        cout << "mset size = " << mset.size() << endl;
	    }
	}
    }

    if (mset.size() > max_msize) {
	// find last element we care about
	cout << "finding nth\n";		
	nth_element(mset.begin(), mset.begin() + max_msize, mset.end(), MSetCmp());
	// erase elements which don't make the grade
	mset.erase(mset.begin() + max_msize, mset.end());
    }
    cout << "sorting\n";
    stable_sort(mset.begin(), mset.end(), MSetCmp());
#endif

    cout << "esize = " << eset.size() << ", etotal = " << etotal << endl;
    if (eset.size()) {
	cout << "max weight in mset = " << eset.front().wt
	     << ", min weight in mset = " << eset.back().wt << endl;
    }
    delete merger;
}
