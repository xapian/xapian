/* multimatch.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005 Olly Betts
 * Copyright 2003 Orange PCS Ltd
 * Copyright 2003 Sam Liddicott
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

#include <config.h>
#include "multimatch.h"
#include "match.h"
#include "localmatch.h"
#include "emptymatch.h"
#include "omdebug.h"
#include "omenquireinternal.h"

#include "emptypostlist.h"
#include "branchpostlist.h"
#include "leafpostlist.h"
#include "mergepostlist.h"
#include "biaspostlist.h"

#ifdef XAPIAN_BUILD_BACKEND_REMOTE
#include "networkmatch.h"
#endif /* XAPIAN_BUILD_BACKEND_REMOTE */

#include "document.h"
#include "omqueryinternal.h"

#include "match.h"
#include "stats.h"

#include <xapian/errorhandler.h>

#include <algorithm>
#include <queue>
#include <vector>
#include <map>
#include <set>

using namespace std;

// Comparison functions to determine the order of elements in the MSet
// Return true if a should be listed before b
typedef bool (* mset_cmp)(const Xapian::Internal::MSetItem &, const Xapian::Internal::MSetItem &);

/// Compare a Xapian::Internal::MSetItem, using a custom function
class OmMSetCmp {
    private:
	bool (* fn)(const Xapian::Internal::MSetItem &a, const Xapian::Internal::MSetItem &b);
    public:
	OmMSetCmp(bool (* fn_)(const Xapian::Internal::MSetItem &a, const Xapian::Internal::MSetItem &b))
		: fn(fn_) {}
	bool operator()(const Xapian::Internal::MSetItem &a, const Xapian::Internal::MSetItem &b) const {
	    return fn(a, b);
	}
};

// Comparison which sorts equally weighted MSetItems in docid order
static bool
msetcmp_forward(const Xapian::Internal::MSetItem &a, const Xapian::Internal::MSetItem &b)
{
    if (a.wt > b.wt) return true;
    if (a.wt < b.wt) return false;
    // Two special cases to make min_item compares work when did == 0
    // NB note the ordering: if a.did == b.did == 0, we should return false
    if (a.did == 0) return false;
    if (b.did == 0) return true;
    return (a.did < b.did);
}

// Comparison which sorts equally weighted MSetItems in reverse docid order
static bool
msetcmp_reverse(const Xapian::Internal::MSetItem &a, const Xapian::Internal::MSetItem &b)
{
    if (a.wt > b.wt) return true;
    if (a.wt < b.wt) return false;
    return (a.did > b.did);
}

// Comparison which sorts by a value - used when sort_bands == 1.
// If sort keys compare equal, return documents in docid order.
static bool
msetcmp_sort_forward(const Xapian::Internal::MSetItem &a,
		     const Xapian::Internal::MSetItem &b)
{
    // "bigger is better"
    if (a.sort_key > b.sort_key) return true;
    if (a.sort_key < b.sort_key) return false;
    // two special cases to make min_item compares work when did == 0
    if (a.did == 0) return false;
    if (b.did == 0) return true; 
    return (a.did < b.did);
}

// Comparison which sorts by a value - used when sort_bands == 1.
// If sort keys compare equal, return documents in weight, then docid order.
static bool
msetcmp_sort_forward_relevance(const Xapian::Internal::MSetItem &a,
			       const Xapian::Internal::MSetItem &b)
{
    // "bigger is better"
    if (a.sort_key > b.sort_key) return true;
    if (a.sort_key < b.sort_key) return false;
    if (a.wt > b.wt) return true;
    if (a.wt < b.wt) return false;
    // two special cases to make min_item compares work when did == 0
    if (a.did == 0) return false;
    if (b.did == 0) return true; 
    return (a.did < b.did);
}

// Comparison which sorts by a value - used when sort_bands == 1.
// If sort keys compare equal, return documents in reverse docid order.
static bool
msetcmp_sort_reverse(const Xapian::Internal::MSetItem &a,
		     const Xapian::Internal::MSetItem &b)
{
    // "bigger is better"
    if (a.sort_key > b.sort_key) return true;
    if (a.sort_key < b.sort_key) return false;
    return (a.did > b.did);
}

// Comparison which sorts by a value - used when sort_bands == 1.
// If sort keys compare equal, return documents in reverse docid order.
static bool
msetcmp_sort_reverse_relevance(const Xapian::Internal::MSetItem &a,
			       const Xapian::Internal::MSetItem &b)
{
    // "bigger is better"
    if (a.sort_key > b.sort_key) return true;
    if (a.sort_key < b.sort_key) return false;
    if (a.wt > b.wt) return true;
    if (a.wt < b.wt) return false;
    return (a.did > b.did);
}

class MSetSortCmp {
    private:
	Xapian::Database db;
	double factor;
	Xapian::valueno sort_key;
	bool forward;
	int bands;
    public:
	MSetSortCmp(const Xapian::Database &db_, int bands_, double percent_scale,
		    Xapian::valueno sort_key_, bool forward_)
	    : db(db_), factor(percent_scale * bands_ / 100.0),
	      sort_key(sort_key_), forward(forward_), bands(bands_) {
	}
	bool operator()(const Xapian::Internal::MSetItem &a, const Xapian::Internal::MSetItem &b) const {
	    int band_a = int(ceil(a.wt * factor));
	    int band_b = int(ceil(b.wt * factor));
	    if (band_a > bands) band_a = bands;
	    if (band_b > bands) band_b = bands;
	    
	    if (band_a != band_b) return band_a > band_b;
	    if (sort_key != Xapian::valueno(-1)) {
		if (a.sort_key.empty()) {
		    Xapian::Document doc = db.get_document(a.did);
		    a.sort_key = doc.get_value(sort_key);
		}
		if (b.sort_key.empty()) {
		    Xapian::Document doc = db.get_document(b.did);
		    b.sort_key = doc.get_value(sort_key);
		}
		// "bigger is better"
		if (a.sort_key > b.sort_key) return true;
		if (a.sort_key < b.sort_key) return false;
	    }
	    if (forward) return a.did < b.did;
	    return a.did > b.did;
	}
};

////////////////////////////////////
// Initialisation and cleaning up //
////////////////////////////////////
MultiMatch::MultiMatch(const Xapian::Database &db_, const Xapian::Query::Internal * query_,
		       Xapian::termcount qlen,
		       const Xapian::RSet & omrset, Xapian::valueno collapse_key_,
		       int percent_cutoff_, Xapian::weight weight_cutoff_,
		       bool sort_forward_, Xapian::valueno sort_key_,
		       int sort_bands_, bool sort_by_relevance_,
		       time_t bias_halflife_,
		       Xapian::weight bias_weight_, Xapian::ErrorHandler * errorhandler_,
		       StatsGatherer * gatherer_,
		       const Xapian::Weight * weight_)
	: gatherer(gatherer_), db(db_), query(query_),
	  collapse_key(collapse_key_), percent_cutoff(percent_cutoff_),
	  weight_cutoff(weight_cutoff_), sort_forward(sort_forward_),
	  sort_key(sort_key_), sort_bands(sort_bands_),
	  sort_by_relevance(sort_by_relevance_),
	  bias_halflife(bias_halflife_), bias_weight(bias_weight_),
	  errorhandler(errorhandler_), weight(weight_)
{
    DEBUGCALL(MATCH, void, "MultiMatch", db_ << ", " << query_ << ", " <<
	      qlen << ", " <<
	      omrset << ", " << collapse_key_ << ", " << percent_cutoff_ <<
	      ", " << weight_cutoff_ << ", " << sort_forward << ", " <<
	      sort_key_ << ", " << sort_bands_ << ", " << sort_by_relevance_ <<
	      ", " << bias_halflife_ << ", " << bias_weight_ << ", " <<
	      errorhandler_ << ", [gatherer_], [weight_]");
    if (!query) return;

    query->validate_query();

    vector<Xapian::RSet> subrsets;
    if (db.internal.size() == 1) {
	// Shortcut the common case.
	subrsets.push_back(omrset);
    } else {
	Xapian::doccount number_of_leaves = db.internal.size();
	// Can't just use resize - that creates N copies of the same RSet!
	for (int i = 0; i < number_of_leaves; ++i) {
	    subrsets.push_back(Xapian::RSet());
	}

	const set<Xapian::docid> & items = omrset.internal->items;
	set<Xapian::docid>::const_iterator i; 
	for (i = items.begin(); i != items.end(); ++i) {
	    Xapian::doccount local_docid = (*i - 1) / number_of_leaves + 1;
	    Xapian::doccount subdatabase = (*i - 1) % number_of_leaves;
	    subrsets[subdatabase].add_document(local_docid);
	}
    }
    
    vector<Xapian::RSet>::const_iterator subrset = subrsets.begin();

    vector<Xapian::Internal::RefCntPtr<Xapian::Database::Internal> >::const_iterator i;
    for (i = db.internal.begin(); i != db.internal.end(); ++i) {
	Assert(subrset != subrsets.end());
	Xapian::Database::Internal *subdb = (*i).get();
	Assert(subdb);
	Xapian::Internal::RefCntPtr<SubMatch> smatch;
	try {
	    // There is currently only one special case, for network databases.
#ifdef XAPIAN_BUILD_BACKEND_REMOTE
	    const NetworkDatabase *netdb = subdb->as_networkdatabase();
	    if (netdb) {
		if (sort_key != Xapian::valueno(-1) || sort_bands) {
		    throw Xapian::UnimplementedError("sort_key and sort_bands not supported with remote backend");
		}
		if (bias_halflife) {
		    throw Xapian::UnimplementedError("bias_halflife and bias_weight not supported with remote backend");
		}
		smatch = Xapian::Internal::RefCntPtr<SubMatch>(
			new RemoteSubMatch(netdb, query, qlen,
			    *subrset, collapse_key,
			    sort_forward, percent_cutoff, weight_cutoff,
			    gatherer.get(), weight));
	    } else {
#endif /* XAPIAN_BUILD_BACKEND_REMOTE */
		smatch = Xapian::Internal::RefCntPtr<SubMatch>(new LocalSubMatch(subdb, query, qlen, *subrset, gatherer.get(), weight));
#ifdef XAPIAN_BUILD_BACKEND_REMOTE
	    }
#endif /* XAPIAN_BUILD_BACKEND_REMOTE */
	} catch (Xapian::Error & e) {
	    if (errorhandler) {
		DEBUGLINE(EXCEPTION, "Calling error handler for creation of a SubMatch from a database and query.");
		(*errorhandler)(e);
		// Continue match without this sub-postlist.
		smatch = Xapian::Internal::RefCntPtr<SubMatch>(new EmptySubMatch());
	    } else {
		throw;
	    }
	}
	leaves.push_back(smatch);
	++subrset;
    }
    Assert(subrset == subrsets.end());

    gatherer->set_global_stats(omrset.size());
    prepare_matchers();
}

MultiMatch::~MultiMatch()
{
    DEBUGCALL(MATCH, void, "~MultiMatch", "");
}

void
MultiMatch::prepare_matchers()
{
    DEBUGCALL(MATCH, void, "MultiMatch::prepare_matchers", "");
    Assert(query);
    bool prepared;
    bool nowait = true;
    do {
	prepared = true;
	vector<Xapian::Internal::RefCntPtr<SubMatch> >::iterator leaf;
	for (leaf = leaves.begin(); leaf != leaves.end(); ++leaf) {
	    try {
		if (!(*leaf)->prepare_match(nowait)) prepared = false;
	    } catch (Xapian::Error & e) {
		if (errorhandler) {
		    DEBUGLINE(EXCEPTION, "Calling error handler for prepare_match() on a SubMatch.");
		    (*errorhandler)(e);
		    // Continue match without this sub-match.
		    *leaf = Xapian::Internal::RefCntPtr<SubMatch>(new EmptySubMatch());
		    prepared = false;
		} else {
		    throw;
		}
	    }
	}
	// Use blocking IO on subsequent passes, so that we don't go into
	// a tight loop.
	nowait = false;
    } while (!prepared);
}

string
MultiMatch::get_collapse_key(PostList *pl, Xapian::docid did,
			     Xapian::valueno keyno, Xapian::Internal::RefCntPtr<Xapian::Document::Internal> &doc)
{		      
    DEBUGCALL(MATCH, string, "MultiMatch::get_collapse_key", pl << ", " << did << ", " << keyno << ", [doc]");
    const string *key = pl->get_collapse_key();
    if (key) RETURN(*key);
    if (doc.get() == 0) {
	unsigned int multiplier = db.internal.size();
	Assert(multiplier != 0);
	Xapian::doccount n = (did - 1) % multiplier; // which actual database
	Xapian::docid m = (did - 1) / multiplier + 1; // real docid in that database

   	Xapian::Internal::RefCntPtr<Xapian::Document::Internal> temp(db.internal[n]->open_document(m, true));
	doc = temp;
    }
    RETURN(doc->get_value(keyno));
}

Xapian::weight
MultiMatch::getorrecalc_maxweight(PostList *pl)
{
    DEBUGCALL(MATCH, Xapian::weight, "MultiMatch::getorrecalc_maxweight", pl);
    Xapian::weight wt;
    if (recalculate_w_max) {
	DEBUGLINE(MATCH, "recalculating max weight");
	wt = pl->recalc_maxweight();
	recalculate_w_max = false;
    } else {
	wt = pl->get_maxweight();
	AssertParanoid(fabs(wt - pl->recalc_maxweight()) < 1e-9);
    }
    DEBUGLINE(MATCH, "max possible doc weight = " << wt);
    RETURN(wt);
}

void
MultiMatch::get_mset(Xapian::doccount first, Xapian::doccount maxitems,
		     Xapian::doccount check_at_least,
		     Xapian::MSet & mset, const Xapian::MatchDecider *mdecider)
{
    DEBUGCALL(MATCH, void, "MultiMatch::get_mset", first << ", " << maxitems
	      << ", " << check_at_least << ", ...");
    if (check_at_least < maxitems) check_at_least = maxitems;

    if (!query) {
	mset = Xapian::MSet(); // FIXME: mset.get_firstitem() will return 0 not first
	return;
    }

    Assert(!leaves.empty());

    // Start matchers
    {
	vector<Xapian::Internal::RefCntPtr<SubMatch> >::iterator leaf;
	for (leaf = leaves.begin(); leaf != leaves.end(); ++leaf) {
	    try {
		// FIXME: look if we can push the "check_at_least" stuff
		// into the remote match handling too.
		(*leaf)->start_match(first + check_at_least);
	    } catch (Xapian::Error & e) {
		if (errorhandler) {
		    DEBUGLINE(EXCEPTION, "Calling error handler for "
			      "start_match() on a SubMatch.");
		    (*errorhandler)(e);
		    // Continue match without this sub-match.
		    *leaf = Xapian::Internal::RefCntPtr<SubMatch>(new EmptySubMatch());
		} else {
		    throw;
		}
	    }
	}
    }

    // Get postlists
    vector<PostList *> postlists;
    vector<Xapian::Internal::RefCntPtr<SubMatch> >::iterator i;
    for (i = leaves.begin(); i != leaves.end(); ++i) {
	// FIXME: errorhandler here? (perhaps not needed if this simply makes a pending postlist)
	postlists.push_back((*i)->get_postlist(first + check_at_least, this));
    }

    // Get term info
    map<string, Xapian::MSet::Internal::TermFreqAndWeight> termfreqandwts;
    {
	vector<Xapian::Internal::RefCntPtr<SubMatch> >::iterator leaf;
	vector<PostList * >::iterator pl_iter;
	Assert(leaves.size() == postlists.size());
	for (leaf = leaves.begin(), pl_iter = postlists.begin();
	     leaf != leaves.end();
	     ++leaf, ++pl_iter) {
	    try {
		termfreqandwts = (*leaf)->get_term_info();
		break;
	    } catch (Xapian::Error & e) {
		if (e.get_type() == "Xapian::InternalError" &&
		    e.get_msg().substr(0, 13) == "EmptySubMatch") {
		    DEBUGLINE(MATCH, "leaf is an EmptySubMatch, trying next");
		} else {
		    if (!errorhandler) throw;
		    DEBUGLINE(EXCEPTION, "Calling error handler for "
			      "get_term_info() on a SubMatch.");
		    (*errorhandler)(e);
		    // Continue match without this sub-match.
		    *leaf = Xapian::Internal::RefCntPtr<SubMatch>(new EmptySubMatch());
		    delete *pl_iter;
		    *pl_iter = new EmptyPostList;
		}
	    }
	}
    }
    
    // Get a single combined postlist
    PostList *pl;
    Assert(postlists.size() != 0);
    if (postlists.size() == 1) {
	pl = postlists.front();
    } else {
	pl = new MergePostList(postlists, this, errorhandler);
    }

    // FIXME: a temporary bodge to allow this to be used - I'll write
    // a proper API later, promise - Olly
    if (bias_halflife) {
	pl = new BiasPostList(pl, db,
	       	new OmBiasFunctor(db, bias_weight, bias_halflife), this);
    }

    DEBUGLINE(MATCH, "pl = (" << pl->get_description() << ")");

    // Empty result set
    Xapian::doccount docs_matched = 0;
    Xapian::weight greatest_wt = 0;
    vector<Xapian::Internal::MSetItem> items;

    // maximum weight a document could possibly have
    const Xapian::weight max_weight = pl->recalc_maxweight();

    DEBUGLINE(MATCH, "pl = (" << pl->get_description() << ")");
    recalculate_w_max = false;

    Xapian::doccount matches_upper_bound = pl->get_termfreq_max();
    Xapian::doccount matches_lower_bound = pl->get_termfreq_min();
    Xapian::doccount matches_estimated   = pl->get_termfreq_est();
    Xapian::doccount duplicates_found = 0;
    Xapian::doccount documents_considered = 0;

    // Check if any results have been asked for (might just be wanting
    // maxweight).
    if (check_at_least == 0) {
	delete pl;
	if (mdecider != NULL) {
	    // Lower bound must be set to 0 as the match decider could discard
	    // all hits.
	    matches_lower_bound = 0;
	} else if (collapse_key != Xapian::valueno(-1)) {
	    // Lower bound must be set to no more than 1, since it's possible
	    // that all hits will be collapsed to a single hit.
	    if (matches_lower_bound > 1) matches_lower_bound = 1;
	}

	mset = Xapian::MSet(new Xapian::MSet::Internal(
					   first,
					   matches_upper_bound,
					   matches_lower_bound,
					   matches_estimated,
					   max_weight, greatest_wt, items,
					   termfreqandwts,
					   0));
	return;
    }

    // Set max number of results that we want - this is used to decide
    // when to throw away unwanted items.
    Xapian::doccount max_msize = first + check_at_least;
    items.reserve(max_msize + 1);

    // Set the minimum item, used to compare against to see if an item
    // should be considered for the mset.
    Xapian::Internal::MSetItem min_item(weight_cutoff, 0);

    Xapian::weight percent_factor = percent_cutoff / 100.0;
 
    // Table of keys which have been seen already, for collapsing.
    map<string, pair<Xapian::Internal::MSetItem,Xapian::weight> > collapse_tab;

    /// Comparison functor for sorting MSet
    // The sort_bands == 1 case is special - then we only need to compare
    // weights when the sortkeys are identical.
    OmMSetCmp mcmp(sort_bands != 1 ?
	(sort_forward ? msetcmp_forward : msetcmp_reverse) :
	(sort_forward ?
	    (sort_by_relevance ? msetcmp_sort_forward_relevance : msetcmp_sort_forward) :
	    (sort_by_relevance ? msetcmp_sort_reverse_relevance : msetcmp_sort_reverse)));

    // Perform query

    // We form the mset in two stages.  In the first we fill up our working
    // mset.  Adding a new document does not remove another.
    //
    // In the second, we consider documents which rank higher than the current
    // lowest ranking document in the mset.  Each document added expels the
    // current lowest ranking document.
    //
    // If a percentage cutoff is in effect, it can cause the matcher to return
    // from the second stage from the first.

    // Is the mset a valid heap?
    bool is_heap = false; 

    while (true) {
	if (recalculate_w_max) {
	    if (min_item.wt > 0.0 && getorrecalc_maxweight(pl) < min_item.wt) {
		DEBUGLINE(MATCH, "*** TERMINATING EARLY (1)");
		break;
	    }
	}

	if (next_handling_prune(pl, min_item.wt, this)) {
	    DEBUGLINE(MATCH, "*** REPLACING ROOT");

	    // no need for a full recalc (unless we've got to do one because
	    // of a prune elsewhere) - we're just switching to a subtree
	    if (getorrecalc_maxweight(pl) < min_item.wt) {
		DEBUGLINE(MATCH, "*** TERMINATING EARLY (2)");
		break;
	    }
	}

	if (pl->at_end()) {
	    DEBUGLINE(MATCH, "Reached end of potential matches");
	    break;
	}

	Xapian::docid did = pl->get_docid();
	Xapian::weight wt = 0.0;
	// Only calculate the weight if we need it for mcmp - otherwise
	// we calculate it below only if we keep the item
	if (min_item.wt > 0.0) wt = pl->get_weight();

	DEBUGLINE(MATCH, "Candidate document id " << did << " wt " << wt);
	Xapian::Internal::MSetItem new_item(wt, did);
	if (sort_key != Xapian::valueno(-1) && sort_bands == 1) {
	    Xapian::Document doc = db.get_document(new_item.did);
	    new_item.sort_key = doc.get_value(sort_key);
	}

	// test if item has high enough weight (or sort key if sort_bands == 1)
	// to get into proto-mset
	if (sort_bands == 1 || min_item.wt > 0.0)
	    if (!mcmp(new_item, min_item)) continue;

	Xapian::Internal::RefCntPtr<Xapian::Document::Internal> doc;

	// Use the decision functor if any.
	// FIXME: if results are from MSetPostList then we can omit this step
	if (mdecider != NULL) {
	    if (doc.get() == 0) {
		unsigned int multiplier = db.internal.size();
		Assert(multiplier != 0);
		Xapian::doccount n = (did - 1) % multiplier; // which actual database
		Xapian::docid m = (did - 1) / multiplier + 1; // real docid in that database

		Xapian::Internal::RefCntPtr<Xapian::Document::Internal> temp(db.internal[n]->open_document(m, true));
		doc = temp;
	    }
	    Xapian::Document mydoc(doc.get());
	    if (!mdecider->operator()(mydoc)) continue;
	}

	if (min_item.wt <= 0.0) {
	    // we didn't calculate the weight above, but now we will need it
	    wt = pl->get_weight();
	    new_item.wt = wt;
	}

	bool pushback = true;
	documents_considered++;

	// Perform collapsing on key if requested.
	if (collapse_key != Xapian::valueno(-1)) {
	    new_item.collapse_key = get_collapse_key(pl, did, collapse_key,
						     doc);

	    // Don't collapse on null key
	    if (!new_item.collapse_key.empty()) {
		map<string, pair<Xapian::Internal::MSetItem, Xapian::weight> >::iterator oldkey;
		oldkey = collapse_tab.find(new_item.collapse_key);
		if (oldkey == collapse_tab.end()) {
		    DEBUGLINE(MATCH, "collapsem: new key: " <<
			      new_item.collapse_key);
		    // Key not been seen before
		    collapse_tab.insert(make_pair(new_item.collapse_key,
					make_pair(new_item, Xapian::weight(0))));
		} else {
		    duplicates_found++;
		    const Xapian::Internal::MSetItem &old_item = oldkey->second.first;
		    // FIXME: what about sort_bands == 1 case here?
		    if (mcmp(old_item, new_item)) {
			DEBUGLINE(MATCH, "collapsem: better exists: " <<
				  new_item.collapse_key);
			// There's already a better match with this key
			++oldkey->second.first.collapse_count;
			// But maybe the weight is worth noting
			if (new_item.wt > oldkey->second.second) {
			    oldkey->second.second = new_item.wt;
			}
			continue;
		    }
		    // Make a note of the updated collapse count in the
		    // replacement item
		    new_item.collapse_count = old_item.collapse_count + 1;

		    // This is best match with this key so far:
		    // remove the old one from the MSet
		    if ((sort_bands != 1 && min_item.wt <= 0.0) ||
			mcmp(old_item, min_item)) {
			// Old one hasn't fallen out of MSet yet
			// Scan through (unsorted) MSet looking for entry
			// FIXME: more efficient way than just scanning?
			Xapian::docid olddid = old_item.did;
			DEBUGLINE(MATCH, "collapsem: removing " << olddid <<
				  ": " << new_item.collapse_key);
			vector<Xapian::Internal::MSetItem>::iterator i;
			for (i = items.begin(); i->did != olddid; ++i) {
			    Assert(i != items.end());
			}
			*i = new_item;
			pushback = false;
			// We can replace an arbitrary element in O(log N)
			// but have to do it by hand (in this case the new
			// elt is bigger, so we just swap down the tree)
			// FIXME: implement this, and clean up is_heap
			// handling
			is_heap = false;
		    }
		    // Keep the old weight as it is now second best so far
		    oldkey->second = make_pair(new_item,
					       oldkey->second.first.wt);
		}
	    }
	}

	// OK, actually add the item to the mset.
	if (pushback) {
	    ++docs_matched;
	    if (items.size() >= max_msize) {
		if (sort_bands > 1) {
		    if (!is_heap) {
			is_heap = true;
			make_heap<vector<Xapian::Internal::MSetItem>::iterator,
				  OmMSetCmp>(items.begin(), items.end(), mcmp);
		    }
		    Xapian::weight tmp = min_item.wt;
		    min_item = items.front();
		    min_item.wt = tmp;
#if 0
		    // FIXME: This optimisation gives incorrect results.
		    // Disable for now, but check that the comparisons
		    // are the correct way round.  We really should rework
		    // to share code with MSetSortCmp anyway...
		    if (new_item.sort_key.empty()) {
			Xapian::Document doc = db.get_document(new_item.did);
			new_item.sort_key = doc.get_value(sort_key);
		    }
		    if (min_item.wt > new_item.wt) {
			if (min_item.sort_key >= new_item.sort_key)
			    pushback = false;
		    } else {
			if (min_item.sort_key <= new_item.sort_key) {
			    pop_heap<vector<Xapian::Internal::MSetItem>::iterator,
				     OmMSetCmp>(items.begin(), items.end(),
						mcmp);
			    items.pop_back();
			}
		    }
#endif
		    if (pushback) {
			items.push_back(new_item);
			push_heap<vector<Xapian::Internal::MSetItem>::iterator,
				  OmMSetCmp>(items.begin(), items.end(), mcmp);
		    }
		} else {
		    items.push_back(new_item);
		    if (!is_heap) {
			is_heap = true;
			make_heap<vector<Xapian::Internal::MSetItem>::iterator,
				  OmMSetCmp>(items.begin(), items.end(), mcmp);
		    } else {
			push_heap<vector<Xapian::Internal::MSetItem>::iterator,
				  OmMSetCmp>(items.begin(), items.end(), mcmp);
		    }
		    pop_heap<vector<Xapian::Internal::MSetItem>::iterator,
			     OmMSetCmp>(items.begin(), items.end(), mcmp);
		    items.pop_back(); 
		    if (sort_bands == 1) {
			Xapian::weight tmp = min_item.wt;
			min_item = items.front();
			min_item.wt = tmp;
		    } else {
			min_item = items.front();
		    }
		    if (getorrecalc_maxweight(pl) < min_item.wt) {
			DEBUGLINE(MATCH, "*** TERMINATING EARLY (3)");
			break;
		    }
		}
	    } else {
		items.push_back(new_item);
		is_heap = false;
		if (!sort_bands && items.size() == max_msize) {
		    // We're done if this is a forward boolean match
		    // (bodgetastic, FIXME better if we can)
		    if (max_weight == 0 && sort_forward) break;
		}
	    }
	}

	// Keep a track of the greatest weight we've seen.
	if (wt > greatest_wt) {
	    greatest_wt = wt;
	    if (percent_cutoff) {
		Xapian::weight w = wt * percent_factor;
		if (w > min_item.wt) {
		    min_item.wt = w;
		    min_item.did = 0;
		    if (!is_heap) {
			is_heap = true;
			make_heap<vector<Xapian::Internal::MSetItem>::iterator,
				  OmMSetCmp>(items.begin(), items.end(),
						  mcmp);
		    }
		    while (!items.empty() && items.front().wt < min_item.wt) {
			pop_heap<vector<Xapian::Internal::MSetItem>::iterator,
				 OmMSetCmp>(items.begin(), items.end(), mcmp);
			Assert(items.back().wt < min_item.wt);
			items.pop_back();
		    }
#ifdef XAPIAN_DEBUG_PARANOID
		    vector<Xapian::Internal::MSetItem>::const_iterator i;
		    for (i = items.begin(); i != items.end(); ++i) {
			Assert(i->wt >= min_item.wt);
		    }
#endif
		}
	    }
	    if (sort_bands > 1) {
		if (greatest_wt >= getorrecalc_maxweight(pl)) {
		    if (!is_heap) {
			is_heap = true;
			make_heap<vector<Xapian::Internal::MSetItem>::iterator,
				  OmMSetCmp>(items.begin(), items.end(), mcmp);
		    }
		    // greatest_wt cannot now rise any further, so we now know
		    // exactly where the relevance bands are.
		    Xapian::weight w = greatest_wt / sort_bands *
			    floor(items.front().wt * sort_bands / greatest_wt);
		    if (w > min_item.wt) min_item.wt = w;
		}
	    }
	}
    }

    // done with posting list tree
    delete pl;

    double percent_scale = 0;
    if (!items.empty() && greatest_wt > 0) {
	// Find the document with the highest weight, then total up the
	// weights for the terms it contains
	vector<Xapian::Internal::MSetItem>::const_iterator best;
	best = min_element(items.begin(), items.end(), mcmp);

	Xapian::termcount matching_terms = 0;
	map<string,
	    Xapian::MSet::Internal::TermFreqAndWeight>::const_iterator i;

	Xapian::TermIterator docterms = db.termlist_begin(best->did);
	Xapian::TermIterator docterms_end = db.termlist_end(best->did);
	while (docterms != docterms_end) {
	    i = termfreqandwts.find(*docterms);
	    if (i != termfreqandwts.end()) {
		percent_scale += i->second.termweight;
		++matching_terms;
		if (matching_terms == termfreqandwts.size()) break;
	    }
	    ++docterms;
	}
	if (matching_terms < termfreqandwts.size()) {
	    // OK, work out weight corresponding to 100%
	    double denom = 0;
	    for (i = termfreqandwts.begin(); i != termfreqandwts.end(); ++i)
		denom += i->second.termweight;

	    DEBUGLINE(MATCH, "denom = " << denom << " percent_scale = " << percent_scale);
	    Assert(percent_scale <= denom);
	    denom *= greatest_wt;
	    Assert(denom > 0);
	    percent_scale /= denom;
	} else {
	    // If all the terms match, the 2 sums of weights cancel
	    percent_scale = 1.0 / greatest_wt;
	}
	Assert(percent_scale > 0);
	if (percent_cutoff) {
	    // FIXME: better to sort and binary chop maybe?  we
	    // could use the sort above to find "best" too.
	    // Or we could just use a linear scan here instead.

	    // trim the mset to the correct answer...
	    Xapian::weight min_wt = percent_factor / percent_scale;
	    if (!is_heap) {
		is_heap = true;
		make_heap<vector<Xapian::Internal::MSetItem>::iterator,
			  OmMSetCmp>(items.begin(), items.end(), mcmp);
	    }
	    while (!items.empty() && items.front().wt < min_wt) {
		pop_heap<vector<Xapian::Internal::MSetItem>::iterator,
			 OmMSetCmp>(items.begin(), items.end(), mcmp);
		Assert(items.back().wt < min_wt);
		items.pop_back();
	    }
#ifdef XAPIAN_DEBUG_PARANOID
	    vector<Xapian::Internal::MSetItem>::const_iterator i;
	    for (i = items.begin(); i != items.end(); ++i) {
		Assert(i->wt >= min_wt);
	    }
#endif
	}
	percent_scale *= 100.0;
    }

    if (sort_bands > 1) {
	sort(items.begin(), items.end(),
	     MSetSortCmp(db, sort_bands, percent_scale,
			 sort_key, sort_forward));
	if (items.size() > max_msize) {
	    items.erase(items.begin() + max_msize, items.end());
	}
    }

    if (items.size() < max_msize) {
	DEBUGLINE(MATCH, "items.size() = " << items.size() <<
		  ", max_msize = " << max_msize << ", setting bounds equal");
	Assert(percent_cutoff || docs_matched == items.size());
	matches_lower_bound = matches_upper_bound = matches_estimated
	    = items.size();
    } else {
	if (percent_cutoff) {
	    // another approach: Xapian::doccount new_est = items.size() * (1 - percent_factor) / (1 - min_item.wt / greatest_wt);
	    Xapian::doccount new_est;
	    new_est = Xapian::doccount((1 - percent_factor) * matches_estimated);
	    matches_estimated = max(size_t(new_est), items.size());
	    // and another: items.size() + (1 - greatest_wt * percent_factor / min_item.wt) * (matches_estimated - items.size());

	    // Very likely an underestimate, but we can't really do better without
	    // checking further matches...  Only possibility would be to track how
	    // many docs made the min weight test but didn't make the candidate set
	    // since the last greatest_wt change, which we could use if the top
	    // documents matched all the prob terms.
	    matches_lower_bound = items.size();
	    // matches_upper_bound could be reduced by the number of documents
	    // which fail the min weight test
	}

	DEBUGLINE(MATCH,
		"docs_matched = " << docs_matched << ", " <<
		"matches_lower_bound = " << matches_lower_bound << ", " <<
		"matches_estimated = " << matches_estimated << ", " <<
		"matches_upper_bound = " << matches_upper_bound);

	Assert(matches_estimated >= matches_lower_bound);
	Assert(matches_estimated <= matches_upper_bound);

	if (collapse_key != Xapian::valueno(-1)) {
	    // Lower bound must be set to no more than the number of collapse
	    // values we've got, since it's possible that all further hits
	    // will be collapsed to a single value.
	    matches_lower_bound = collapse_tab.size();

	    // The estimate for the number of hits can be modified by
	    // multiplying it by the rate at which we've been finding
	    // duplicates.
	    if (documents_considered > 0) {
		double collapse_rate =
			double(duplicates_found) / double(documents_considered);
		matches_estimated -=
			Xapian::doccount(double(matches_estimated) *
					   collapse_rate);
	    }

	    // We can safely reduce the upper bound by the number of
	    // duplicates we've seen.
	    matches_upper_bound -= duplicates_found;

	    matches_estimated = max(matches_estimated, matches_lower_bound);
	    matches_estimated = min(matches_estimated, matches_upper_bound);
	} else if (!percent_cutoff) {
	    Assert(docs_matched <= matches_upper_bound);
	    if (docs_matched > matches_lower_bound)
		matches_lower_bound = docs_matched;
	    if (docs_matched > matches_estimated)
		matches_estimated = docs_matched;
	}
    }

    DEBUGLINE(MATCH, items.size() << " items in potential mset");

    if (check_at_least > maxitems) {
	// Remove unwanted trailing entries
	if (maxitems == 0) {
	    items.clear();
	} else if (items.size() > first + maxitems) {
	    if (sort_bands <= 1)
		nth_element(items.begin(),
			    items.begin() + first + maxitems,
			    items.end(),
			    mcmp);
	    // Erase the unwanted trailing items.
	    items.erase(items.begin() + first + maxitems);
	}
    }
    if (first > 0) {
	// Remove unwanted leading entries
	if (items.size() <= first) {
	    items.clear();
	} else {
	    DEBUGLINE(MATCH, "finding " << first << "th");
	    if (sort_bands <= 1)
		nth_element(items.begin(), items.begin() + first, items.end(),
			    mcmp);
	    // Erase the leading ``first'' elements
	    items.erase(items.begin(), items.begin() + first);
	}
    }

    DEBUGLINE(MATCH, "msize = " << items.size());

    if (sort_bands <= 1 && !items.empty()) {
	DEBUGLINE(MATCH, "sorting");

	// Need a stable sort, but this is provided by comparison operator
	sort(items.begin(), items.end(), mcmp);

	DEBUGLINE(MATCH, "max weight in mset = " << items[0].wt <<
		  ", min weight in mset = " << items.back().wt);
    }

    Assert(matches_estimated >= matches_lower_bound);
    Assert(matches_estimated <= matches_upper_bound);

    // We may need to qualify any collapse_count to see if the highest weight
    // collapsed item would have qualified percent_cutoff 
    // We WILL need tp restore collapse_count to the mset by taking from
    // collapse_tab; this is what comes of copying around whole objects
    // instead of taking references, we find it hard to update collapse_count
    // of an item that has already been pushed-back as we don't know where it is
    // any more.  If we keep or find references we won't need to mess with
    // is_heap so much maybe?
    if (collapse_key != Xapian::valueno(-1) && /*percent_cutoff &&*/ !items.empty() &&
	!collapse_tab.empty()) {
	// Nicked this formula from above, but for some reason percent_scale
	// has since been multiplied by 100 so we take that into account
	Xapian::weight min_wt = percent_factor / (percent_scale / 100);
	vector<Xapian::Internal::MSetItem>::iterator i;
	for (i = items.begin(); i != items.end() && !collapse_tab.empty(); ++i) {
	    // Is this a collapsed hit?
	    if (/*i->collapse_count > 0 &&*/ !i->collapse_key.empty()) {
		map<string, pair<Xapian::Internal::MSetItem, Xapian::weight> >::iterator key;
		key = collapse_tab.find(i->collapse_key);
		// Because we collapse, each collapse key can only occur once
		// in the items, we remove from collapse_tab here as processed
		// so we can quit early.  Therefore each time we find an item
		// with a collapse_key the collapse_key must be in collapse_tab
		Assert(key != collapse_tab.end());
		// If weight of top collapsed item is not relevent enough
		// then collapse count is bogus in every way
		// FIXME: Should this be <=?
		if (key->second.second < min_wt) i->collapse_count = 0;
		else i->collapse_count = key->second.first.collapse_count;
		// When collapse_tab is finally empty we can finish this process
		// without examining any further hits
		collapse_tab.erase(key);
	    }
	}
    }
    
    mset = Xapian::MSet(new Xapian::MSet::Internal(
				       first,
				       matches_upper_bound,
				       matches_lower_bound,
				       matches_estimated,
				       max_weight, greatest_wt, items,
				       termfreqandwts,
				       percent_scale));
}

// This method is called by branch postlists when they rebalance
// in order to recalculate the weights in the tree
void
MultiMatch::recalc_maxweight()
{
    DEBUGCALL(MATCH, void, "MultiMatch::recalc_maxweight", "");
    recalculate_w_max = true;
}
