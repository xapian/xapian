/* match.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

// Compare an MSetItem, using a custom function
class MSetCmp {
    public:
	bool (* fn)(const MSetItem &a, const MSetItem &b);
	MSetCmp(bool (* _fn)(const MSetItem &a, const MSetItem &b)) : fn(_fn) {}
	bool operator()(const MSetItem &a, const MSetItem &b) {
	    return fn(a, b);
	}
};

// Comparison which sorts equally weighted MSetItems in docid order
bool msetcmp_forward(const MSetItem &a, const MSetItem &b) {
    if(a.wt > b.wt) return true;
    if(a.wt == b.wt) return a.did < b.did;
    return false;
}

// Comparison which sorts equally weighted MSetItems in reverse docid order
bool msetcmp_reverse(const MSetItem &a, const MSetItem &b) {
    if(a.wt > b.wt) return true;
    if(a.wt == b.wt) return a.did > b.did;
    return false;
}

////////////////////////////////////
// Initialisation and cleaning up //
////////////////////////////////////

Match::Match(IRDatabase *_database)
	: default_op(MOP_OR),
	  do_collapse(false),
	  have_added_terms(false),
	  query_ready(false)
{
    database = _database;
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
    query_ready = false;
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
    query_ready = false;
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
    query_ready = false;
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

// This method is called by branch postlists when they rebalance
// in order to recalculate the weights in the tree
void
Match::recalc_maxweight()
{
    recalculate_maxweight = true;
}

// Prepare the query for running, if it isn't already ready
PostList *
Match::build_query()
{
    if(!query_ready) {
	query_ready = true;
	max_weight = 0;

	if (query.size() == 0) return NULL; // No query

	// Add default operator to all remaining terms.  Unless set with
	// set_default_op(), the default operator is MOP_OR
	while (query.size() > 1) add_op(default_op);

	max_weight = query.top()->recalc_maxweight();
    }
    return query.top();
}

// Convenience wrapper
void
Match::match(doccount first, doccount maxitems,
	     vector<MSetItem> &mset, mset_cmp cmp)
{
    doccount mtotal;
    match(first, maxitems, mset, cmp, &mtotal);
}

// This is the method which runs the query, generating the M set
void
Match::match(doccount first, doccount maxitems,
	     vector<MSetItem> &mset, mset_cmp cmp,  doccount *mtotal)
{
    Assert(maxitems > 0);

    MSetCmp mcmp(cmp);

    // Prepare query
    *mtotal = 0;
    mset.clear();

    PostList * merger = build_query();
    if(merger == NULL) return;

    DebugMsg("match.match(" << merger->intro_term_description() << ")" << endl);

    weight w_min = 0;
    if (min_weight_percent >= 0) w_min = min_weight_percent * max_weight / 100;

    doccount max_msize = first + maxitems;

    weight w_max = max_weight;
    recalculate_maxweight = false;

    map<IRKey, MSetItem> collapse_table;

    // Perform query
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

        (*mtotal)++;
	
        weight w = merger->get_weight();
        
        if (w > w_min) {
	    docid did = merger->get_docid();
	    bool add_item = true;
	    MSetItem mitem(w, did);

	    // Item has high enough weight to go in MSet: do collapse if wanted
	    if(do_collapse) {
		IRDocument * irdoc = database->open_document(did);
		IRKey irkey = irdoc->get_key(collapse_key);
		map<IRKey, MSetItem>::iterator oldkey;
		oldkey = collapse_table.find(irkey);
		if(oldkey == collapse_table.end()) {
		    DebugMsg("collapsem: new key: " << irkey.value << endl);
		    // Key not been seen before
		    collapse_table.insert(pair<IRKey, MSetItem>(irkey, mitem));
		} else {
		    MSetItem olditem = (*oldkey).second;
		    if(mcmp.operator()(olditem, mitem)) {
			DebugMsg("collapsem: better exists: " << irkey.value << endl);
			// There's already a better match with this key
			add_item = false;
		    } else {
			// This is best match with this key so far:
			// remove the old one from the MSet
			if(olditem.wt >= w_min) { // FIXME: should use MSetCmp
			    // Old one hasn't fallen out of MSet yet
			    // Scan through (unsorted) MSet looking for entry
			    // FIXME: more efficient way that just scanning?
			    weight olddid = olditem.did;
			    DebugMsg("collapsem: removing " << olddid << ": " << irkey.value << endl);
			    vector<MSetItem>::iterator i = mset.begin();
			    for(;;) {
				if(i->did == olddid) {
				    mset.erase(i);
				    break;
				}
				i++;
			    }
			}
			oldkey->second = mitem;
		    }
		}
	    }

	    if(add_item) {
		mset.push_back(mitem);

		// FIXME: find balance between larger size for more efficient
		// nth_element and smaller size for better w_min optimisations
		if (mset.size() == max_msize * 2) {
		    // find last element we care about
		    DebugMsg("finding nth" << endl);
		    nth_element(mset.begin(), mset.begin() + max_msize,
				mset.end(), mcmp);
		    // erase elements which don't make the grade
		    mset.erase(mset.begin() + max_msize, mset.end());
		    w_min = mset.back().wt;
		    DebugMsg("mset size = " << mset.size() << endl);
		}
	    }
	}
    }

    if (mset.size() > max_msize) {
	// find last element we care about
	DebugMsg("finding nth" << endl);
	nth_element(mset.begin(), mset.begin() + max_msize, mset.end(), mcmp);
	// erase elements which don't make the grade
	mset.erase(mset.begin() + max_msize, mset.end());
    }
    DebugMsg("sorting" << endl);

    if(first > 0) {
	// Remove unwanted leading entries
	if(mset.size() <= first) {
	    mset.clear();
	} else {
	    DebugMsg("finding " << first << "th" << endl);
	    nth_element(mset.begin(), mset.begin() + first, mset.end(), mcmp);
	    // erase the leading ``first'' elements
	    mset.erase(mset.begin(), mset.begin() + first);
	}
    }

    // Need a stable sort, but this is provided by comparison operator
    sort(mset.begin(), mset.end(), mcmp);

    DebugMsg("msize = " << mset.size() << ", mtotal = " << *mtotal << endl);
    if (mset.size()) {
	DebugMsg("max weight in mset = " << mset[0].wt <<
		 ", min weight in mset = " << mset[mset.size() - 1].wt << endl);
    }
    delete merger;
}
