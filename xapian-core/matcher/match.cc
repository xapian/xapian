/* match.cc
 *
 * ----START-LICENCE----
 * Copyright 1999 Dialog Corporation
 * 
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include "match.h"
#include "andpostlist.h"
#include "orpostlist.h"
#include "xorpostlist.h"
#include "andnotpostlist.h"
#include "andmaybepostlist.h"
#include "filterpostlist.h"
#include "emptypostlist.h"
#include "dbpostlist.h"
#include "irdocument.h"
#include "rset.h"

#include "bm25weight.h"

#include <algorithm>
#include <queue>

/////////////////////////////////////////////
// Comparison operators which we will need //
/////////////////////////////////////////////

// Return true if a has more postings than b
class PLPCmpGt {
    public:
        bool operator()(const PostList *a, const PostList *b) {
            return a->get_termfreq() > b->get_termfreq();
        }
};

// Return true if a has fewer postings than b
class PLPCmpLt {
    public:
        bool operator()(const PostList *a, const PostList *b) {
            return a->get_termfreq() < b->get_termfreq();
        }
};

// Determine the order of elements in the MSet
// Return true if a should be listed before b
// (By default, equally weighted items will be returned in reverse
// document id number.)
class MSetCmp {
    public:
        bool operator()(const MSetItem &a, const MSetItem &b) {
	    if(a.wt > b.wt) return true;
	    if(a.wt == b.wt) return a.did > b.did;
	    return false;
        }
};

////////////////////////////////////
// Initialisation and cleaning up //
////////////////////////////////////

Match::Match(IRDatabase *_database)
	: default_op(MOP_OR),
	  have_added_terms(false)
{
    database = _database;
    max_msize = 1000;
    min_weight_percent = -1;
    rset = NULL;
}

Match::~Match()
{
    while (!weights.empty()) {
	delete(weights.back());
	weights.pop_back();
    }
}

DBPostList *
Match::mk_postlist(const termname& tname,
		   RSet * rset)
{
    // FIXME - this should be centralised into a postlist factory
    DBPostList * pl = database->open_post_list(tname, rset);
    if(rset) rset->will_want_termfreq(tname);

    BM25Weight * wt = new BM25Weight();
    weights.push_back(wt); // Remember it for deleting
    wt->set_stats(database, pl->get_termfreq(), tname, rset);
    pl->set_termweight(wt);
    return pl;
}

////////////////////////
// Building the query //
////////////////////////

void
Match::add_term(const termname& tname)
{
    Assert((have_added_terms = true) == true);
    // We want to push a null PostList in most (all?) situations
    // for similar reasons to using the muscat3.6 zerofreqs option
    if (database->term_exists(tname)) {
	query.push(mk_postlist(tname, rset));
    } else {
	query.push(new EmptyPostList());
    }
}

// FIXME: sort out error handling in next method (e.g. term not found...)
void
Match::add_oplist(matchop op, const vector<termname> &terms)
{
    Assert((have_added_terms = true) == true);
    Assert(op == MOP_OR || op == MOP_AND);
    if (op == MOP_OR) {
	// FIXME: try using a heap instead (C++ sect 18.8)?
	
	// Put terms into a priority queue, such that those with greatest
	// term frequency are returned first.
	priority_queue<PostList*, vector<PostList*>, PLPCmpGt> pq;
	vector<termname>::const_iterator i;
	for (i = terms.begin(); i != terms.end(); i++) {
	    // for an OR, we can just ignore zero freq terms
	    if (database->term_exists(*i)) {
		pq.push(mk_postlist(*i, rset));
	    }
	}

	// Build a tree balanced by the term frequencies
	// (similar to building a huffman encoding tree).
	//
	// This scheme reduces the number of objects common terms
	// get "pulled" through, reducing the amount of work done which
	// speeds things up.
	if (pq.empty()) {
	    query.push(new EmptyPostList());
	    return;
	}

	while (true) {
	    PostList *p = pq.top();
	    pq.pop();
	    if (pq.empty()) {
		query.push(p);		
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
	if (!database->term_exists(*i)) {
	    // a zero freq term => the AND has zero freq
	    vector<PostList *>::const_iterator j;
	    for (j = sorted.begin(); j != sorted.end(); j++) delete *j;
	    sorted.clear();
	    break;
	}
	sorted.push_back(mk_postlist(*i, rset));
    }
    
    if (sorted.empty()) {
	query.push(new EmptyPostList());
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
    query.push(p);		
}

bool
Match::add_op(matchop op)
{
    if (query.size() < 2) return false;
    PostList *left, *right;

    right = query.top();
    query.pop();
    left = query.top();
    query.pop();
    switch (op) {
     case MOP_AND:
	left = new AndPostList(left, right, this);
	break;
     case MOP_OR:
	left = new OrPostList(left, right, this);
	break;
     case MOP_FILTER:
	left = new FilterPostList(left, right, this);
	break;
     case MOP_AND_NOT:
	left = new AndNotPostList(left, right, this);
	break;
     case MOP_AND_MAYBE:
	left = new AndMaybePostList(left, right, this);
	break;
     case MOP_XOR:
	left = new XorPostList(left, right, this);
	break;
    }
    query.push(left);

    return true;
}

///////////////////
// Run the query //
///////////////////

// This method is called by 
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

    if (query.size() == 0) return; // No query

    // Add default operator to all remaining terms.  Unless set with
    // set_default_op(), the default operator is MOP_OR
    while (query.size() > 1) add_op(default_op);

    merger = query.top();

    weight w_max = max_weight = merger->recalc_maxweight();
    recalculate_maxweight = false;

    weight w_min = 0;
    if (min_weight_percent >= 0) w_min = min_weight_percent * max_weight;

    while (1) {
	if (recalculate_maxweight) {
	    recalculate_maxweight = false;
	    w_max = merger->recalc_maxweight();
	    DebugMsg("max possible doc weight = " << w_max << endl);
	    if (w_max < w_min) {
		DebugMsg("*** TERMINATING EARLY (1)" << endl);
		break;
	    }
	}    

	PostList *ret = merger->next(w_min);
        if (ret) {
	    delete merger;
	    merger = ret;

	    DebugMsg("*** REPLACING ROOT" << endl);
	    // no need for a full recalc (unless we've got to do one because
	    // of a prune elsewhere) - we're just switching to a subtree
	    w_max = merger->get_maxweight();
	    DebugMsg("max possible doc weight = " << w_max << endl);
            AssertParanoid(recalculate_maxweight || fabs(w_max - merger->recalc_maxweight()) < 1e-9);

	    if (w_max < w_min) {
		DebugMsg("*** TERMINATING EARLY (2)" << endl);
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
		DebugMsg("finding nth" << endl);
		nth_element(mset.begin(), mset.begin() + max_msize, mset.end(), MSetCmp());
		// erase elements which don't make the grade
	        mset.erase(mset.begin() + max_msize, mset.end());
	        w_min = mset.back().wt;
	        DebugMsg("mset size = " << mset.size() << endl);
	    }
	}
    }

    if (mset.size() > max_msize) {
	// find last element we care about
	DebugMsg("finding nth" << endl);
	nth_element(mset.begin(), mset.begin() + max_msize, mset.end(), MSetCmp());
	// erase elements which don't make the grade
	mset.erase(mset.begin() + max_msize, mset.end());
    }
    DebugMsg("sorting" << endl);

    // Need a stable sort, but this is provided by comparison operator
    sort(mset.begin(), mset.end(), MSetCmp());

    msize = mset.size();

    DebugMsg("msize = " << msize << ", mtotal = " << mtotal << endl);
    if (msize) {
	DebugMsg("max weight in mset = " << mset[0].wt
		 << ", min weight in mset = " << mset[msize - 1].wt << endl);
    }
    delete merger;
    merger = NULL;
}
