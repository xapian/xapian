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

// Compare an OMMSetItem, using a custom function
class MSetCmp {
    public:
	bool (* fn)(const OMMSetItem &a, const OMMSetItem &b);
	MSetCmp(bool (* _fn)(const OMMSetItem &a, const OMMSetItem &b))
		: fn(_fn) {}
	bool operator()(const OMMSetItem &a, const OMMSetItem &b) {
	    return fn(a, b);
	}
};

// Comparison which sorts equally weighted MSetItems in docid order
bool msetcmp_forward(const OMMSetItem &a, const OMMSetItem &b) {
    if(a.wt > b.wt) return true;
    if(a.wt == b.wt) return a.did < b.did;
    return false;
}

// Comparison which sorts equally weighted MSetItems in reverse docid order
bool msetcmp_reverse(const OMMSetItem &a, const OMMSetItem &b) {
    if(a.wt > b.wt) return true;
    if(a.wt == b.wt) return a.did > b.did;
    return false;
}

////////////////////////////////////
// Initialisation and cleaning up //
////////////////////////////////////

OMMatch::OMMatch(IRDatabase *_database)
	: do_collapse(false),
	  have_added_terms(false),
	  query_ready(false)
{
    database = _database;
    min_weight_percent = -1;
    rset = NULL;
}

OMMatch::~OMMatch()
{
    while (!weights.empty()) {
	delete(weights.back());
	weights.pop_back();
    }
}

DBPostList *
OMMatch::mk_postlist(const termname& tname, RSet * rset)
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
OMMatch::postlist_from_queries(om_queryop op, const vector<OMQuery *> &queries)
{
    Assert(op == OM_MOP_OR || op == OM_MOP_AND);
    Assert(queries.size() >= 2);

    // Open a postlist for each query, and store these postlists in a vector.
    vector<PostList *> postlists;
    postlists.reserve(queries.size());
    vector<OMQuery *>::const_iterator q;
    for(q = queries.begin(); q != queries.end(); q++) {
	postlists.push_back(postlist_from_query(*q));
	cout << "Made postlist: get_termfreq() = " << postlists.back()->get_termfreq() << endl;
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

// Make a postlist from a query object
PostList *
OMMatch::postlist_from_query(const OMQuery *qu)
{
    PostList *pl = NULL;
    cout << "Op:" << qu->op << " Size:" << qu->subqs.size() << endl;
    Assert(!qu->isnull); // FIXME Throw exception, rather than assert
    switch (qu->op) {
	case OM_MOP_LEAF:
	    // Make a postlist for a single term
	    Assert(qu->subqs.size() == 0);
	    if (database->term_exists(qu->tname)) {
		cout << "Leaf: tname = " << qu->tname << endl;
		pl = mk_postlist(qu->tname, rset);
	    } else {
		cout << "Leaf: tname = " << qu->tname << " (not in database)" << endl;
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
	    pl = postlist_from_queries(qu->op, qu->subqs);
	    break;
	case OM_MOP_FILTER:
	    Assert(qu->subqs.size() == 2);
	    pl = new FilterPostList(postlist_from_query(qu->subqs[0]),
				    postlist_from_query(qu->subqs[1]),
				    this);
	    break;
	case OM_MOP_AND_NOT:
	    Assert(qu->subqs.size() == 2);
	    pl = new AndNotPostList(postlist_from_query(qu->subqs[0]),
				    postlist_from_query(qu->subqs[1]),
				    this);
	    break;
	case OM_MOP_AND_MAYBE:
	    Assert(qu->subqs.size() == 2);
	    pl = new AndMaybePostList(postlist_from_query(qu->subqs[0]),
				    postlist_from_query(qu->subqs[1]),
				    this);
	    break;
	case OM_MOP_XOR:
	    Assert(qu->subqs.size() == 2);
	    pl = new XorPostList(postlist_from_query(qu->subqs[0]),
				    postlist_from_query(qu->subqs[1]),
				    this);
	    break;
    }
    return pl;
}

////////////////////////
// Building the query //
////////////////////////

void
OMMatch::set_query(const OMQuery *qu)
{
    query.push(postlist_from_query(qu));
}

///////////////////
// Run the query //
///////////////////

// This method is called by branch postlists when they rebalance
// in order to recalculate the weights in the tree
void
OMMatch::recalc_maxweight()
{
    recalculate_maxweight = true;
}

// Prepare the query for running, if it isn't already ready
PostList *
OMMatch::build_query()
{
    if(!query_ready) {
	query_ready = true;
	max_weight = 0;

	if (query.size() == 0) return NULL; // No query

	Assert(query.size() == 1);
	max_weight = query.top()->recalc_maxweight();
    }
    return query.top();
}

// This is the method which runs the query, generating the M set
void
OMMatch::match(doccount first, doccount maxitems,
	     vector<OMMSetItem> &mset, mset_cmp cmp,  doccount *mbound)
{
    Assert(maxitems > 0);

    MSetCmp mcmp(cmp);

    // Prepare query
    *mbound = 0;
    mset.clear();

    PostList * merger = build_query();
    if(merger == NULL) return;

    DebugMsg("match.match(" << merger->intro_term_description() << ")" << endl);

    weight w_min = 0;
    if (min_weight_percent >= 0) w_min = min_weight_percent * max_weight / 100;

    doccount max_msize = first + maxitems;

    weight w_max = max_weight;
    recalculate_maxweight = false;

    map<IRKey, OMMSetItem> collapse_table;

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

        (*mbound)++;
	
        weight w = merger->get_weight();
        
        if (w > w_min) {
	    docid did = merger->get_docid();
	    bool add_item = true;
	    OMMSetItem mitem(w, did);

	    // Item has high enough weight to go in MSet: do collapse if wanted
	    if(do_collapse) {
		IRDocument * irdoc = database->open_document(did);
		IRKey irkey = irdoc->get_key(collapse_key);
		map<IRKey, OMMSetItem>::iterator oldkey;
		oldkey = collapse_table.find(irkey);
		if(oldkey == collapse_table.end()) {
		    DebugMsg("collapsem: new key: " << irkey.value << endl);
		    // Key not been seen before
		    collapse_table.insert(pair<IRKey, OMMSetItem>(irkey, mitem));
		} else {
		    OMMSetItem olditem = (*oldkey).second;
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
			    vector<OMMSetItem>::iterator i = mset.begin();
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

    DebugMsg("msize = " << mset.size() << ", mbound = " << *mbound << endl);
    if (mset.size()) {
	DebugMsg("max weight in mset = " << mset[0].wt <<
		 ", min weight in mset = " << mset[mset.size() - 1].wt << endl);
    }
    delete merger;
}
