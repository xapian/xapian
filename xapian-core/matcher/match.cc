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

Match::Match(IRDatabase *database)
	: have_added_terms(false)
{
    DB = database;
    max_msize = 1000;
    min_weight_percent = -1;
    rset = NULL;
}

void
Match::add_term(termid id)
{
    Assert((have_added_terms = true) == true);
    // We want to push a null PostList in most (all?) situations
    // for similar reasons to using the muscat3.6 zerofreqs option
    if (id) {
	q.push(DB->open_post_list(id, rset));
	if(rset) rset->will_want_termfreq(id);
	// FIXME - should get called automatically when we open a post list
	// -- want a postlist factory
    } else {
	q.push(new EmptyPostList());
    }
}

// FIXME: sort out error handling in next 2 methods (e.g. term not found...)
void
Match::add_oplist(matchop op, const vector<termname> &terms)
{
    Assert(op == OR || op == AND);
    vector<termid> ids;
    vector<termname>::const_iterator i;
    for (i = terms.begin(); i != terms.end(); i++) {
	ids.push_back(DB->term_name_to_id(*i));
    }
    Match::add_oplist(op, ids);
}

void
Match::add_oplist(matchop op, const vector<termid> &ids)
{
    Assert((have_added_terms = true) == true);
    Assert(op == OR || op == AND);
    if (op == OR) {
	// FIXME: try using a heap instead (C++ sect 18.8)?
	priority_queue<PostList*, vector<PostList*>, PLPCmpGt> pq;
	vector<termid>::const_iterator i;
	for (i = ids.begin(); i != ids.end(); i++) {
	    // for an OR, we can just ignore zero freq terms
	    termid id = *i;
	    if (id) {
		pq.push(DB->open_post_list(id, rset));
		if(rset) rset->will_want_termfreq(id);
		// FIXME - should get called automatically when we open a
		// post list -- want a postlist factory
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
    vector<termid>::const_iterator i;
    for (i = ids.begin(); i != ids.end(); i++) {
	termid id = *i;
	if (id == 0) {
	    // a zero freq term => the AND has zero freq
	    vector<PostList *>::const_iterator j;
	    for (j = sorted.begin(); j != sorted.end(); j++) delete *j;
	    sorted.clear();
	    break;
	}
	sorted.push_back(DB->open_post_list(id, rset));
	if(rset) rset->will_want_termfreq(id);
	// FIXME - should get called automatically when we open a post list
	// -- want a postlist factory
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
            return a.w > b.w;
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

        mtotal++;
	
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

    msize = mset.size();

    cout << "msize = " << msize << ", mtotal = " << mtotal << endl;
    if (msize) {
	cout << "max weight in mset = " << mset[0].w
	     << ", min weight in mset = " << mset[msize - 1].w << endl;
    }
    delete merger;
    merger = NULL;
}
