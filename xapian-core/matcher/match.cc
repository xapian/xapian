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

// Compare an OmMSetItem, using a custom function
class MSetCmp {
    public:
	bool (* fn)(const OmMSetItem &a, const OmMSetItem &b);
	MSetCmp(bool (* _fn)(const OmMSetItem &a, const OmMSetItem &b))
		: fn(_fn) {}
	bool operator()(const OmMSetItem &a, const OmMSetItem &b) {
	    return fn(a, b);
	}
};

// Comparison which sorts equally weighted MSetItems in docid order
bool msetcmp_forward(const OmMSetItem &a, const OmMSetItem &b) {
    if(a.wt > b.wt) return true;
    if(a.wt == b.wt) return a.did < b.did;
    return false;
}

// Comparison which sorts equally weighted MSetItems in reverse docid order
bool msetcmp_reverse(const OmMSetItem &a, const OmMSetItem &b) {
    if(a.wt > b.wt) return true;
    if(a.wt == b.wt) return a.did > b.did;
    return false;
}

////////////////////////////////////
// Initialisation and cleaning up //
////////////////////////////////////

OmMatch::OmMatch(IRDatabase *database_)
	: query(NULL),
	  do_collapse(false),
	  have_added_terms(false)
{
    database = database_;
    min_weight_percent = -1;
    rset = NULL;
}

OmMatch::~OmMatch()
{
    while (!weights.empty()) {
	delete(weights.back());
	weights.pop_back();
    }
}

DBPostList *
OmMatch::mk_postlist(const om_termname& tname, RSet * rset)
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

// Make a postlist from a vector of query objects.
// Operation must be either AND or OR.
// Optimise query by building tree carefully.
PostList *
OmMatch::postlist_from_queries(om_queryop op, const vector<OmQuery *> &queries)
{
    Assert(op == OM_MOP_OR || op == OM_MOP_AND);
    Assert(queries.size() >= 2);

    // Open a postlist for each query, and store these postlists in a vector.
    vector<PostList *> postlists;
    postlists.reserve(queries.size());
    vector<OmQuery *>::const_iterator q;
    for(q = queries.begin(); q != queries.end(); q++) {
	postlists.push_back(postlist_from_query(*q));
	DebugMsg("Made postlist: get_termfreq() = " << postlists.back()->get_termfreq() << endl);
    }

    // Build tree
    if (op == OM_MOP_OR) {
	// Build nice tree for OR-ed postlists
	// Want postlists with most entries to be at top of tree, to reduce
	// average number of nodes an entry gets "pulled" through.
	//
	// Put postlists into a priority queue, such that those with greatest
	// term frequency are returned first.
	// FIXME: try using a heap instead (C++ sect 18.8)?

	priority_queue<PostList *, vector<PostList *>, PLPCmpGt> pq;
	vector<PostList *>::const_iterator i;
	for (i = postlists.begin(); i != postlists.end(); i++) {
	    // for an OR, we can just ignore zero freq terms
	    if(*i == NULL) {
		// This shouldn't happen, but would mean that a postlist
		// didn't get opened.  Might as well try and recover.
	    } else if((*i)->get_termfreq() == 0) {
		delete *i;
	    } else {
		pq.push(*i);
	    }
	}

	// If none of the postlists had any entries, return an EmptyPostList.
	if (pq.empty()) {
	    return new EmptyPostList();
	}

	// Build a tree balanced by the term frequencies
	// (similar to building a huffman encoding tree).
	//
	// This scheme reduces the number of objects common terms
	// get "pulled" through, reducing the amount of work done which
	// speeds things up.
	while (true) {
	    PostList *pl = pq.top();
	    pq.pop();
	    if (pq.empty()) {
		return pl;
	    }
	    // NB right is always <= left - we can use this to optimise
	    pl = new OrPostList(pq.top(), pl, this);
	    pq.pop();
	    pq.push(pl);
	}
    } else {
	// Build nice tree for AND-ed terms
	// SORT list into ascending freq order
	// AND last two elements, then AND with each subsequent element
	
	// 
	vector<PostList *>::const_iterator i;
	for (i = postlists.begin(); i != postlists.end(); i++) {
	    if((*i)->get_termfreq() == 0) {
		// a zero freq term => the AND has zero freq
		vector<PostList *>::const_iterator j;
		for (j = postlists.begin(); j != postlists.end(); j++)
		    delete *j;
		postlists.clear();
		break;
	    }
	}

	if (postlists.empty()) {
	    return new EmptyPostList();
	}

	stable_sort(postlists.begin(), postlists.end(), PLPCmpLt());

	PostList *pl = postlists.back();
	postlists.pop_back();
	while (!postlists.empty()) {	    
	    // NB right is always <= left - we use this to optimise
	    pl = new AndPostList(postlists.back(), pl, this);
	    postlists.pop_back();
	}
	return pl;
    }
}

// Make a postlist from a query object - this is called recursively down
// the query tree.
PostList *
OmMatch::postlist_from_query(const OmQuery *query_)
{
    PostList *pl = NULL;

    Assert(!query_->isnull); // This shouldn't happen, because we check the
			 // top level of the query, and isnull should only
			 // ever occur there.

    switch (query_->op) {
	case OM_MOP_LEAF:
	    // Make a postlist for a single term
	    Assert(query_->subqs.size() == 0);
	    if (database->term_exists(query_->tname)) {
		DebugMsg("Leaf: tname = " << query_->tname << endl);
		pl = mk_postlist(query_->tname, rset);
	    } else {
		DebugMsg("Leaf: tname = " << query_->tname << " (not in database)" << endl);
		// Term doesn't exist in this database.  However, we create
		// a (empty) postlist for it to help make distributed searching
		// cleaner (term might exist in other databases).
		// This is similar to using the muscat3.6 zerofreqs option.
		pl = new EmptyPostList();
	    }
	    break;
	case OM_MOP_AND:
	case OM_MOP_OR:
	    // Build a tree of postlists for AND or OR
	    pl = postlist_from_queries(query_->op, query_->subqs);
	    break;
	case OM_MOP_FILTER:
	    Assert(query_->subqs.size() == 2);
	    pl = new FilterPostList(postlist_from_query(query_->subqs[0]),
				    postlist_from_query(query_->subqs[1]),
				    this);
	    break;
	case OM_MOP_AND_NOT:
	    Assert(query_->subqs.size() == 2);
	    pl = new AndNotPostList(postlist_from_query(query_->subqs[0]),
				    postlist_from_query(query_->subqs[1]),
				    this);
	    break;
	case OM_MOP_AND_MAYBE:
	    Assert(query_->subqs.size() == 2);
	    pl = new AndMaybePostList(postlist_from_query(query_->subqs[0]),
				    postlist_from_query(query_->subqs[1]),
				    this);
	    break;
	case OM_MOP_XOR:
	    Assert(query_->subqs.size() == 2);
	    pl = new XorPostList(postlist_from_query(query_->subqs[0]),
				    postlist_from_query(query_->subqs[1]),
				    this);
	    break;
    }
    return pl;
}

////////////////////////
// Building the query //
////////////////////////

void
OmMatch::set_query(const OmQuery *query_)
{
    // Clear existing query
    max_weight = 0;
    if(query) {
	delete query;
	query = NULL;
    }
    
    // Prepare query
    if(!query_->isnull) {
	query = postlist_from_query(query_);
	max_weight = query->recalc_maxweight();
    }
}

///////////////////
// Run the query //
///////////////////

// This method is called by branch postlists when they rebalance
// in order to recalculate the weights in the tree
void
OmMatch::recalc_maxweight()
{
    recalculate_maxweight = true;
}

// This is the method which runs the query, generating the M set
void
OmMatch::match(om_doccount first, om_doccount maxitems,
	       vector<OmMSetItem> & mset, mset_cmp cmp,
	       om_doccount * mbound, om_weight * greatest_wt)
{
    // Prepare query
    *mbound = 0;
    *greatest_wt = 0;
    mset.clear();

    MSetCmp mcmp(cmp);

    if(query == NULL) return;
    if(maxitems == 0) return;

    DebugMsg("match.match(" << query->intro_term_description() << ")" << endl);

    om_weight w_min = 0;
    if (min_weight_percent >= 0) w_min = min_weight_percent * max_weight / 100;

    om_doccount max_msize = first + maxitems;

    om_weight w_max = max_weight;
    recalculate_maxweight = false;

    map<IRKey, OmMSetItem> collapse_table;

    // Perform query
    while (1) {
	if (recalculate_maxweight) {
	    recalculate_maxweight = false;
	    w_max = query->recalc_maxweight();
	    DebugMsg("max possible doc weight = " << w_max << endl);
	    if (w_max < w_min) {
		DebugMsg("*** TERMINATING EARLY (1)" << endl);
		break;
	    }
	}    

	PostList *ret = query->next(w_min);
        if (ret) {
	    delete query;
	    query = ret;

	    DebugMsg("*** REPLACING ROOT" << endl);
	    // no need for a full recalc (unless we've got to do one because
	    // of a prune elsewhere) - we're just switching to a subtree
	    w_max = query->get_maxweight();
	    DebugMsg("max possible doc weight = " << w_max << endl);
            AssertParanoid(recalculate_maxweight || fabs(w_max - query->recalc_maxweight()) < 1e-9);

	    if (w_max < w_min) {
		DebugMsg("*** TERMINATING EARLY (2)" << endl);
		break;
	    }
	}

	if (query->at_end()) break;

        (*mbound)++;
	
        om_weight w = query->get_weight();
        
        if (w > w_min) {
	    om_docid did = query->get_docid();
	    bool add_item = true;
	    OmMSetItem mitem(w, did);

	    // Item has high enough weight to go in MSet: do collapse if wanted
	    if(do_collapse) {
		IRDocument * irdoc = database->open_document(did);
		IRKey irkey = irdoc->get_key(collapse_key);
		map<IRKey, OmMSetItem>::iterator oldkey;
		oldkey = collapse_table.find(irkey);
		if(oldkey == collapse_table.end()) {
		    DebugMsg("collapsem: new key: " << irkey.value << endl);
		    // Key not been seen before
		    collapse_table.insert(pair<IRKey, OmMSetItem>(irkey, mitem));
		} else {
		    OmMSetItem olditem = (*oldkey).second;
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
			    om_weight olddid = olditem.did;
			    DebugMsg("collapsem: removing " << olddid << ": " << irkey.value << endl);
			    vector<OmMSetItem>::iterator i = mset.begin();
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
		if(w > *greatest_wt) *greatest_wt = w;

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

    DebugMsg("msize = " << mset.size() << ", mbound = " << *mbound << endl);
    if (mset.size()) {
	DebugMsg("max weight in mset = " << mset[0].wt <<
		 ", min weight in mset = " << mset[mset.size() - 1].wt << endl);
    }
    delete query;
    query = NULL;
}

// This method runs the query, generating the M set, but doesn't calcualate
// any weights (all weights in result are set to 1)
void
OmMatch::boolmatch(om_doccount first, om_doccount maxitems,
		   vector<OmMSetItem> &mset)
{
    // Prepare query
    mset.clear();

    if(query == NULL) return;
    if(maxitems == 0) return;

    DebugMsg("match.boolmatch(" << query->intro_term_description() << ")" << endl);

    om_doccount max_msize = first + maxitems;

    map<IRKey, OmMSetItem> collapse_table;

    // Perform query
    while(mset.size() < max_msize) {
	PostList *ret = query->next(0);
	if (ret) {
	    delete query;
	    query = ret;

	    DebugMsg("*** REPLACING ROOT" << endl);
	}

	if (query->at_end()) break;

	om_docid did = query->get_docid();
	bool add_item = true;
	OmMSetItem mitem(1.0, did);

	// Item has high enough weight to go in MSet: do collapse if wanted
	if(do_collapse) {
	    IRDocument * irdoc = database->open_document(did);
	    IRKey irkey = irdoc->get_key(collapse_key);
	    map<IRKey, OmMSetItem>::iterator oldkey;
	    oldkey = collapse_table.find(irkey);
	    if(oldkey == collapse_table.end()) {
		DebugMsg("collapsem: new key: " << irkey.value << endl);
		// Key not been seen before
		collapse_table.insert(pair<IRKey, OmMSetItem>(irkey, mitem));
	    } else {
		DebugMsg("collapsem: already exists: " << irkey.value << endl);
		// There's already a better match with this key
		add_item = false;
	    }
	}

	if(add_item) {
	    mset.push_back(mitem);
	}
    }

    Assert (mset.size() <= max_msize);

    if(first > 0) {
	// Remove unwanted leading entries
	if(mset.size() <= first) {
	    mset.clear();
	} else {
	    // erase the leading ``first'' elements
	    mset.erase(mset.begin(), mset.begin() + first);
	}
    }

    DebugMsg("msize = " << mset.size() << endl);
    delete query;
    query = NULL;
}
