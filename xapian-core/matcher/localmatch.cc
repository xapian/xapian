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
	    DEBUGLINE(MATCH, "termweights are: " << amax << " and " << bmax);
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

    if (postlists.empty()) {
	EmptyPostList *pl = new EmptyPostList();
	pl->set_termweight(mk_weight());
	return pl;
    }

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
    if (pq.empty()) {
	EmptyPostList *pl = new EmptyPostList();
	pl->set_termweight(mk_weight());
	return pl;
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
	  users_query(),
	  rset(0),
	  requested_weighting("bm25"),
	  do_collapse(false),
	  max_or_terms(0),
	  mcmp(msetcmp_forward),
	  querysize(1)
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

PostList *
LocalMatch::mk_postlist(const om_termname & tname, om_termcount wqf)
{
    DEBUGLINE(MATCH, "LocalMatch::mk_postlist(" << tname << ", " << wqf << ")");

    LeafPostList * pl = database->open_post_list(tname);

    // FIXME: pass the weight type and the info needed to create it to the
    // postlist instead
    IRWeight * wt = mk_weight(tname, wqf);
    om_weight term_weight = wt->get_maxpart();

    pl->set_termweight(wt);

    om_doccount term_freq = statssource.get_total_termfreq(tname);

    term_weights.insert(std::make_pair(tname, term_weight));
    term_frequencies.insert(std::make_pair(tname, term_freq));

    DEBUGLINE(MATCH, " weight = " << term_weight <<
	      ", frequency = " << term_freq);

    return pl;
}

IRWeight *
LocalMatch::mk_weight(om_termname tname_, om_termcount wqf_)
{
    IRWeight * wt = IRWeight::create(actual_weighting);
    wt->set_stats(&statssource, querysize, wqf_, tname_);
    return wt;
}

/////////////////////////////////////////////////////////////////////
// Setting query options
//

void
LocalMatch::set_options(const OmSettings & mopts)
{
    Assert(!is_prepared);

    int val = mopts.get_int("match_percent_cutoff", 0);
    if (val > 0) {
	min_weight_percent = val;
    }

    val = mopts.get_int("match_collapse_key", -1);
    if (val >= 0) {
	do_collapse = true;
	collapse_key = val;
    }

    max_or_terms = mopts.get_int("match_max_or_terms", 0);

    if (mopts.get_bool("match_sort_forward", true)) {
	mcmp = OmMSetCmp(msetcmp_forward);
    } else {
	mcmp = OmMSetCmp(msetcmp_reverse);
    }

    requested_weighting = mopts.get("match_weighting_scheme", "bm25");
}


void
LocalMatch::set_rset(const OmRSet & omrset)
{
    Assert(!is_prepared);
    std::auto_ptr<RSet> new_rset(new RSet(database, omrset));
    rset = new_rset;
}

// Make a postlist from a vector of query objects.
// Operation must be either AND or OR.
// Optimise query by building tree carefully.
PostList *
LocalMatch::postlist_from_queries(OmQuery::op op,
				  const std::vector<OmQueryInternal *> &queries,
				  om_termcount window)
{
    Assert(op == OmQuery::OP_OR || op == OmQuery::OP_AND || op == OmQuery::OP_NEAR ||
	   op == OmQuery::OP_PHRASE);
    Assert(queries.size() >= 2);

    // Open a postlist for each query, and store these postlists in a vector.
    std::vector<PostList *> postlists;
    postlists.reserve(queries.size());

    std::vector<OmQueryInternal *>::const_iterator q;
    for (q = queries.begin(); q != queries.end(); q++) {
	postlists.push_back(postlist_from_query(*q));
	DEBUGLINE(MATCH, "Made postlist: get_termfreq() = " <<
		  postlists.back()->get_termfreq());
    }

    // Build tree
    switch (op) {
	case OmQuery::OP_AND:
	    return build_and_tree(postlists);

	case OmQuery::OP_NEAR:
	{
	    PostList *res = build_and_tree(postlists);
	    // FIXME: handle EmptyPostList return specially?
	    return new NearPostList(res, window, postlists);
	}

	case OmQuery::OP_PHRASE:
	{
	    // build_and_tree reorders postlists, but the order is
	    // important for phrase, so we need to keep a copy
	    // FIXME: there must be a cleaner way for this to work...
	    std::vector<PostList *> postlists_orig = postlists;
	    PostList *res = build_and_tree(postlists);
	    // FIXME: handle EmptyPostList return specially?
	    return new PhrasePostList(res, window, postlists_orig);
	}

	case OmQuery::OP_OR:
	    if (max_or_terms != 0) {
		// Select top terms
		DEBUGLINE(API, "Selecting top " << max_or_terms <<
			  " terms, out of " << postlists.size() << ".");
		if (postlists.size() > max_or_terms) {
		    // Call recalc_maxweight() as otherwise get_maxweight()
		    // may not be valid before next() or skip_to()
		    std::vector<PostList *>::iterator j;
		    for (j = postlists.begin(); j != postlists.end(); j++)
			(*j)->recalc_maxweight();
		    std::nth_element(postlists.begin(),
				postlists.begin() + max_or_terms,
				postlists.end(), PlCmpGtTermWt());
		    DEBUGLINE(MATCH, "Discarding " <<
			      (postlists.size() - max_or_terms) <<
			      " terms.");

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
    Assert(0);
    return NULL;
}

// Make a postlist from a query object - this is called recursively down
// the query tree.
PostList *
LocalMatch::postlist_from_query(const OmQueryInternal *query)
{
    // This should never fail, because isdefined should only be false
    // at the root of a query tree
    Assert(query->isdefined);

    switch (query->op) {
	case OmQuery::OP_LEAF:
	    // Make a postlist for a single term
	    Assert(query->subqs.size() == 0);
	    return mk_postlist(query->tname, query->wqf);
	case OmQuery::OP_AND:
	case OmQuery::OP_OR:
	case OmQuery::OP_PHRASE:
	case OmQuery::OP_NEAR:
	    // Build a tree of postlists for AND, OR, PHRASE, or NEAR
	    return postlist_from_queries(query->op, query->subqs,
					 query->window);
	case OmQuery::OP_FILTER:
	    Assert(query->subqs.size() == 2);
	    return new FilterPostList(postlist_from_query(query->subqs[0]),
				      postlist_from_query(query->subqs[1]),
				      this);
	case OmQuery::OP_AND_NOT:
	    Assert(query->subqs.size() == 2);
	    return new AndNotPostList(postlist_from_query(query->subqs[0]),
				      postlist_from_query(query->subqs[1]),
				      this);
	case OmQuery::OP_AND_MAYBE:
	    Assert(query->subqs.size() == 2);
	    return new AndMaybePostList(postlist_from_query(query->subqs[0]),
					postlist_from_query(query->subqs[1]),
					this);
	case OmQuery::OP_XOR:
	    Assert(query->subqs.size() == 2);
	    return new XorPostList(postlist_from_query(query->subqs[0]),
				   postlist_from_query(query->subqs[1]),
				   this);
    }
    Assert(false);
    return NULL;
}

////////////////////////
// Building the query //
////////////////////////

void
LocalMatch::set_query(const OmQueryInternal *query)
{
    Assert(!is_prepared);

    // Clear existing query
    term_weights.clear();
    term_frequencies.clear();

    // If query is boolean, set weighting to boolean
    if (query->is_bool()) {
	actual_weighting = "bool";
    } else {
	actual_weighting = requested_weighting;
    }

    // Remember query
    users_query = *query;
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

///////////////////
// Run the query //
///////////////////

// This method is called by branch postlists when they rebalance
// in order to recalculate the weights in the tree
void
LocalMatch::recalc_maxweight()
{
    recalculate_w_max = true;
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
	DEBUGLINE(MATCH, "collapsem: new key: " << new_item.collapse_key.value);
	// Key not been seen before
	collapse_table.insert(std::pair<OmKey, OmMSetItem>(new_item.collapse_key, new_item));
    } else {
	const OmMSetItem olditem = (*oldkey).second;
	if(mcmp(olditem, new_item)) {
	    DEBUGLINE(MATCH, "collapsem: better exists: " <<
		      new_item.collapse_key.value);
	    // There's already a better match with this key
	    add_item = false;
	} else {
	    // This is best match with this key so far:
	    // remove the old one from the MSet
	    if(mcmp(olditem, min_item)) {
		// Old one hasn't fallen out of MSet yet
		// Scan through (unsorted) MSet looking for entry
		// FIXME: more efficient way that just scanning?
		om_docid olddid = olditem.did;
		DEBUGLINE(MATCH, "collapsem: removing " << olddid <<
			  ": " << new_item.collapse_key.value);
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
	DEBUGLINE(MATCH, "LocalMatch::prepare_match() - Gathering my statistics");
	gather_query_statistics();
	is_prepared = true;
    }
    return true;
}

// This is the method which runs the query, generating the M set
bool
LocalMatch::get_mset(om_doccount first,
		     om_doccount maxitems,
		     OmMSet & mset,
		     const OmMatchDecider *mdecider,
		     bool nowait)
{
    // Check that we have prepared to run the query
    Assert(is_prepared);

    // Check that we have a valid query to run
    if (!(users_query.isdefined)) {
	throw OmInvalidArgumentError("Query is not defined.");
    }

    // Root postlist of query tree
    querysize = users_query.qlen;
    PostList * query = postlist_from_query(&users_query);

    Assert(query != NULL);
    DEBUGLINE(MATCH, "query = (" << query->intro_term_description() << ")");

    // Empty result set
    om_doccount mbound = 0;
    om_weight greatest_wt = 0;
    std::vector<OmMSetItem> items;
    std::map<om_termname, OmMSet::TermFreqAndWeight> termfreqandwts;

    for (std::map<om_termname, om_weight>::const_iterator i = term_weights.begin(); i != term_weights.end(); i++) {
	termfreqandwts[i->first].termweight = i->second;
    }
    for (std::map<om_termname, om_doccount>::const_iterator i = term_frequencies.begin(); i != term_frequencies.end(); i++) {
	termfreqandwts[i->first].termfreq = i->second;
    }

    // Extra weight object - used to calculate part of doc weight which
    // doesn't come from the sum.
    IRWeight * extra_weight = mk_weight();

    // Max "extra weight" that an item can get (ie, not from the postlist tree).
    om_weight max_extra_weight = extra_weight->get_maxextra();
    // Calculate max_weight a document could possibly have
    om_weight max_weight = query->recalc_maxweight() + max_extra_weight;

    om_weight w_max = max_weight; // w_max may decrease as tree is pruned
    recalculate_w_max = false;

    // Check that any results have been asked for (might just be wanting
    // maxweight)
    if (maxitems == 0) {
	delete query;
	delete extra_weight;

	mset = OmMSet(first, mbound, max_weight, greatest_wt, items, termfreqandwts);

	return true;
    }

    // Set max number of results that we want - this is used to decide
    // when to throw away unwanted items.
    om_doccount max_msize = first + maxitems;

    // FIXME: may be able to loop through postlists to do this check now?
#if 0 // def MUS_DEBUG_PARANOID
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
	if (recalculate_w_max) {
	    recalculate_w_max = false;
	    w_max = query->recalc_maxweight() + max_extra_weight;
	    DEBUGLINE(MATCH, "max possible doc weight = " << w_max);
	    if (w_max < min_item.wt) {
		DEBUGLINE(MATCH, "*** TERMINATING EARLY (1)");
		break;
	    }
	}

	PostList *ret = query->next(min_item.wt - max_extra_weight);
        if (ret) {
	    delete query;
	    query = ret;

	    DEBUGLINE(MATCH, "*** REPLACING ROOT");
	    // no need for a full recalc (unless we've got to do one because
	    // of a prune elsewhere) - we're just switching to a subtree
	    w_max = query->get_maxweight() + max_extra_weight;
	    DEBUGLINE(MATCH, "max possible doc weight = " << w_max);
            AssertParanoid(recalculate_w_max || fabs(w_max - max_extra_weight - query->recalc_maxweight()) < 1e-9);

	    if (w_max < min_item.wt) {
		DEBUGLINE(MATCH, "*** TERMINATING EARLY (2)");
		break;
	    }
	}

	if (query->at_end()) break;

        mbound++;

	om_docid did = query->get_docid();
	DEBUGLINE(MATCH, "database->get_doclength(" << did << ") == " <<
		  database->get_doclength(did));
	DEBUGLINE(MATCH, "query->get_doclength() == " <<
		  query->get_doclength());
	AssertEqDouble(database->get_doclength(did), query->get_doclength());
        om_weight wt = query->get_weight() +
		extra_weight->get_sumextra(query->get_doclength());

	OmMSetItem new_item(wt, did);

	if(mcmp(new_item, min_item)) {
	    bool add_item = true;

	    RefCntPtr<LeafDocument> irdoc;

	    // Use the decision functor if any.
	    if (mdecider != 0) {
		if (irdoc.get() == 0) {
		    RefCntPtr<LeafDocument> temp(database->open_document(did));
		    irdoc = temp;
		}
		OmDocument mydoc(irdoc);
		add_item = mdecider->operator()(mydoc);
	    }

	    // Item has high enough weight to go in MSet: do collapse if wanted
	    if(add_item && do_collapse) {
		if (irdoc.get() == 0) {
		    RefCntPtr<LeafDocument> temp(database->open_document(did));
		    irdoc = temp;
		}
		new_item.collapse_key = irdoc.get()->get_key(collapse_key);
		add_item = perform_collapse(items, collapse_table, did,
					    new_item, min_item);
	    }

	    if(add_item) {
		items.push_back(new_item);

		// Keep a track of the greatest weight we've seen.
		if(wt > greatest_wt) greatest_wt = wt;

		// FIXME: find balance between larger size for more efficient
		// nth_element and smaller size for better minimum weight
		// optimisations
		if (items.size() == max_msize * 2) {
		    // find last element we care about
		    DEBUGLINE(MATCH, "finding nth");
		    std::nth_element(items.begin(), items.begin() + max_msize,
				items.end(), mcmp);
		    // erase elements which don't make the grade
		    items.erase(items.begin() + max_msize, items.end());
		    min_item = items.back();
		    DEBUGLINE(MATCH, "mset size = " << items.size());
		}
	    }
	}
    }

    // done with posting list tree
    delete query;
    delete extra_weight;

    if (items.size() > max_msize) {
	// find last element we care about
	DEBUGLINE(MATCH, "finding nth");
	std::nth_element(items.begin(), items.begin() + max_msize, items.end(), mcmp);
	// erase elements which don't make the grade
	items.erase(items.begin() + max_msize, items.end());
    }

    DEBUGLINE(MATCH, "sorting");

    if(first > 0) {
	// Remove unwanted leading entries
	if(items.size() <= first) {
	    items.clear();
	} else {
	    DEBUGLINE(MATCH, "finding " << first << "th");
	    std::nth_element(items.begin(), items.begin() + first, items.end(), mcmp);
	    // erase the leading ``first'' elements
	    items.erase(items.begin(), items.begin() + first);
	}
    }

    // Need a stable sort, but this is provided by comparison operator
    std::sort(items.begin(), items.end(), mcmp);

    DEBUGLINE(MATCH, "msize = " << items.size() << ", mbound = " << mbound);
    if (items.size()) {
	DEBUGLINE(MATCH, "max weight in mset = " << items[0].wt <<
		  ", min weight in mset = " << items[items.size() - 1].wt);
    }

    // Get initial max weight and the maximum extra weight contribution
    mset = OmMSet(first, mbound, max_weight, greatest_wt, items, termfreqandwts);

    return true;
}
