/* expand.cc
 */

#include "expand.h"
#include "rset.h"
#include "database.h"
#include "ortermlist.h"

#include <algorithm>

class ESetCmp {
    public:
        bool operator()(const ESetItem &a, const ESetItem &b) {
	    if(a.wt > b.wt) return true;
	    if(a.wt != b.wt) return false;
	    return a.tname > b.tname;
        }
};

class TLPCmpGt {
    public:
	bool operator()(const TermList *a, const TermList *b) {
	    return a->get_approx_size() > b->get_approx_size();
	}
};

TermList *
Expand::build_tree(const RSet *rset)
{
    // FIXME: try using a heap instead (C++ sect 18.8)?
    priority_queue<TermList*, vector<TermList*>, TLPCmpGt> pq;
    vector<RSetItem>::const_iterator i;
    for (i = rset->documents.begin();
	 i != rset->documents.end();
	 i++) {
	pq.push(database->open_term_list((*i).did));
    }

    // Build a tree balanced by the term frequencies
    // (similar to building a huffman encoding tree).
    //
    // This scheme reduces the number of objects common terms
    // get "pulled" through, reducing the amount of work done which
    // speeds things up.
    if (pq.empty()) {
	// return new EmptyTermList(); FIXME - do this
	return NULL;
    }

    while (true) {
	cout << "Expand: found termlist" << endl;
	TermList *p = pq.top();
	pq.pop();
	if (pq.empty()) {
	    return p;              
	}
	// NB right is always <= left - we can use this to optimise
	p = new OrTermList(pq.top(), p);
	// p = new OrTermList(pq.top(), p, this); FIXME - for optimising
	pq.pop();
	pq.push(p);
    }
}

void
Expand::expand(const RSet *rset)
{    
    eset.clear();
    etotal = 0;

    if (rset->get_rsize() == 0) return; // No query

    weight w_min = 0;

    TermList *merger = Expand::build_tree(rset);

    bool recalculate_maxweight = false;

    while (1) {
	if (recalculate_maxweight) {
#if 0
	    recalculate_maxweight = false;
	    w_max = merger->recalc_maxweight();
	    cout << "max possible doc weight = " << w_max << endl;
	    if (w_max < w_min) {
		cout << "*** TERMINATING EARLY (1)" << endl;
		break;
	    }
#endif
	}    

	//TermList *ret = merger->next(w_min);
	TermList *ret = merger->next();
        if (ret) {
	    delete merger;
	    merger = ret;

	    cout << "*** REPLACING ROOT\n";
	    // no need for a full recalc (unless we've got to do one because
	    // of a prune elsewhere) - we're just switching to a subtree
#if 0
	    w_max = merger->get_maxweight();
	    cout << "max possible doc weight = " << w_max << endl;
            AssertParanoid(recalculate_maxweight || fabs(w_max - merger->recalc_maxweight()) < 1e-9);

	    if (w_max < w_min) {
		cout << "*** TERMINATING EARLY (2)" << endl;
		break;
	    }
#endif
	}

	if (merger->at_end()) break;

        etotal++;
	
        weight wt = merger->get_weight();
        
        if (wt > w_min) {
	    termname tname = merger->get_termname();
	    eset.push_back(ESetItem(wt, tname));

	    // FIXME: find balance between larger size for more efficient
	    // nth_element and smaller size for better w_min optimisations
	    if (eset.size() == max_esize * 2) {
		// find last element we care about
		cout << "finding nth\n";		
		nth_element(eset.begin(),
			    eset.begin() + max_esize,
			    eset.end(),
			    ESetCmp());
		// erase elements which don't make the grade
	        eset.erase(eset.begin() + max_esize, eset.end());
	        w_min = eset.back().wt;
	        cout << "eset size = " << eset.size() << endl;
	    }
	}
    }

    if (eset.size() > max_esize) {
	// find last element we care about
	cout << "finding nth\n";		
	nth_element(eset.begin(), eset.begin() + max_esize, eset.end(), ESetCmp());
	// erase elements which don't make the grade
	eset.erase(eset.begin() + max_esize, eset.end());
    }
    cout << "sorting\n";

    // Need a stable sort, but this is provided by comparison operator
    sort(eset.begin(), eset.end(), ESetCmp());

    cout << "esize = " << eset.size() << ", etotal = " << etotal << endl;
    if (eset.size()) {
	cout << "max weight in eset = " << eset.front().wt
	     << ", min weight in eset = " << eset.back().wt << endl;
    }
    delete merger;
}
