/* multimatch.cc
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include "multimatch.h"
#include "submatch.h"
#include "localmatch.h"
#include "emptysubmatch.h"
#include "omdebug.h"
#include "omenquireinternal.h"

#include "emptypostlist.h"
#include "branchpostlist.h"
#include "leafpostlist.h"
#include "mergepostlist.h"

#include "document.h"
#include "omqueryinternal.h"

#include "submatch.h"
#include "stats.h"

#include "msetcmp.h"

#include <xapian/errorhandler.h>
#include <xapian/version.h> // For XAPIAN_HAS_REMOTE_BACKEND

#ifdef XAPIAN_HAS_REMOTE_BACKEND
#include "remotesubmatch.h"
#include "remote-database.h"
#endif /* XAPIAN_HAS_REMOTE_BACKEND */

#include <algorithm>
#include <queue>
#include <vector>
#include <map>
#include <set>

using namespace std;

const Xapian::Enquire::Internal::sort_setting REL =
	Xapian::Enquire::Internal::REL;
const Xapian::Enquire::Internal::sort_setting REL_VAL =
	Xapian::Enquire::Internal::REL_VAL;
const Xapian::Enquire::Internal::sort_setting VAL =
	Xapian::Enquire::Internal::VAL;
#if 0 // VAL_REL isn't currently used which causes a warning with SGI CC.
const Xapian::Enquire::Internal::sort_setting VAL_REL =
	Xapian::Enquire::Internal::VAL_REL;
#endif

////////////////////////////////////
// Initialisation and cleaning up //
////////////////////////////////////
MultiMatch::MultiMatch(const Xapian::Database &db_,
		       const Xapian::Query::Internal * query_,
		       Xapian::termcount qlen,
		       const Xapian::RSet & omrset,
		       Xapian::valueno collapse_key_,
		       int percent_cutoff_, Xapian::weight weight_cutoff_,
		       Xapian::Enquire::docid_order order_,
		       Xapian::valueno sort_key_,
		       Xapian::Enquire::Internal::sort_setting sort_by_,
		       bool sort_value_forward_,
		       Xapian::ErrorHandler * errorhandler_,
		       StatsGatherer * gatherer_,
		       const Xapian::Weight * weight_)
	: gatherer(gatherer_), db(db_), query(query_),
	  collapse_key(collapse_key_), percent_cutoff(percent_cutoff_),
	  weight_cutoff(weight_cutoff_), order(order_),
	  sort_key(sort_key_), sort_by(sort_by_),
	  sort_value_forward(sort_value_forward_),
	  errorhandler(errorhandler_), weight(weight_),
	  is_remote(db.internal.size())
{
    DEBUGCALL(MATCH, void, "MultiMatch", db_ << ", " << query_ << ", " <<
	      qlen << ", " << omrset << ", " << collapse_key_ << ", " <<
	      percent_cutoff_ << ", " << weight_cutoff_ << ", " <<
	      int(order_) << ", " << sort_key_ << ", " <<
	      int(sort_by_) << ", " << sort_value_forward_ << ", " <<
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
	subrsets.reserve(number_of_leaves);
	for (size_t i = 0; i < number_of_leaves; ++i) {
	    subrsets.push_back(Xapian::RSet());
	}

	const set<Xapian::docid> & items = omrset.internal->get_items();
	set<Xapian::docid>::const_iterator j;
	for (j = items.begin(); j != items.end(); ++j) {
	    Xapian::doccount local_docid = (*j - 1) / number_of_leaves + 1;
	    Xapian::doccount subdatabase = (*j - 1) % number_of_leaves;
	    subrsets[subdatabase].add_document(local_docid);
	}
    }

    Assert(subrsets.size() == db.internal.size());
    for (size_t i = 0; i != db.internal.size(); ++i) {
	Xapian::Database::Internal *subdb = db.internal[i].get();
	Assert(subdb);
	Xapian::Internal::RefCntPtr<SubMatch> smatch;
	try {
	    // There is currently only one special case, for network databases.
#ifdef XAPIAN_HAS_REMOTE_BACKEND
	    RemoteDatabase *rem_db = subdb->as_remotedatabase();
	    if (rem_db) {
		is_remote[i] = true;
		rem_db->set_query(query, qlen, collapse_key, order, sort_key,
				  sort_by, sort_value_forward, percent_cutoff,
				  weight_cutoff, weight, subrsets[i]);
		bool decreasing_relevance =
		    (sort_by == REL || sort_by == REL_VAL);
		smatch = new RemoteSubMatch(rem_db, gatherer.get(),
					    decreasing_relevance);
	    } else {
#endif /* XAPIAN_HAS_REMOTE_BACKEND */
		smatch = new LocalSubMatch(subdb, query, qlen, subrsets[i], gatherer.get(), weight);
#ifdef XAPIAN_HAS_REMOTE_BACKEND
	    }
#endif /* XAPIAN_HAS_REMOTE_BACKEND */
	} catch (Xapian::Error & e) {
	    if (!errorhandler) throw;
	    DEBUGLINE(EXCEPTION, "Calling error handler for creation of a SubMatch from a database and query.");
	    (*errorhandler)(e);
	    // Continue match without this sub-postlist.
	    smatch = new EmptySubMatch;
	}
	leaves.push_back(smatch);
    }

    gatherer->set_global_stats(omrset.size());

    // We use a vector<bool> to track which SubMatches we're already prepared.
    vector<bool> prepared;
    prepared.resize(leaves.size(), false);
    size_t unprepared = leaves.size();
    bool nowait = true;
    while (unprepared) {
	for (size_t leaf = 0; leaf < leaves.size(); ++leaf) {
	    if (prepared[leaf]) continue;
	    try {
		if (leaves[leaf]->prepare_match(nowait)) {
		    prepared[leaf] = true;
		    --unprepared;
		}
	    } catch (Xapian::Error & e) {
		if (!errorhandler) throw;

		DEBUGLINE(EXCEPTION, "Calling error handler for prepare_match() on a SubMatch.");
		(*errorhandler)(e);
		// Continue match without this sub-match.
		leaves[leaf] = new EmptySubMatch();
		prepared[leaf] = true;
		--unprepared;
	    }
	}
	// Use blocking IO on subsequent passes, so that we don't go into
	// a tight loop.
	nowait = false;
    }
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
		(*leaf)->start_match(first + maxitems, first + check_at_least);
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

    // Get postlists and term info
    vector<PostList *> postlists;
    map<string, Xapian::MSet::Internal::TermFreqAndWeight> termfreqandwts;
    map<string, Xapian::MSet::Internal::TermFreqAndWeight> * termfreqandwts_ptr;
    termfreqandwts_ptr = &termfreqandwts;
    {
	vector<Xapian::Internal::RefCntPtr<SubMatch> >::iterator leaf;
	for (leaf = leaves.begin(); leaf != leaves.end(); ++leaf) {
	    PostList *pl;
	    try {
		pl = (*leaf)->get_postlist_and_term_info(this, termfreqandwts_ptr);
		if (termfreqandwts_ptr && !termfreqandwts.empty())
		    termfreqandwts_ptr = NULL;
	    } catch (Xapian::Error & e) {
		if (!errorhandler) throw;
		DEBUGLINE(EXCEPTION, "Calling error handler for "
			  "get_term_info() on a SubMatch.");
		(*errorhandler)(e);
		// FIXME: check if *ALL* the remote servers have failed!
		// Continue match without this sub-match.
		*leaf = new EmptySubMatch();
		pl = new EmptyPostList;
	    }
	    postlists.push_back(pl);
	}
    }
    Assert(!postlists.empty());

    // Get a single combined postlist
    PostList *pl;
    if (postlists.size() == 1) {
	pl = postlists.front();
    } else {
	pl = new MergePostList(postlists, this, errorhandler);
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
	} else if (collapse_key != Xapian::BAD_VALUENO) {
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
    Xapian::doccount max_msize = first + maxitems;
    items.reserve(max_msize + 1);

    // Tracks the minimum item currently eligible for the MSet - we compare
    // candidate items against this.
    Xapian::Internal::MSetItem min_item(0.0, 0);

    // Minimum weight an item must have to be worth considering.
    Xapian::weight min_weight = weight_cutoff;

    Xapian::weight percent_factor = percent_cutoff / 100.0;

    // Table of keys which have been seen already, for collapsing.
    map<string, pair<Xapian::Internal::MSetItem,Xapian::weight> > collapse_tab;

    /// Comparison functor for sorting MSet
    bool sort_forward = (order != Xapian::Enquire::DESCENDING);
    MSetCmp mcmp(get_msetcmp_function(sort_by, sort_forward, sort_value_forward));

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
	    if (min_weight > 0.0 && getorrecalc_maxweight(pl) < min_weight) {
		DEBUGLINE(MATCH, "*** TERMINATING EARLY (1)");
		break;
	    }
	}

	if (next_handling_prune(pl, min_weight, this)) {
	    DEBUGLINE(MATCH, "*** REPLACING ROOT");

	    if (min_weight > 0.0) {
		// No need for a full recalc (unless we've got to do one
		// because of a prune elsewhere) - we're just switching to a
		// subtree.
		if (getorrecalc_maxweight(pl) < min_weight) {
		    DEBUGLINE(MATCH, "*** TERMINATING EARLY (2)");
		    break;
		}
	    }
	}

	if (pl->at_end()) {
	    DEBUGLINE(MATCH, "Reached end of potential matches");
	    break;
	}

	// Only calculate the weight if we need it for mcmp, or there's a
	// percentage or weight cutoff in effect.  Otherwise we calculate it
	// below if we haven't already rejected this candidate.
	Xapian::weight wt = 0.0;
	bool calculated_weight = false;
	if (sort_by != VAL || min_weight > 0.0) {
	    wt = pl->get_weight();
	    if (wt < min_weight) continue;
	    calculated_weight = true;
	}

	Xapian::docid did = pl->get_docid();
	DEBUGLINE(MATCH, "Candidate document id " << did << " wt " << wt);
	Xapian::Internal::MSetItem new_item(wt, did);
	if (sort_by != REL) {
	    const unsigned int multiplier = db.internal.size();
	    Assert(multiplier != 0);
	    Xapian::doccount n = (new_item.did - 1) % multiplier; // which actual database
	    Xapian::docid m = (new_item.did - 1) / multiplier + 1; // real docid in that database
	    Xapian::Internal::RefCntPtr<Xapian::Document::Internal> doc(db.internal[n]->open_document(m, true));
	    new_item.sort_key = doc->get_value(sort_key);

	    // We're sorting by value (in part at least), so compare the item
	    // against the lowest currently in the proto-mset.  If sort_by is
	    // VAL, then new_item.wt won't yet be set, but that doesn't
	    // matter since it's not used by the sort function.
	    if (!mcmp(new_item, min_item)) continue;
	}

	Xapian::Internal::RefCntPtr<Xapian::Document::Internal> doc;

	// Use the decision functor if any.
	if (mdecider != NULL) {
	    const unsigned int multiplier = db.internal.size();
	    Assert(multiplier != 0);
	    Xapian::doccount n = (did - 1) % multiplier; // which actual database
	    // If the results are from a remote database, then the functor will
	    // already have been applied there so we can skip this step.
	    if (!is_remote[n]) {
		if (doc.get() == 0) {
		    Xapian::docid m = (did - 1) / multiplier + 1; // real docid in that database

		    Xapian::Internal::RefCntPtr<Xapian::Document::Internal> temp(db.internal[n]->open_document(m, true));
		    doc = temp;
		}
		Xapian::Document mydoc(doc.get());
		if (!mdecider->operator()(mydoc)) continue;
	    }
	}

	if (!calculated_weight) {
	    // we didn't calculate the weight above, but now we will need it
	    wt = pl->get_weight();
	    new_item.wt = wt;
	}

	bool pushback = true;
	documents_considered++;

	// Perform collapsing on key if requested.
	if (collapse_key != Xapian::BAD_VALUENO) {
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
		    ++duplicates_found;
		    Xapian::Internal::MSetItem &old_item = oldkey->second.first;
		    // FIXME: what about the (sort_by != REL) case here?
		    if (mcmp(old_item, new_item)) {
			DEBUGLINE(MATCH, "collapsem: better exists: " <<
				  new_item.collapse_key);
			// There's already a better match with this key
			++old_item.collapse_count;
			// But maybe the weight is worth noting
			if (new_item.wt > oldkey->second.second) {
			    oldkey->second.second = new_item.wt;
			}
			continue;
		    }
		    // Make a note of the updated collapse count in the
		    // replacement item
		    new_item.collapse_count = old_item.collapse_count + 1;

		    // There was a previous item in the collapse tab so
		    // the MSet can't be empty.
		    Assert(!items.empty());

		    // This is best potential MSet entry with this key which
		    // we've seen so far.  Check if the previous best entry
		    // with this key might still be in the proto-MSet.  If it
		    // might be, we need to check through for it.
		    if (old_item.wt >= min_weight && mcmp(old_item, min_item)) {
			// Scan through (unsorted) MSet looking for entry.
			// FIXME: more efficient way than just scanning?
			Xapian::docid olddid = old_item.did;
			vector<Xapian::Internal::MSetItem>::iterator i;
			for (i = items.begin(); i != items.end(); ++i) {
			    if (i->did == olddid) {
				DEBUGLINE(MATCH, "collapsem: removing " <<
					  olddid << ": " <<
					  new_item.collapse_key);
				// We can replace an arbitrary element in
				// O(log N) but have to do it by hand (in this
				// case the new elt is bigger, so we just swap
				// down the tree).
				// FIXME: implement this, and clean up is_heap
				// handling
				*i = new_item;
				pushback = false;
				is_heap = false;
				break;
			    }
			}
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
		items.push_back(new_item);
		if (!is_heap) {
		    is_heap = true;
		    make_heap<vector<Xapian::Internal::MSetItem>::iterator,
			      MSetCmp>(items.begin(), items.end(), mcmp);
		} else {
		    push_heap<vector<Xapian::Internal::MSetItem>::iterator,
			      MSetCmp>(items.begin(), items.end(), mcmp);
		}
		pop_heap<vector<Xapian::Internal::MSetItem>::iterator,
			 MSetCmp>(items.begin(), items.end(), mcmp);
		items.pop_back();

		min_item = items.front();
		if (sort_by == REL || sort_by == REL_VAL) {
		    if (docs_matched >= check_at_least) {
			if (min_item.wt > min_weight) min_weight = min_item.wt;
		    }
		}
		if (getorrecalc_maxweight(pl) < min_weight) {
		    DEBUGLINE(MATCH, "*** TERMINATING EARLY (3)");
		    break;
		}
	    } else {
		items.push_back(new_item);
		is_heap = false;
		if (sort_by == REL && items.size() == max_msize) {
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
		if (w > min_weight) {
		    min_weight = w;
		    if (!is_heap) {
			is_heap = true;
			make_heap<vector<Xapian::Internal::MSetItem>::iterator,
				  MSetCmp>(items.begin(), items.end(), mcmp);
		    }
		    while (!items.empty() && items.front().wt < min_weight) {
			pop_heap<vector<Xapian::Internal::MSetItem>::iterator,
				 MSetCmp>(items.begin(), items.end(), mcmp);
			Assert(items.back().wt < min_weight);
			items.pop_back();
		    }
#ifdef XAPIAN_DEBUG_PARANOID
		    vector<Xapian::Internal::MSetItem>::const_iterator i;
		    for (i = items.begin(); i != items.end(); ++i) {
			Assert(i->wt >= min_weight);
		    }
#endif
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

	if (termfreqandwts.size() > 1) {
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
	    // Special case for MatchAll queries
	    i = termfreqandwts.find("");
	    if (i != termfreqandwts.end()) {
		percent_scale += i->second.termweight;
		++matching_terms;
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
	} else {
	    // If there's only a single term in the query, the top document
	    // must score 100%.
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
			  MSetCmp>(items.begin(), items.end(), mcmp);
	    }
	    while (!items.empty() && items.front().wt < min_wt) {
		pop_heap<vector<Xapian::Internal::MSetItem>::iterator,
			 MSetCmp>(items.begin(), items.end(), mcmp);
		Assert(items.back().wt < min_wt);
		items.pop_back();
	    }
#ifdef XAPIAN_DEBUG_PARANOID
	    vector<Xapian::Internal::MSetItem>::const_iterator j;
	    for (j = items.begin(); j != items.end(); ++j) {
		Assert(j->wt >= min_wt);
	    }
#endif
	}
	percent_scale *= 100.0;
    }

    if (items.size() < max_msize) {
	DEBUGLINE(MATCH, "items.size() = " << items.size() <<
		  ", max_msize = " << max_msize << ", setting bounds equal");
	Assert(percent_cutoff || docs_matched == items.size());
	matches_lower_bound = matches_upper_bound = matches_estimated
	    = items.size();
    } else {
	if (percent_cutoff) {
	    // another approach: Xapian::doccount new_est = items.size() * (1 - percent_factor) / (1 - min_weight / greatest_wt);
	    Xapian::doccount new_est;
	    new_est = Xapian::doccount((1 - percent_factor) * matches_estimated);
	    matches_estimated = max(size_t(new_est), items.size());
	    // and another: items.size() + (1 - greatest_wt * percent_factor / min_weight) * (matches_estimated - items.size());

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

	if (collapse_key != Xapian::BAD_VALUENO) {
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

    if (first > 0) {
	// Remove unwanted leading entries
	if (items.size() <= first) {
	    items.clear();
	} else {
	    DEBUGLINE(MATCH, "finding " << first << "th");
	    nth_element(items.begin(), items.begin() + first, items.end(),
			mcmp);
	    // Erase the leading ``first'' elements
	    items.erase(items.begin(), items.begin() + first);
	}
    }

    DEBUGLINE(MATCH, "msize = " << items.size());

    if (!items.empty()) {
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
    // We WILL need to restore collapse_count to the mset by taking from
    // collapse_tab; this is what comes of copying around whole objects
    // instead of taking references, we find it hard to update collapse_count
    // of an item that has already been pushed-back as we don't know where it is
    // any more.  If we keep or find references we won't need to mess with
    // is_heap so much maybe?
    if (collapse_key != Xapian::BAD_VALUENO && /*percent_cutoff &&*/ !items.empty() &&
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
		// If weight of top collapsed item is not relevant enough
		// then collapse count is bogus in every way
		// FIXME: Should this be <=?
		if (key->second.second < min_wt)
		    i->collapse_count = 0;
		else
		    i->collapse_count = key->second.first.collapse_count;
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
