/* localmatch.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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
#include "localmatch.h"
#include "omdebug.h"

#include "andpostlist.h"
#include "orpostlist.h"
#include "xorpostlist.h"
#include "andnotpostlist.h"
#include "andmaybepostlist.h"
#include "filterpostlist.h"
#include "phrasepostlist.h"
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

/** Class providing an operator which returns true if a has a (strictly)
 *  greater number of postings than b.
 */
class PLPCmpGt {
    public:
	/** Return true if and only if a has a strictly greater number of
	 *  postings than b.
	 */
        bool operator()(const PostList *a, const PostList *b) {
            return a->get_termfreq() > b->get_termfreq();
        }
};

/** Class providing an operator which returns true if a has a (strictly)
 *  smaller number of postings than b.
 */
class PLPCmpLt {
    public:
	/** Return true if and only if a has a strictly smaller number of
	 *  postings than b.
	 */
        bool operator()(const PostList *a, const PostList *b) {
            return a->get_termfreq() < b->get_termfreq();
        }
};

/** Class providing an operator which returns true if a has a (strictly)
 *  greater termweight than b.
 */
class PlCmpGtTermWt {
    public:
	/** Return true if and only if a has a strictly greater termweight
	 *  than b.
	 */
	bool operator()(const PostList *a, const PostList *b) {
	    om_weight amax = a->get_maxweight();
	    om_weight bmax = b->get_maxweight();
	    DebugMsg("termweights are: " << amax << " and " << bmax << endl);
	    return amax > bmax;
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

// FIXME: postlists passed by reference and needs to be kept "nice"
// for NearPostList - so if there's a zero freq term it needs to be
// empty, else it needs to have all the postlists in (though the order
// can be different) - this is ultra-icky
PostList *
LocalMatch::build_and_tree(std::vector<PostList *> &postlists)
{
    // Build nice tree for AND-ed terms
    // SORT list into ascending freq order
    // AND last two elements, then AND with each subsequent element
	
    std::vector<PostList *>::const_iterator i;
    for (i = postlists.begin(); i != postlists.end(); i++) {
	if ((*i)->get_termfreq() == 0) {
	    // a zero freq term => the AND has zero freq
	    for (i = postlists.begin(); i != postlists.end(); i++)
		delete *i;
	    postlists.clear();
	    break;
	}
    }

    if (postlists.empty()) return new EmptyPostList();
    
    std::stable_sort(postlists.begin(), postlists.end(), PLPCmpLt());

    int j = postlists.size() - 1;
    PostList *pl = postlists[j];
    while (j > 0) {
	j--;
	// NB right is always <= left - we use this to optimise.
	pl = new AndPostList(postlists[j], pl, this);
    }
    return pl;
}

PostList *
LocalMatch::build_or_tree(std::vector<PostList *> &postlists)
{
    // Build nice tree for OR-ed postlists
    // Want postlists with most entries to be at top of tree, to reduce
    // average number of nodes an entry gets "pulled" through.
    //
    // Put postlists into a priority queue, such that those with greatest
    // term frequency are returned first.
    // FIXME: try using a heap instead (C++ sect 18.8)?

    std::priority_queue<PostList *, std::vector<PostList *>, PLPCmpGt> pq;
    std::vector<PostList *>::const_iterator i;
    for (i = postlists.begin(); i != postlists.end(); i++) {
	// for an OR, we can just ignore zero freq terms
	if ((*i)->get_termfreq() == 0) {
	    delete *i;
	} else {
	    pq.push(*i);
	}
    }
    postlists.clear();

    // If none of the postlists had any entries, return an EmptyPostList.
    if (pq.empty()) return new EmptyPostList();

    // Build a tree balanced by the term frequencies
    // (similar to building a huffman encoding tree).
    //
    // This scheme reduces the number of objects common terms
    // get "pulled" through, reducing the amount of work done which
    // speeds things up.
    while (true) {
	PostList *pl = pq.top();
	pq.pop();
	if (pq.empty()) return pl;
	// NB right is always <= left - we can use this to optimise
	pl = new OrPostList(pq.top(), pl, this);
	pq.pop();
	pq.push(pl);
    }
}

////////////////////////////////////
// Initialisation and cleaning up //
////////////////////////////////////

LocalMatch::LocalMatch(IRDatabase *database_)
	: database(database_),
	  statssource(),
	  min_weight_percent(-1),
	  max_weight_needs_calc(true),
	  query(0),
	  users_query(),
	  extra_weight(0),
	  rset(0),
	  requested_weighting(IRWeight::WTTYPE_BM25),
	  do_collapse(false),
	  max_or_terms(0),
	  mcmp(msetcmp_forward)
{
    statssource.my_collection_size_is(database->get_doccount());
    statssource.my_average_length_is(database->get_avlength());
}

void
LocalMatch::link_to_multi(StatsGatherer *gatherer)
{
    Assert(!is_prepared);
    statssource.connect_to_gatherer(gatherer);
}

LocalMatch::~LocalMatch()
{
    del_query_tree();
}

LeafPostList *
LocalMatch::mk_postlist(const om_termname& tname)
{
    // FIXME - this should be centralised into a postlist factory
    LeafPostList * pl = database->open_post_list(tname);

    // FIXME - query size is currently fixed as 1
    // FIXME - want to use within query frequency here.
    IRWeight * wt = mk_weight(1, tname);

    pl->set_termweight(wt);
    return pl;
}


void
LocalMatch::mk_extra_weight()
{
    if(extra_weight == 0) {
	extra_weight = mk_weight(1, "");
    }
}

IRWeight *
LocalMatch::mk_weight(om_doclength querysize_,
		      om_termname tname_)
{
    IRWeight * wt = IRWeight::create(actual_weighting);
    weights.push_back(wt); // Remember it for deleting
    wt->set_stats(&statssource, querysize_, tname_);
    return wt;
}

void
LocalMatch::del_query_tree()
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
LocalMatch::set_options(const OmMatchOptions & moptions_)
{
    Assert(!is_prepared);
    Assert(query == NULL);

    if(moptions_.percent_cutoff > 0) {
	min_weight_percent = moptions_.percent_cutoff;
    }

    if(moptions_.do_collapse) {
	do_collapse = true;
	collapse_key = moptions_.collapse_key;
    }

    max_or_terms = moptions_.max_or_terms;

    mcmp = moptions_.get_sort_comparator();
}


void
LocalMatch::set_rset(const OmRSet & omrset)
{
    Assert(!is_prepared);
    del_query_tree();
    std::auto_ptr<RSet> new_rset(new RSet(database, omrset));
    rset = new_rset;
}

void
LocalMatch::set_weighting(IRWeight::weight_type wt_type_)
{
    Assert(!is_prepared);
    Assert(query == NULL);
    requested_weighting = wt_type_;
    max_weight_needs_calc = true;
    del_query_tree();
}

// Make a postlist from a vector of query objects.
// Operation must be either AND or OR.
// Optimise query by building tree carefully.
PostList *
LocalMatch::postlist_from_queries(om_queryop op,
				  const std::vector<OmQueryInternal *> &queries,
				  om_termcount window)
{
    Assert(op == OM_MOP_OR || op == OM_MOP_AND || op == OM_MOP_NEAR ||
	   op == OM_MOP_PHRASE);
    Assert(queries.size() >= 2);

    // Open a postlist for each query, and store these postlists in a vector.
    std::vector<PostList *> postlists;
    postlists.reserve(queries.size());

    std::vector<OmQueryInternal *>::const_iterator q;
    for (q = queries.begin(); q != queries.end(); q++) {
	postlists.push_back(postlist_from_query(*q));
	DebugMsg("Made postlist: get_termfreq() = " <<
		 postlists.back()->get_termfreq() << endl);
    }

    // Build tree
    switch (op) {
	case OM_MOP_AND:
	    return build_and_tree(postlists);

	case OM_MOP_NEAR:
	{
	    PostList *res = build_and_tree(postlists);
	    // FIXME: handle EmptyPostList return specially?
	    return new NearPostList(res, window, postlists);
	}

	case OM_MOP_PHRASE:
	{
	    PostList *res = build_and_tree(postlists);
	    // FIXME: handle EmptyPostList return specially?
	    return new PhrasePostList(res, window, postlists);
	}
    
	case OM_MOP_OR:
	    if (max_or_terms != 0) {
		// Select top terms
		DebugMsg("Selecting top " << max_or_terms << " terms, out of " <<
			 postlists.size() << "." << endl);
		if (postlists.size() > max_or_terms) {
		    // Call recalc_maxweight() as otherwise get_maxweight()
		    // may not be valid before next() or skip_to()
		    std::vector<PostList *>::iterator j;
		    for (j = postlists.begin(); j != postlists.end(); j++)
			(*j)->recalc_maxweight();
		    std::nth_element(postlists.begin(),
				postlists.begin() + max_or_terms,
				postlists.end(), PlCmpGtTermWt());
		    DebugMsg("Discarding " << (postlists.size() - max_or_terms) <<
			     " terms." << endl);

		    std::vector<PostList *>::const_iterator i;
	 	    for (i = postlists.begin() + max_or_terms;
			 i != postlists.end(); i++) {
			delete *i;
		    }
		    postlists.erase(postlists.begin() + max_or_terms,
				    postlists.end());
	 	}
	     }

	     return build_or_tree(postlists);

	default:
	    Assert(0);
    }
}

// Make a postlist from a query object - this is called recursively down
// the query tree.
PostList *
LocalMatch::postlist_from_query(const OmQueryInternal *query_)
{
    // This should be true happen, because we check the
    // top level of the query, and !isdefined should only
    // ever occur there.
    Assert(query_->isdefined);

    switch (query_->op) {
	case OM_MOP_LEAF:
	    // Make a postlist for a single term
	    Assert(query_->subqs.size() == 0);
	    if (!database->term_exists(query_->tname)) {
		DebugMsg("Leaf: tname = " << query_->tname << " (not in database)" << endl);
		// Term doesn't exist in this database.  However, we create
		// a (empty) postlist for it to help make distributed searching
		// cleaner (term might exist in other databases).
		// This is similar to using the muscat3.6 zerofreqs option.
		return new EmptyPostList();
	    }
	    DebugMsg("Leaf: tname = " << query_->tname << endl);
	    return mk_postlist(query_->tname);
	case OM_MOP_AND:
	case OM_MOP_OR:
	case OM_MOP_PHRASE:
	case OM_MOP_NEAR:
	    // Build a tree of postlists for AND, OR, PHRASE, or NEAR
	    return postlist_from_queries(query_->op, query_->subqs,
					 query_->window);
	case OM_MOP_FILTER:
	    Assert(query_->subqs.size() == 2);
	    return new FilterPostList(postlist_from_query(query_->subqs[0]),
				      postlist_from_query(query_->subqs[1]),
				      this);
	case OM_MOP_AND_NOT:
	    Assert(query_->subqs.size() == 2);
	    return new AndNotPostList(postlist_from_query(query_->subqs[0]),
				      postlist_from_query(query_->subqs[1]),
				      this);
	case OM_MOP_AND_MAYBE:
	    Assert(query_->subqs.size() == 2);
	    return new AndMaybePostList(postlist_from_query(query_->subqs[0]),
					postlist_from_query(query_->subqs[1]),
					this);
	    break;
	case OM_MOP_XOR:
	    Assert(query_->subqs.size() == 2);
	    return new XorPostList(postlist_from_query(query_->subqs[0]),
				   postlist_from_query(query_->subqs[1]),
				   this);
    }
    Assert(false);
}

////////////////////////
// Building the query //
////////////////////////

void
LocalMatch::set_query(const OmQueryInternal *query_)
{
    Assert(!is_prepared);
    // Clear existing query
    max_weight = 0;
    if(query) {
	delete query;
	query = NULL;
    }
    max_weight_needs_calc = true;
    del_query_tree();

    // If query is boolean, set weighting to boolean
    if(query_->is_bool()) {
	actual_weighting = IRWeight::WTTYPE_BOOL;
    } else {
	actual_weighting = requested_weighting;
    }

    // Remember query
    users_query = *query_;
}

void
LocalMatch::gather_query_statistics()
{
    om_termname_list terms = users_query.get_terms();

    om_termname_list::const_iterator tname;
    for (tname = terms.begin(); tname != terms.end(); tname++) {
	statssource.my_termfreq_is(*tname, database->get_termfreq(*tname));
	if(rset.get() != 0) rset->will_want_reltermfreq(*tname);
    }

    if (rset.get() != 0) {
	rset->calculate_stats();
	rset->give_stats_to_statssource(statssource);
    }
}

void
LocalMatch::build_query_tree()
{
    if (query == 0) {
	select_query_terms();

	DebugMsg("LocalMatch::build_query_tree()" << endl);
	query = postlist_from_query(&users_query);
	DebugMsg("LocalMatch::query = (" << query->intro_term_description() <<
		 ")" << endl);
    }
}

void
LocalMatch::select_query_terms()
{
    term_weights.clear();
    if(max_or_terms != 0) {
	om_termname_list terms = users_query.get_terms();

	om_termname_list::const_iterator tname;
	for (tname = terms.begin(); tname != terms.end(); tname++) {
	    IRWeight * wt = mk_weight(1, *tname);
	    term_weights.insert(std::make_pair(*tname, wt->get_maxpart()));
	    DebugMsg("TERM `" <<  *tname << "' get_maxpart = " <<
		     wt->get_maxpart() << endl);
	}
    }
}

///////////////////
// Run the query //
///////////////////

// This method is called by branch postlists when they rebalance
// in order to recalculate the weights in the tree
void
LocalMatch::recalc_maxweight()
{
    recalculate_maxweight = true;
}

// Internal method to perform the collapse operation
inline bool
LocalMatch::perform_collapse(std::vector<OmMSetItem> &mset,
         		  std::map<OmKey, OmMSetItem> &collapse_table,
			  om_docid did,
			  const OmMSetItem &new_item,
			  const OmMSetItem &min_item)
{
    // Don't collapse on null key
    if(new_item.collapse_key.value.size() == 0) return true;

    bool add_item = true;
    std::map<OmKey, OmMSetItem>::iterator oldkey;
    oldkey = collapse_table.find(new_item.collapse_key);
    if(oldkey == collapse_table.end()) {
	DebugMsg("collapsem: new key: " << new_item.collapse_key.value << endl);
	// Key not been seen before
	collapse_table.insert(std::pair<OmKey, OmMSetItem>(new_item.collapse_key, new_item));
    } else {
	const OmMSetItem olditem = (*oldkey).second;
	if(mcmp(olditem, new_item)) {
	    DebugMsg("collapsem: better exists: " << new_item.collapse_key.value << endl);
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
		DebugMsg("collapsem: removing " << olddid << ": " << new_item.collapse_key.value << endl);
		std::vector<OmMSetItem>::iterator i = mset.begin();
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

bool
LocalMatch::prepare_match(bool nowait)
{
    if(!is_prepared) {
	DebugMsg("LocalMatch::prepare_match() - Gathering my statistics" << endl);
	gather_query_statistics();
	is_prepared = true;
    }
    return true;
}

// Return the maximum possible weight, calculating it if necessary.
om_weight
LocalMatch::get_max_weight()
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
bool
LocalMatch::get_mset(om_doccount first,
		    om_doccount maxitems,
		    std::vector<OmMSetItem> & mset,
		    om_doccount * mbound,
		    om_weight * greatest_wt,
		    const OmMatchDecider *mdecider,
		    bool nowait)
{
    // Check that we have prepared to run the query
    Assert(is_prepared);

    build_query_tree();
    Assert(query != 0);

    // Empty result set
    *mbound = 0;
    *greatest_wt = 0;
    mset.clear();

    // Check that we have a valid query to run
    if(!(users_query.isdefined)) {
	throw OmInvalidArgumentError("Query is not defined.");
    }

    // Check that any results have been asked for (might just be wanting
    // maxweight)
    if(maxitems == 0) {
	return true;
    }

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
    for(std::vector<IRWeight *>::const_iterator i = weights.begin();
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
    std::map<OmKey, OmMSetItem> collapse_table;

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
	DebugMsg("database->get_doclength(" << did << ") == " <<
		 database->get_doclength(did) << endl);
	DebugMsg("query->get_doclength() == " <<
		 query->get_doclength() << endl);
	AssertEqDouble(database->get_doclength(did), query->get_doclength());
        om_weight wt = query->get_weight() +
		extra_weight->get_sumextra(query->get_doclength());
//		extra_weight->get_sumextra(database->get_doclength(did));
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
		new_item.collapse_key = irdoc.get()->get_key(collapse_key);
		add_item = perform_collapse(mset, collapse_table, did,
					    new_item, min_item);
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
		    std::nth_element(mset.begin(), mset.begin() + max_msize,
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
	std::nth_element(mset.begin(), mset.begin() + max_msize, mset.end(), mcmp);
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
	    std::nth_element(mset.begin(), mset.begin() + first, mset.end(), mcmp);
	    // erase the leading ``first'' elements
	    mset.erase(mset.begin(), mset.begin() + first);
	}
    }

    // Need a stable sort, but this is provided by comparison operator
    std::sort(mset.begin(), mset.end(), mcmp);

    DebugMsg("msize = " << mset.size() << ", mbound = " << *mbound << endl);
    if (mset.size()) {
	DebugMsg("max weight in mset = " << mset[0].wt <<
		 ", min weight in mset = " << mset[mset.size() - 1].wt << endl);
    }

    // Query now needs to be recalculated if it is needed again.
    delete query;
    query = 0;

    return true;
}
