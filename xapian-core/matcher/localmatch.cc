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
#include "mergepostlist.h"
#include "extraweightpostlist.h"

#include "document.h"
#include "rset.h"
#include "omqueryinternal.h"
#include "omdocumentparams.h"

#include "../api/omdatabaseinternal.h"

#include "match.h"
#include "stats.h"
#include "irweight.h"

#include <algorithm>
#include "autoptr.h"
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
	 *  than b; with the proviso that if the estimated termfrequency
	 *  of the a or b is 0, the termweight is considered to be 0.
	 */
	bool operator()(const PostList *a, const PostList *b) {
	    om_weight amax, bmax;
	    if (a->get_termfreq() == 0)
		amax = 0;
	    else
		amax = a->get_maxweight();

	    if (b->get_termfreq() == 0)
		bmax = 0;
	    else
		bmax = b->get_maxweight();

	    DEBUGLINE(MATCH, "termweights are: " << amax << " and " << bmax);
	    return amax > bmax;
	}
};

// FIXME: postlists passed by reference and needs to be kept "nice"
// for NearPostList - so if there's a zero freq term it needs to be
// empty, else it needs to have all the postlists in (though the order
// can be different) - this is ultra-icky
PostList *
LocalSubMatch::build_and_tree(std::vector<PostList *> &postlists,
			      MultiMatch *matcher)
{
    // Build nice tree for AND-ed terms
    // SORT list into ascending freq order
    // AND last two elements, then AND with each subsequent element

// FIXME: if we throw away zero frequency postlists at this point,
// max_weight will come out lower...
#if 0
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
#endif

    if (postlists.empty()) {
	EmptyPostList *pl = new EmptyPostList();
	return pl;
    }

    std::stable_sort(postlists.begin(), postlists.end(), PLPCmpLt());

    int j = postlists.size() - 1;
    PostList *pl = postlists[j];
    while (j > 0) {
	j--;
	// NB right is always <= left - we use this to optimise.
	pl = new AndPostList(postlists[j], pl, matcher);
    }
    return pl;
}

PostList *
LocalSubMatch::build_or_tree(std::vector<PostList *> &postlists,
			     MultiMatch *matcher)
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
// FIXME: if we throw away zero frequency postlists at this point,
// max_weight will come out lower...
#if 0
	// for an OR, we can just ignore zero freq terms
	if ((*i)->get_termfreq() == 0) {
	    delete *i;
	} else {
	    pq.push(*i);
	}
#else
	pq.push(*i);
#endif
    }
    postlists.clear();

    // If none of the postlists had any entries, return an EmptyPostList.
    if (pq.empty()) {
	EmptyPostList *pl = new EmptyPostList();
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
	pl = new OrPostList(pq.top(), pl, matcher);
	pq.pop();
	pq.push(pl);
    }
}

// Make a postlist from a vector of query objects.
// Operation must be either AND or OR.
// Optimise query by building tree carefully.
PostList *
LocalSubMatch::postlist_from_queries(OmQuery::op op,
				     const std::vector<OmQueryInternal *> &queries,
				     om_termcount window,
				     MultiMatch *matcher)
{
    Assert(op == OmQuery::OP_OR || op == OmQuery::OP_AND ||
	   op == OmQuery::OP_NEAR || op == OmQuery::OP_PHRASE);
    Assert(queries.size() >= 2);

    // Open a postlist for each query, and store these postlists in a vector.
    std::vector<PostList *> postlists;
    postlists.reserve(queries.size());

    std::vector<OmQueryInternal *>::const_iterator q;
    for (q = queries.begin(); q != queries.end(); q++) {
	postlists.push_back(postlist_from_query(*q, matcher));
	DEBUGLINE(MATCH, "Made postlist: get_termfreq() = " <<
		  postlists.back()->get_termfreq());
    }

    // Build tree
    switch (op) {
	case OmQuery::OP_AND:
	    return build_and_tree(postlists, matcher);

	case OmQuery::OP_NEAR:
	{
	    PostList *res = build_and_tree(postlists, matcher);
	    // FIXME: handle EmptyPostList return specially?
	    return new NearPostList(res, window, postlists);
	}

	case OmQuery::OP_PHRASE:
	{
	    // build_and_tree reorders postlists, but the order is
	    // important for phrase, so we need to keep a copy
	    // FIXME: there must be a cleaner way for this to work...
	    std::vector<PostList *> postlists_orig = postlists;
	    PostList *res = build_and_tree(postlists, matcher);
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

	     return build_or_tree(postlists, matcher);

	default:
	    Assert(0);
    }
    Assert(0);
    return NULL;
}

// Make a postlist from a query object - this is called recursively down
// the query tree.
PostList *
LocalSubMatch::postlist_from_query(const OmQueryInternal *query,
				   MultiMatch *matcher)
{
    // This should never fail, because isdefined should only be false
    // at the root of a query tree
    Assert(query->isdefined);

    switch (query->op) {
	case OmQuery::OP_LEAF: {
	    // Make a postlist for a single term
	    Assert(query->subqs.size() == 0);
	    OmMSet::TermFreqAndWeight info;
	
	    // FIXME: pass the weight type and the info needed to create it to the
	    // postlist instead
	    IRWeight * wt = mk_weight(query);
	    info.termweight = wt->get_maxpart();

	    // MULTI - this statssource should be the combined one...
	    info.termfreq = statssource->get_total_termfreq(query->tname);

	    DEBUGLINE(MATCH, " weight = " << info.termweight <<
		      ", frequency = " << info.termfreq);

	    term_info.insert(std::make_pair(query->tname, info));

	    // MULTI
	    LeafPostList * pl = db->open_post_list(query->tname);
	    pl->set_termweight(wt);
	    return pl;
	}
	case OmQuery::OP_AND:
	case OmQuery::OP_OR:
	case OmQuery::OP_PHRASE:
	case OmQuery::OP_NEAR:
	    // Build a tree of postlists for AND, OR, PHRASE, or NEAR
	    return postlist_from_queries(query->op, query->subqs,
					 query->window, matcher);
	case OmQuery::OP_FILTER:
	    Assert(query->subqs.size() == 2);
	    return new FilterPostList(postlist_from_query(query->subqs[0], matcher),
				      postlist_from_query(query->subqs[1], matcher),
				      matcher);
	case OmQuery::OP_AND_NOT:
	    Assert(query->subqs.size() == 2);
	    return new AndNotPostList(postlist_from_query(query->subqs[0], matcher),
				      postlist_from_query(query->subqs[1], matcher),
				      matcher);
	case OmQuery::OP_AND_MAYBE:
	    Assert(query->subqs.size() == 2);
	    return new AndMaybePostList(postlist_from_query(query->subqs[0], matcher),
					postlist_from_query(query->subqs[1], matcher),
					matcher);
	case OmQuery::OP_XOR:
	    Assert(query->subqs.size() == 2);
	    return new XorPostList(postlist_from_query(query->subqs[0], matcher),
				   postlist_from_query(query->subqs[1], matcher),
				   matcher);
    }
    Assert(false);
    return NULL;
}

////////////////////////
// Building the query //
////////////////////////

bool
LocalSubMatch::prepare_match(bool nowait)
{
    if (!is_prepared) {
	DEBUGLINE(MATCH, "LocalSubMatch::prepare_match() - Gathering my statistics");
	om_termname_list terms = users_query.get_terms();

	om_termname_list::const_iterator tname;
	for (tname = terms.begin(); tname != terms.end(); tname++) {
	    // MULTI
	    register_term(*tname);
	    if (rset.get() != 0) rset->will_want_reltermfreq(*tname);
	}

	if (rset.get() != 0) {
	    rset->calculate_stats();
	    rset->give_stats_to_statssource(statssource);
	}
	is_prepared = true;
    }
    return true;
}

PostList *
LocalSubMatch::get_postlist(om_doccount maxitems, MultiMatch *matcher)
{
    PostList *pl = postlist_from_query(&users_query, matcher);
    IRWeight *wt = mk_weight();
    // don't bother with an ExtraWeightPostList if there's no extra weight
    // contribution.
    if (wt->get_maxextra() == 0) {
	delete wt;
	return pl;
    }
    return new ExtraWeightPostList(pl, wt);
}



IRWeight *
LocalSubMatch::mk_weight(const OmQueryInternal *query_)
{
    om_termname tname = "";
    om_termcount wqf = 1;
    if (query_) {
	tname = query_->tname;
	wqf = query_->wqf;
    }
    IRWeight * wt = IRWeight::create(weighting_scheme, mopts);
    wt->set_stats(statssource, querysize, wqf, tname);
#ifdef MUS_DEBUG_PARANOID
    if (!tname.empty()) {
	AutoPtr<IRWeight> extra_weight(mk_weight());
	// Check that max_extra weight is really right
	AssertEqDouble(wt->get_maxextra(), extra_weight->get_maxextra());
    }
#endif /* MUS_DEBUG_PARANOID */
    return wt;
}
