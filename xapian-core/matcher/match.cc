#include "match.h"
#include "andpostlist.h"
#include "orpostlist.h"
#include "xorpostlist.h"
#include "andnotpostlist.h"
#include "andmaybepostlist.h"
#include "filterpostlist.h"
#include "irdocument.h"

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
{
    DB = database;
    max_msize = 1000;
    min_weight_percent = -1;
}

bool
Match::add_term(termid id)
{
    // FIXME: we might want to push a null PostList in some situations...
    // for similar reasons to using the muscat3.6 zerofreqs option
    if (!id) return false;

    q.push(DB->open_post_list(id));
    return true;
}

// FIXME: sort out error handling in next 2 methods (e.g. term not found...)
bool
Match::add_oplist(matchop op, const vector<termname> &terms)
{
    Assert(op == OR || op == AND);
    vector<termid> ids;
    vector<termname>::const_iterator i;
    for (i = terms.begin(); i != terms.end(); i++) {
	ids.push_back(DB->term_name_to_id(*i));
    }
    return Match::add_oplist(op, ids);
}

bool
Match::add_oplist(matchop op, const vector<termid> &ids)
{
    Assert(op == OR || op == AND);
    if (op == OR) {
	// FIXME: try using a heap instead (C++ sect 18.8)?
	priority_queue<PostList*, vector<PostList*>, PLPCmpGt> pq;
	vector<termid>::const_iterator i;
	for (i = ids.begin(); i != ids.end(); i++) {
	    pq.push(DB->open_post_list(*i));
	}

	// Build a tree balanced by the term frequencies
	// (similar to building a huffman encoding tree).
	//
	// This scheme reduces the number of objects common terms
	// get "pulled" through, reducing the amount of work done which
	// speeds things up.
	while (true) {
	    PostList *p = pq.top();
	    pq.pop();
	    if (pq.empty()) {
		q.push(p);		
		break;
	    }
	    // NB right is always <= left - we can use this to optimise
	    p = new OrPostList(pq.top(), p, this);
	    pq.pop();
	    pq.push(p);
	}
    } else {
	// Build nice tree for AND-ed terms
	// SORT list into ascending freq order
	// AND last two elements, then AND with each subsequent element
	vector<PostList *> sorted;
	vector<termid>::const_iterator i;
	for (i = ids.begin(); i != ids.end(); i++) {
	    sorted.push_back(DB->open_post_list(*i));
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
    return true;
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

#if 0
    if (boolmerger) {
	if (merger) {
	    if (anti_filter) {
		merger = new AndNotPostList(merger, boolmerger, this);
	    } else {
		merger = new FilterPostList(merger, boolmerger, this);
	    }
	} else {
	    // FIXME: What to do if anti_filter is set here?
	    // at present, we just return no hits
	    if (anti_filter) return;
	    merger = boolmerger;
	}
    } else if (!merger)	{
    	return;
    }
#endif

void
Match::match()
{    
    weight w_min = 0;
    int sorted_to = 0;
    
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

    if (min_weight_percent >= 0) w_min = min_weight_percent * max_weight;

    // FIXME: partial_sort?
    // FIXME: quicker to just resort whole lot than sort and merge?
    while (1) {
	if (recalculate_maxweight) {
	    recalculate_maxweight = false;
	    w_max = merger->recalc_maxweight();
	    cout << "max possible doc weight = " << w_max << endl;
	}    

	PostList *ret = merger->next(w_min);
        if (ret) {
	    delete merger;
	    merger = ret;

	    cout << "*** REPLACING ROOT\n";
	    // no need for a full recalc - we're just switching to a subtree
	    w_max = merger->get_maxweight();
	    cout << "max possible doc weight = " << w_max << endl;
	    AssertParanoid(fabs(w_max - merger->recalc_maxweight()) < 1e-9);

	    if (w_max < w_min) {
		cout << "*** TERMINATING EARLY" << endl;
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
	    // resorting and smaller size for better w_min optimisations
	    if (mset.size() == max_msize * 2) {
	        // sort new elements
	        cout << "sorting\n";		
	        stable_sort(mset.begin() + sorted_to, mset.end(), MSetCmp());
		// merge with existing elements
	        cout << "merging\n";
                if (sorted_to) {
		    inplace_merge(mset.begin(), mset.begin() + sorted_to, mset.end(), MSetCmp());
		}
	        msize = max_msize;
	        sorted_to = msize;
		// erase elements which don't make the grade
	        mset.erase(mset.begin() + sorted_to, mset.end());
	        w_min = mset.back().w;
	        cout << "mset size = " << mset.size() << endl;
	    }
	}
    }

    cout << "sorting\n";
    stable_sort(mset.begin() + sorted_to, mset.end(), MSetCmp());
    cout << "merging\n";
    if (sorted_to) {
	inplace_merge(mset.begin(), mset.begin() + sorted_to, mset.end(), MSetCmp());
    }
    msize = mset.size();
    if (max_msize < msize) {
	sorted_to = msize = max_msize;
	mset.erase(mset.begin() + sorted_to, mset.end());
	w_min = mset.back().w;
    }
    cout << "mset size = " << mset.size() << endl;

    cout << "msize = " << msize << ", mtotal = " << mtotal << endl;
    if (msize) {
	cout << "max weight in mset = " << mset[0].w
	     << ", min weight in mset = " << mset[msize - 1].w << endl;
    }
    delete merger;
    merger = NULL;
}
