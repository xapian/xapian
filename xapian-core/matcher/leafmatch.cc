/* leafmatch.cc
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

#include "config.h"
#include "leafmatch.h"

#include "andpostlist.h"
#include "orpostlist.h"
#include "xorpostlist.h"
#include "andnotpostlist.h"
#include "andmaybepostlist.h"
#include "filterpostlist.h"
#include "emptypostlist.h"
#include "leafpostlist.h"

#include "document.h"
#include "rset.h"
#include "omqueryinternal.h"
#include "omdocumentparams.h"

#include "match.h"
#include "stats.h"
#include "irweight.h"
#include "bm25weight.h"
//#include "tradweight.h"

#include <algorithm>
#include <memory>
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

LeafMatch::LeafMatch(IRDatabase *database_)
	: database(database_),
	  statsleaf(),
	  min_weight_percent(-1),
	  max_weight_needs_calc(true),
	  query(0),
	  users_query(),
	  extra_weight(0),
	  rset(0),
	  wt_type(IRWeight::WTTYPE_BM25),
	  do_collapse(false)
{
}

void
LeafMatch::link_to_multi(StatsGatherer *gatherer)
{
    Assert(!is_prepared);
    statsleaf.connect_to_gatherer(gatherer);
    statsleaf.my_collection_size_is(database->get_doccount());
    statsleaf.my_average_length_is(database->get_avlength());
}

LeafMatch::~LeafMatch()
{
    del_query_tree();
}

LeafPostList *
LeafMatch::mk_postlist(const om_termname& tname, RSet * rset)
{
    // FIXME - this should be centralised into a postlist factory
    LeafPostList * pl = database->open_post_list(tname, rset);
    if(rset) rset->will_want_termfreq(tname);

    IRWeight * wt = mk_weight(1, tname, rset);
    statsleaf.my_termfreq_is(tname, pl->get_termfreq());
    // Query size of 1 for now.  FIXME
    pl->set_termweight(wt);
    return pl;
}


void
LeafMatch::mk_extra_weight()
{
    if(extra_weight == 0) {
	extra_weight = mk_weight(1, "", rset);
    }
}

IRWeight *
LeafMatch::mk_weight(om_doclength querysize_,
		     om_termname tname_,
		     const RSet * rset_)
{
    IRWeight * wt = IRWeight::create(wt_type);
    //IRWeight * wt = new TradWeight();
    weights.push_back(wt); // Remember it for deleting
    wt->set_stats(&statsleaf, querysize_, tname_, rset_);
    return wt;
}

void
LeafMatch::del_query_tree()
{
    delete query;
    query = 0;

    extra_weight = 0;
    while (!weights.empty()) {
	delete(weights.back());
	weights.pop_back();
    }
}

/////////////////////////////////////////////////////////////////////
// Setting query options
//
void
LeafMatch::set_collapse_key(om_keyno key)
{
    Assert(!is_prepared);
    do_collapse = true;
    collapse_key = key;
}

void
LeafMatch::set_no_collapse()
{
    Assert(!is_prepared);
    do_collapse = false;
}

void
LeafMatch::set_min_weight_percent(int pcent)
{
    Assert(!is_prepared);
    min_weight_percent = pcent;
}

void
LeafMatch::set_rset(RSet *rset_)
{
    Assert(!is_prepared);
    Assert(query == NULL);
    rset = rset_;
    del_query_tree();
}

void
LeafMatch::set_weighting(IRWeight::weight_type wt_type_)
{
    Assert(!is_prepared);
    Assert(query == NULL);
    wt_type = wt_type_;
    max_weight_needs_calc = true;
    del_query_tree();
}

// Make a postlist from a vector of query objects.
// Operation must be either AND or OR.
// Optimise query by building tree carefully.
PostList *
LeafMatch::postlist_from_queries(om_queryop op,
			       const vector<OmQueryInternal *> &queries)
{
    Assert(op == OM_MOP_OR || op == OM_MOP_AND);
    Assert(queries.size() >= 2);

    // Open a postlist for each query, and store these postlists in a vector.
    vector<PostList *> postlists;
    postlists.reserve(queries.size());
    vector<OmQueryInternal *>::const_iterator q;
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
LeafMatch::postlist_from_query(const OmQueryInternal *query_)
{
    PostList *pl = NULL;

    // This should be true happen, because we check the
    // top level of the query, and !isdefined should only
    // ever occur there.
    Assert(query_->isdefined);

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
LeafMatch::set_query(const OmQueryInternal *query_)
{
    Assert(!is_prepared);
    // Clear existing query
    max_weight = 0;
    if(query) {
	delete query;
	query = NULL;
    }

    // Remember query
    users_query = *query_;
}

/// Build the query tree, if it isn't already built.
void
LeafMatch::build_query_tree()
{
    if (query == 0) {
	query = postlist_from_query(&users_query);
    }

    DebugMsg("LeafMatch::query = (" << query->intro_term_description() <<
	     ")" << endl);
}

///////////////////
// Run the query //
///////////////////

// This method is called by branch postlists when they rebalance
// in order to recalculate the weights in the tree
void
LeafMatch::recalc_maxweight()
{
    recalculate_maxweight = true;
}

// Internal method to perform the collapse operation
inline bool
LeafMatch::perform_collapse(vector<OmMSetItem> &mset,
         		  map<OmKey, OmMSetItem> &collapse_table,
			  om_docid did,
			  const OmMSetItem &new_item,
			  const MSetCmp &mcmp,
			  const OmMSetItem &min_item,
			  const LeafDocument *irdoc)
{
    OmKey irkey = irdoc->get_key(collapse_key);

    // Don't collapse on null key
    if(irkey.value.size() == 0) return true;

    bool add_item = true;
    map<OmKey, OmMSetItem>::iterator oldkey;
    oldkey = collapse_table.find(irkey);
    if(oldkey == collapse_table.end()) {
	DebugMsg("collapsem: new key: " << irkey.value << endl);
	// Key not been seen before
	collapse_table.insert(pair<OmKey, OmMSetItem>(irkey, new_item));
    } else {
	const OmMSetItem olditem = (*oldkey).second;
	if(mcmp(olditem, new_item)) {
	    DebugMsg("collapsem: better exists: " << irkey.value << endl);
	    // There's already a better match with this key
	    add_item = false;
	} else {
	    // This is best match with this key so far:
	    // remove the old one from the MSet
	    if(mcmp(olditem, min_item)) {
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
	    oldkey->second = new_item;
	}
    }
    return add_item;
}

void
LeafMatch::prepare_match()
{
    if(!is_prepared) {
	DebugMsg("LeafMatch::prepare_match() - Building query tree" << cout);
	build_query_tree();
	Assert(query != 0);

	DebugMsg("LeafMatch::prepare_match() - Giving my stats to gatherer" << cout);
	statsleaf.contrib_my_stats();

	is_prepared = true;
    }
}

// Return the maximum possible weight, calculating it if necessary.
om_weight
LeafMatch::get_max_weight()
{
    // Check that we have prepared to run the query
    Assert(is_prepared);
    if (max_weight_needs_calc) {
	Assert(query != 0);

	mk_extra_weight();
	max_extra_weight = extra_weight->get_maxextra();

	max_weight = query->recalc_maxweight() + max_extra_weight;
	max_weight_needs_calc = false;
    }

    return max_weight;
}

// This is the method which runs the query, generating the M set
void
LeafMatch::get_mset(om_doccount first,
		    om_doccount maxitems,
		    vector<OmMSetItem> & mset,
		    mset_cmp cmp,
		    om_doccount * mbound,
		    om_weight * greatest_wt,
		    const OmMatchDecider *mdecider)
{
    // Check that we have prepared to run the query
    Assert(is_prepared);
    Assert(query != 0);

    // Empty result set
    *mbound = 0;
    *greatest_wt = 0;
    mset.clear();

    // Set up comparison operator
    MSetCmp mcmp(cmp);

    // Check that we have a valid query to run
    if(!(users_query.isdefined)) return;

    // Check that any results have been asked for (might just be wanting
    // maxweight)
    if(maxitems == 0) return;

    // Set max number of results that we want - this is used to decide
    // when to throw away unwanted items.
    om_doccount max_msize = first + maxitems;

    // Get initial max weight and the maximum extra weight contribution
    om_weight w_max = get_max_weight();
    recalculate_maxweight = false;

    // Ensure that extra_weight is created
    mk_extra_weight();

#ifdef MUS_DEBUG_PARANOID
    // Check that max_extra weight is really right
    for(vector<IRWeight *>::const_iterator i = weights.begin();
	i != weights.end(); i++) {
	Assert(max_extra_weight == (*i)->get_maxextra());
    }
#endif /* MUS_DEBUG_PARANOID */

    // Set the minimum item, used to compare against to see if an item
    // should be considered for the mset.
    OmMSetItem min_item(-1, 0);
    if (min_weight_percent > 0) {
	min_item.wt = min_weight_percent * max_weight / 100;
    }

    // Table of keys which have been seen already, for collapsing.
    map<OmKey, OmMSetItem> collapse_table;

    // Perform query
    while (1) {
	if (recalculate_maxweight) {
	    recalculate_maxweight = false;
	    w_max = query->recalc_maxweight() + max_extra_weight;
	    DebugMsg("max possible doc weight = " << w_max << endl);
	    if (w_max < min_item.wt) {
		DebugMsg("*** TERMINATING EARLY (1)" << endl);
		break;
	    }
	}    

	PostList *ret = query->next(min_item.wt - max_extra_weight);
        if (ret) {
	    delete query;
	    query = ret;

	    DebugMsg("*** REPLACING ROOT" << endl);
	    // no need for a full recalc (unless we've got to do one because
	    // of a prune elsewhere) - we're just switching to a subtree
	    w_max = query->get_maxweight() + max_extra_weight;
	    DebugMsg("max possible doc weight = " << w_max << endl);
            AssertParanoid(recalculate_maxweight || fabs(w_max - max_extra_weight - query->recalc_maxweight()) < 1e-9);

	    if (w_max < min_item.wt) {
		DebugMsg("*** TERMINATING EARLY (2)" << endl);
		break;
	    }
	}

	if (query->at_end()) break;

        (*mbound)++;
	
	om_docid did = query->get_docid();
	// FIXME: next line is inefficient, due to design.  (Makes it hard /
	// impossible to store document lengths in postlists, so they've
	// already been retrieved)
        om_weight wt = query->get_weight() +
		extra_weight->get_sumextra(database->get_doclength(did));
	OmMSetItem new_item(wt, did);
        
	if(mcmp(new_item, min_item)) {
	    bool add_item = true;

	    OmRefCntPtr<LeafDocument> irdoc;

	    // Use the decision functor if any.
	    if (mdecider != 0) {
		if (irdoc.get() == 0) {
		    OmRefCntPtr<LeafDocument> temp(database->open_document(did));
		    irdoc = temp;
		}
		OmDocument mydoc(irdoc);
		add_item = mdecider->operator()(&mydoc);
	    }

	    // Item has high enough weight to go in MSet: do collapse if wanted
	    if(add_item && do_collapse) {
		if (irdoc.get() == 0) {
		    OmRefCntPtr<LeafDocument> temp(database->open_document(did));
		    irdoc = temp;
		}
		add_item = perform_collapse(mset, collapse_table, did,
					    new_item, mcmp, min_item,
					    irdoc.get());
	    }

	    if(add_item) {
		mset.push_back(new_item);

		// Keep a track of the greatest weight we've seen.
		if(wt > *greatest_wt) *greatest_wt = wt;

		// FIXME: find balance between larger size for more efficient
		// nth_element and smaller size for better minimum weight
		// optimisations
		if (mset.size() == max_msize * 2) {
		    // find last element we care about
		    DebugMsg("finding nth" << endl);
		    nth_element(mset.begin(), mset.begin() + max_msize,
				mset.end(), mcmp);
		    // erase elements which don't make the grade
		    mset.erase(mset.begin() + max_msize, mset.end());
		    min_item = mset.back();
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

    // Query now needs to be recalculated if it is needed again.
    delete query;
    query = 0;
}
