/* match.cc
 */

#include "match.h"
#include "andpostlist.h"
#include "orpostlist.h"
#include "xorpostlist.h"
#include "andnotpostlist.h"
#include "andmaybepostlist.h"
#include "filterpostlist.h"
#include "emptypostlist.h"
#include "irdocument.h"
#include "rset.h"

#include "bm25weight.h"

#include <algorithm>

class PLPCmpGt {
    public:
        bool operator()(const PostList *a, const PostList *b) {
            return a->get_termfreq() > b->get_termfreq();
        }
};

class PLPCmpLt {
    public:
        bool operator()(const PostList *a, const PostList *b) {
            return a->get_termfreq() < b->get_termfreq();
        }
};

Match::Match(IRDatabase *database_new)
	: have_added_terms(false)
{
    DB = database_new;
    max_msize = 1000;
    min_weight_percent = -1;
    rset = NULL;
}

DBPostList * mk_postlist(IRDatabase *DB,
			 const termname& tname,
			 RSet * rset) {
    // FIXME - this should be centralised into a postlist factory
    DBPostList * pl = DB->open_post_list(tname, rset);
    if(rset) rset->will_want_termfreq(tname);

    BM25Weight * wt = new BM25Weight();
    wt->set_stats(DB, pl->get_termfreq(), tname, rset);
    pl->set_termweight(wt);
    return pl;
}

void
Match::add_term(const termname& tname)
{
    Assert((have_added_terms = true) == true);
    // We want to push a null PostList in most (all?) situations
    // for similar reasons to using the muscat3.6 zerofreqs option
    if (DB->term_exists(tname)) {
	q.push(mk_postlist(DB, tname, rset));
    } else {
	q.push(new EmptyPostList());
    }
}

// FIXME: sort out error handling in next method (e.g. term not found...)
void
Match::add_oplist(matchop op, const vector<termname> &terms)
{
    Assert((have_added_terms = true) == true);
    Assert(op == OR || op == AND);
    if (op == OR) {
	// FIXME: try using a heap instead (C++ sect 18.8)?
	priority_queue<PostList*, vector<PostList*>, PLPCmpGt> pq;
	vector<termname>::const_iterator i;
	for (i = terms.begin(); i != terms.end(); i++) {
	    // for an OR, we can just ignore zero freq terms
	    if (DB->term_exists(*i)) {
		pq.push(mk_postlist(DB, *i, rset));
	    }
	}

	// Build a tree balanced by the term frequencies
	// (similar to building a huffman encoding tree).
	//
	// This scheme reduces the number of objects common terms
	// get "pulled" through, reducing the amount of work done which
	// speeds things up.
	if (pq.empty()) {
	    q.push(new EmptyPostList());
	    return;
	}

	while (true) {
	    PostList *p = pq.top();
	    pq.pop();
	    if (pq.empty()) {
		q.push(p);		
		return;
	    }
	    // NB right is always <= left - we can use this to optimise
	    p = new OrPostList(pq.top(), p, this);
	    pq.pop();
	    pq.push(p);
	}
    }
    
    // Build nice tree for AND-ed terms
    // SORT list into ascending freq order
    // AND last two elements, then AND with each subsequent element
    vector<PostList *> sorted;
    vector<termname>::const_iterator i;
    for (i = terms.begin(); i != terms.end(); i++) {
	if (!DB->term_exists(*i)) {
	    // a zero freq term => the AND has zero freq
	    vector<PostList *>::const_iterator j;
	    for (j = sorted.begin(); j != sorted.end(); j++) delete *j;
	    sorted.clear();
	    break;
	}
	sorted.push_back(mk_postlist(DB, *i, rset));
    }
    
    if (sorted.empty()) {
	q.push(new EmptyPostList());
	return;
    }

    stable_sort(sorted.begin(), sorted.end(), PLPCmpLt());
    
    PostList *p = sorted.back();
    sorted.pop_back();
    while (!sorted.empty()) {	    
	// NB right is always <= left - we can use this to optimise
	p = new AndPostList(sorted.back(), p, this);
	sorted.pop_back();
    }
    q.push(p);		
}

bool
Match::add_op(matchop op)
{
    if (q.size() < 2) return false;
    PostList *left, *right;

    right = q.top();
    q.pop();
    left = q.top();
    q.pop();
    switch (op) {
     case AND:
	left = new AndPostList(left, right, this);
	break;
     case OR:
	left = new OrPostList(left, right, this);
	break;
     case FILTER:
	left = new FilterPostList(left, right, this);
	break;
     case AND_NOT:
	left = new AndNotPostList(left, right, this);
	break;
     case AND_MAYBE:
	left = new AndMaybePostList(left, right, this);
	break;
     case XOR:
	left = new XorPostList(left, right, this);
	break;
    }
    q.push(left);

    return true;
}

class MSetCmp {
    public:
        bool operator()(const MSetItem &a, const MSetItem &b) {
	    if(a.wt > b.wt) return true;
	    if(a.wt == b.wt) return a.did > b.did;
	    return false;
        }
};

void
Match::recalc_maxweight()
{
    // if we don't have a merger, who the hell is telling us to recalc?
    Assert(merger != NULL);
    recalculate_maxweight = true;
}

void
Match::match()
{    
    
    msize = 0;
    mtotal = 0;
    max_weight = 0;

    merger = NULL;

    if (q.size() == 0) return; // No query

    // FIXME: option to specify default operator? e.g. combine all remaining
    // terms with AND
    while (q.size() > 1) add_op(OR);

    merger = q.top();

    weight w_max = max_weight = merger->recalc_maxweight();
    recalculate_maxweight = false;

    weight w_min = 0;
    if (min_weight_percent >= 0) w_min = min_weight_percent * max_weight;

    while (1) {
	if (recalculate_maxweight) {
	    recalculate_maxweight = false;
	    w_max = merger->recalc_maxweight();
#ifdef MUS_DEBUG_VERBOSE
	    cout << "max possible doc weight = " << w_max << endl;
#endif /* MUS_DEBUG_VERBOSE */
	    if (w_max < w_min) {
#ifdef MUS_DEBUG_VERBOSE
		cout << "*** TERMINATING EARLY (1)" << endl;
#endif /* MUS_DEBUG_VERBOSE */
		break;
	    }
	}    

	PostList *ret = merger->next(w_min);
        if (ret) {
	    delete merger;
	    merger = ret;

#ifdef MUS_DEBUG_VERBOSE
	    cout << "*** REPLACING ROOT\n";
#endif /* MUS_DEBUG_VERBOSE */
	    // no need for a full recalc (unless we've got to do one because
	    // of a prune elsewhere) - we're just switching to a subtree
	    w_max = merger->get_maxweight();
#ifdef MUS_DEBUG_VERBOSE
	    cout << "max possible doc weight = " << w_max << endl;
#endif /* MUS_DEBUG_VERBOSE */
            AssertParanoid(recalculate_maxweight || fabs(w_max - merger->recalc_maxweight()) < 1e-9);

	    if (w_max < w_min) {
#ifdef MUS_DEBUG_VERBOSE
		cout << "*** TERMINATING EARLY (2)" << endl;
#endif /* MUS_DEBUG_VERBOSE */
		break;
	    }
	}

	if (merger->at_end()) break;

        mtotal++;
	
        weight w = merger->get_weight();
        
        if (w > w_min) {
	    docid id = merger->get_docid();
	    mset.push_back(MSetItem(w, id));

	    // FIXME: find balance between larger size for more efficient
	    // nth_element and smaller size for better w_min optimisations
	    if (mset.size() == max_msize * 2) {
		// find last element we care about
#ifdef MUS_DEBUG_VERBOSE
		cout << "finding nth\n";		
#endif /* MUS_DEBUG_VERBOSE */
		nth_element(mset.begin(), mset.begin() + max_msize, mset.end(), MSetCmp());
		// erase elements which don't make the grade
	        mset.erase(mset.begin() + max_msize, mset.end());
	        w_min = mset.back().wt;
#ifdef MUS_DEBUG_VERBOSE
	        cout << "mset size = " << mset.size() << endl;
#endif /* MUS_DEBUG_VERBOSE */
	    }
	}
    }

    if (mset.size() > max_msize) {
	// find last element we care about
#ifdef MUS_DEBUG_VERBOSE
	cout << "finding nth\n";		
#endif /* MUS_DEBUG_VERBOSE */
	nth_element(mset.begin(), mset.begin() + max_msize, mset.end(), MSetCmp());
	// erase elements which don't make the grade
	mset.erase(mset.begin() + max_msize, mset.end());
    }
#ifdef MUS_DEBUG_VERBOSE
    cout << "sorting\n";
#endif /* MUS_DEBUG_VERBOSE */

    // Need a stable sort, but this is provided by comparison operator
    sort(mset.begin(), mset.end(), MSetCmp());

    msize = mset.size();

#ifdef MUS_DEBUG_VERBOSE
    cout << "msize = " << msize << ", mtotal = " << mtotal << endl;
#endif /* MUS_DEBUG_VERBOSE */
    if (msize) {
#ifdef MUS_DEBUG_VERBOSE
	cout << "max weight in mset = " << mset[0].wt
	     << ", min weight in mset = " << mset[msize - 1].wt << endl;
#endif /* MUS_DEBUG_VERBOSE */
    }
    delete merger;
    merger = NULL;
}
