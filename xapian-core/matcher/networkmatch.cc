/* networkmatch.cc
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
#include "networkmatch.h"

#include "stats.h"
#include "utils.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>

////////////////////////////////////
// Initialisation and cleaning up //
////////////////////////////////////

NetworkMatch::NetworkMatch(IRDatabase *database_)
	: database(dynamic_cast<NetworkDatabase *>(database_)),
	  statssource(database->link),
	  max_weight_needs_fetch(true) /*,
	  wt_type(IRWeight::WTTYPE_BM25),
	  do_collapse(false) */
{
    // make sure that the database was a NetworkDatabase after all
    // (dynamic_cast<foo *> returns 0 if the cast fails)
    Assert(database != 0);

    database->link->register_statssource(&statssource);
}

void
NetworkMatch::prepare_match()
{
    if (!is_prepared) {
	database->link->finish_query();

	// Read the remote statistics and give them to the stats source
	//
	statssource.take_remote_stats(database->link->get_remote_stats());

	is_prepared = true;
    }
}

void
NetworkMatch::finish_query()
{
    database->link->finish_query();
}

void
NetworkMatch::link_to_multi(StatsGatherer *gatherer_)
{
    gatherer = gatherer_;
    statssource.connect_to_gatherer(gatherer);
//    statsleaf.my_collection_size_is(database->get_doccount());
//    statsleaf.my_average_length_is(database->get_avlength());
}

//////////////////////////////////////////////////////////
// ########## PAST THIS POINT IMPLEMENTATION ########## //
// ########## IS JUST COPIED FROM LEAFMATCH, ########## //
// ########## AND IS HENCE BOGUS TO THE MAX. ########## //
//////////////////////////////////////////////////////////

NetworkMatch::~NetworkMatch()
{
#if 0
    del_query_tree();
#endif
}

#if 0
LeafPostList *
NetworkMatch::mk_postlist(const om_termname& tname, RSet * rset)
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
#endif


#if 0
void
NetworkMatch::mk_extra_weight()
{
    if(extra_weight == 0) {
	extra_weight = mk_weight(1, "", rset);
    }
}
#endif

#if 0
IRWeight *
NetworkMatch::mk_weight(om_doclength querysize_,
		     om_termname tname_,
		     const RSet * rset_)
{
    IRWeight * wt = IRWeight::create(wt_type);
    //IRWeight * wt = new TradWeight();
    weights.push_back(wt); // Remember it for deleting
    wt->set_stats(&statsleaf, querysize_, tname_, rset_);
    return wt;
}
#endif

#if 0
void
NetworkMatch::del_query_tree()
{
    delete query;
    query = 0;

    extra_weight = 0;
    while (!weights.empty()) {
	delete(weights.back());
	weights.pop_back();
    }
}
#endif

/////////////////////////////////////////////////////////////////////
// Setting query options
//
void
NetworkMatch::set_collapse_key(om_keyno key)
{
    Assert(false);
#if 0
    do_collapse = true;
    collapse_key = key;
#endif
}

void
NetworkMatch::set_no_collapse()
{
    Assert(false);
#if 0
    do_collapse = false;
#endif
}

void
NetworkMatch::set_min_weight_percent(int pcent)
{
    Assert(false);
#if 0
    min_weight_percent = pcent;
#endif
}

void
NetworkMatch::set_rset(RSet *rset_)
{
    Assert(false);
#if 0
    Assert(query == NULL);
    rset = rset_;
    del_query_tree();
#endif
}

void
NetworkMatch::set_weighting(IRWeight::weight_type wt_type_)
{
    database->link->set_weighting(wt_type_);
    max_weight_needs_fetch = true;
}

// Make a postlist from a vector of query objects.
// Operation must be either AND or OR.
// Optimise query by building tree carefully.
#if 0
PostList *
NetworkMatch::postlist_from_queries(om_queryop op,
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
#endif

// Make a postlist from a query object - this is called recursively down
// the query tree.
#if 0
PostList *
NetworkMatch::postlist_from_query(const OmQueryInternal *query_)
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
#endif

////////////////////////
// Building the query //
////////////////////////

void
NetworkMatch::set_query(const OmQueryInternal *query_)
{
    database->link->set_query(query_);
}

/// Build the query tree, if it isn't already built.
#if 0
void
NetworkMatch::build_query_tree()
{
    if (query == 0) {
	query = postlist_from_query(&users_query);
    }

    DebugMsg("NetworkMatch::query = (" << query->intro_term_description() <<
	     ")" << endl);
}
#endif

// Return the maximum possible weight, calculating it if necessary.
om_weight
NetworkMatch::get_max_weight()
{
    Assert(is_prepared);
    if (max_weight_needs_fetch) {
	max_weight = database->link->get_max_weight();
	max_weight_needs_fetch = false;
    }

    return max_weight;
}

///////////////////
// Run the query //
///////////////////

// This method is called by branch postlists when they rebalance
// in order to recalculate the weights in the tree
void
NetworkMatch::recalc_maxweight()
{
    Assert(false);
#if 0
    recalculate_maxweight = true;
#endif
}

// Internal method to perform the collapse operation
#if 0
inline bool
NetworkMatch::perform_collapse(vector<OmMSetItem> &mset,
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
#endif

// This is the method which runs the query, generating the M set
void
NetworkMatch::get_mset(om_doccount first,
		       om_doccount maxitems,
		       vector<OmMSetItem> & mset,
		       mset_cmp cmp,
		       om_doccount * mbound,
		       om_weight * greatest_wt,
		       const OmMatchDecider *mdecider)
{
    Assert(is_prepared);

    if (mdecider != 0) {
	throw OmInvalidArgumentError("Can't use a match decider remotely");
    }

    database->link->send_global_stats(*(gatherer->get_stats()));

    database->link->get_mset(first, maxitems, mset, mbound, greatest_wt);
}
