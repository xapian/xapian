/* expand.cc
 */

#include "expand.h"
#include "rset.h"
#include "database.h"
#include "ortermlist.h"
#include "omassert.h"

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
Expand::build_tree(const RSet *rset, const ExpandWeight *ewt)
{
    // FIXME: try using a heap instead (C++ sect 18.8)?
    priority_queue<TermList*, vector<TermList*>, TLPCmpGt> pq;
    vector<RSetItem>::const_iterator i;
    for (i = rset->documents.begin();
	 i != rset->documents.end();
	 i++) {
	DBTermList *tl = database->open_term_list((*i).did);
	tl->set_weighting(ewt);
	pq.push(tl);
    }

    if (pq.empty()) {
	return NULL;
    }

    // Build a tree balanced by the term frequencies
    // (similar to building a huffman encoding tree).
    //
    // This scheme reduces the number of objects terms from large docs
    // get "pulled" through, reducing the amount of work done which
    // speeds things up.
    while (true) {
	TermList *p = pq.top();
#ifdef MUS_DEBUG_VERBOSE
	//cout << "Expand: adding termlist " << p << " to tree" << endl;
#endif /* MUS_DEBUG_VERBOSE */
	pq.pop();
	if (pq.empty()) {
	    return p;
	}
	// NB right is always <= left - we can use this to optimise
	p = new OrTermList(pq.top(), p);
	pq.pop();
	pq.push(p);
    }
}

void
Expand::expand(const RSet *rset, const ExpandDecider *decider)
{    
    eset.clear();
    etotal = 0;

    if (rset->get_rsize() == 0) return; // No query

    weight w_min = 0;

    // Start weighting scheme
    ExpandWeight ewt(database, rset->get_rsize());

    TermList *merger = build_tree(rset, &ewt);
    if(merger == NULL) return;

#ifdef MUS_DEBUG_VERBOSE
    cout << "ewt.get_maxweight() = " << ewt.get_maxweight() << endl;
#endif /* MUS_DEBUG_VERBOSE */
    while (1) {
	TermList *ret = merger->next();
        if (ret) {
#ifdef MUS_DEBUG_VERBOSE
	    //cout << "*** REPLACING ROOT\n";
#endif /* MUS_DEBUG_VERBOSE */
	    delete merger;
	    merger = ret;
	}

	if (merger->at_end()) break;

	termname tname = merger->get_termname();
	if(decider->want_term(tname)) {
	    etotal++;

	    ExpandBits ebits = merger->get_weighting();
	    weight wt = ewt.get_weight(ebits, tname);

	    if (wt > w_min) {
		eset.push_back(ESetItem(wt, tname));

		// FIXME: find balance between larger size for more efficient
		// nth_element and smaller size for better w_min optimisations
		if (eset.size() == max_esize * 2) {
		    // find last element we care about
#ifdef MUS_DEBUG_VERBOSE
		    cout << "finding nth\n";		
#endif /* MUS_DEBUG_VERBOSE */
		    nth_element(eset.begin(),
				eset.begin() + max_esize,
				eset.end(),
				ESetCmp());
		    // erase elements which don't make the grade
		    eset.erase(eset.begin() + max_esize, eset.end());
		    w_min = eset.back().wt;
#ifdef MUS_DEBUG_VERBOSE
		    cout << "eset size = " << eset.size() << endl;
#endif /* MUS_DEBUG_VERBOSE */
		}
	    }
	}
    }

    if (eset.size() > max_esize) {
	// find last element we care about
#ifdef MUS_DEBUG_VERBOSE
	cout << "finding nth\n";		
#endif /* MUS_DEBUG_VERBOSE */
	nth_element(eset.begin(), eset.begin() + max_esize, eset.end(), ESetCmp());
	// erase elements which don't make the grade
	eset.erase(eset.begin() + max_esize, eset.end());
    }
#ifdef MUS_DEBUG_VERBOSE
    cout << "sorting\n";
#endif /* MUS_DEBUG_VERBOSE */

    // Need a stable sort, but this is provided by comparison operator
    sort(eset.begin(), eset.end(), ESetCmp());

#ifdef MUS_DEBUG_VERBOSE
    cout << "esize = " << eset.size() << ", etotal = " << etotal << endl;
#endif /* MUS_DEBUG_VERBOSE */
    if (eset.size()) {
#ifdef MUS_DEBUG_VERBOSE
	cout << "max weight in eset = " << eset.front().wt
	     << ", min weight in eset = " << eset.back().wt << endl;
#endif /* MUS_DEBUG_VERBOSE */
    }
    delete merger;
}
