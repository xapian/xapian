/* multimatch.cc
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010 Olly Betts
 * Copyright 2003 Orange PCS Ltd
 * Copyright 2003 Sam Liddicott
 * Copyright 2007,2008,2009 Lemur Consulting Ltd
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

#include "autoptr.h"
#include "collapser.h"
#include "debuglog.h"
#include "submatch.h"
#include "localsubmatch.h"
#include "omassert.h"
#include "omenquireinternal.h"

#include "emptypostlist.h"
#include "branchpostlist.h"
#include "mergepostlist.h"

#include "document.h"
#include "omqueryinternal.h"

#include "msetcmp.h"

#include "valuestreamdocument.h"
#include "weightinternal.h"

#include <xapian/errorhandler.h>
#include <xapian/matchspy.h>
#include <xapian/version.h> // For XAPIAN_HAS_REMOTE_BACKEND

#ifdef XAPIAN_HAS_REMOTE_BACKEND
#include "remotesubmatch.h"
#include "remote-database.h"
#endif /* XAPIAN_HAS_REMOTE_BACKEND */

#include <algorithm>
#include <cfloat> // For DBL_EPSILON.
#include <climits> // For UINT_MAX.
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

/** Split an RSet into several sub rsets, one for each database.
 *
 *  @param rset The RSet to split.
 *  @param number_of_subdbs The number of sub databases which exist.
 *  @param subrsets Vector of RSets which will the sub rsets will be placed in.
 *                  This should be empty when the function is called.
 */
static void
split_rset_by_db(const Xapian::RSet * rset,
		 Xapian::doccount number_of_subdbs,
		 vector<Xapian::RSet> & subrsets)
{
    LOGCALL_STATIC_VOID(MATCH, "split_rset_by_db", rset | number_of_subdbs | subrsets);
    if (rset) {
	if (number_of_subdbs == 1) {
	    // The common case of a single database is easy to handle.
	    subrsets.push_back(*rset);
	} else {
	    // Can't just use vector::resize() here, since that creates N
	    // copies of the same RSet!
	    subrsets.reserve(number_of_subdbs);
	    for (size_t i = 0; i < number_of_subdbs; ++i) {
		subrsets.push_back(Xapian::RSet());
	    }

	    const set<Xapian::docid> & rsetitems = rset->internal->get_items();
	    set<Xapian::docid>::const_iterator j;
	    for (j = rsetitems.begin(); j != rsetitems.end(); ++j) {
		Xapian::doccount local_docid = (*j - 1) / number_of_subdbs + 1;
		Xapian::doccount subdatabase = (*j - 1) % number_of_subdbs;
		subrsets[subdatabase].add_document(local_docid);
	    }
	}
    } else {
	// NB vector::resize() creates N copies of the same empty RSet.
	subrsets.resize(number_of_subdbs);
    }
    Assert(subrsets.size() == number_of_subdbs);
}

/** Prepare some SubMatches.
 *
 *  This calls the prepare_match() method on each SubMatch object, causing them
 *  to lookup various statistics.
 *
 *  This method is rather complicated in order to handle remote matches
 *  efficiently.  Instead of simply calling "prepare_match()" on each submatch
 *  and waiting for it to return, it first calls "prepare_match(true)" on each
 *  submatch.  If any of these calls return false, indicating that the required
 *  information has not yet been received from the server, the method will try
 *  those which returned false again, passing "false" as a parameter to
 *  indicate that they should block until the information is ready.
 *
 *  This should improve performance in the case of mixed local-and-remote
 *  searches - the local searchers will all fetch their statistics from disk
 *  without waiting for the remote searchers, so as soon as the remote searcher
 *  statistics arrive, we can move on to the next step.
 */
static void
prepare_sub_matches(vector<Xapian::Internal::RefCntPtr<SubMatch> > & leaves,
		    Xapian::ErrorHandler * errorhandler,
		    Xapian::Weight::Internal & stats)
{
    LOGCALL_STATIC_VOID(MATCH, "prepare_sub_matches", leaves | errorhandler | stats);
    // We use a vector<bool> to track which SubMatches we're already prepared.
    vector<bool> prepared;
    prepared.resize(leaves.size(), false);
    size_t unprepared = leaves.size();
    bool nowait = true;
    while (unprepared) {
	for (size_t leaf = 0; leaf < leaves.size(); ++leaf) {
	    if (prepared[leaf]) continue;
	    try {
		SubMatch * submatch = leaves[leaf].get();
		if (!submatch || submatch->prepare_match(nowait, stats)) {
		    prepared[leaf] = true;
		    --unprepared;
		}
	    } catch (Xapian::Error & e) {
		if (!errorhandler) throw;

		LOGLINE(EXCEPTION, "Calling error handler for prepare_match() on a SubMatch.");
		(*errorhandler)(e);
		// Continue match without this sub-match.
		leaves[leaf] = NULL;
		prepared[leaf] = true;
		--unprepared;
	    }
	}
	// Use blocking IO on subsequent passes, so that we don't go into
	// a tight loop.
	nowait = false;
    }
}

/// Class which applies several match spies in turn.
class MultipleMatchSpy : public Xapian::MatchSpy {
  private:
    /// List of match spies to call, in order.
    const std::vector<Xapian::MatchSpy *> & spies;

  public:
    MultipleMatchSpy(const std::vector<Xapian::MatchSpy *> & spies_)
	    : spies(spies_) {}

    /** Implementation of virtual operator().
     *
     *  This implementation calls all the spies in turn.
     */
    void operator()(const Xapian::Document &doc, Xapian::weight wt);
};

void 
MultipleMatchSpy::operator()(const Xapian::Document &doc, Xapian::weight wt) {
    LOGCALL_VOID(MATCH, "MultipleMatchSpy::operator()", doc | wt);
    vector<Xapian::MatchSpy *>::const_iterator i;
    for (i = spies.begin(); i != spies.end(); ++i) {
	(**i)(doc, wt);
    }
}

////////////////////////////////////
// Initialisation and cleaning up //
////////////////////////////////////
MultiMatch::MultiMatch(const Xapian::Database &db_,
		       const Xapian::Query::Internal * query_,
		       Xapian::termcount qlen,
		       const Xapian::RSet * omrset,
		       Xapian::doccount collapse_max_,
		       Xapian::valueno collapse_key_,
		       int percent_cutoff_, Xapian::weight weight_cutoff_,
		       Xapian::Enquire::docid_order order_,
		       Xapian::valueno sort_key_,
		       Xapian::Enquire::Internal::sort_setting sort_by_,
		       bool sort_value_forward_,
		       Xapian::ErrorHandler * errorhandler_,
		       Xapian::Weight::Internal & stats,
		       const Xapian::Weight * weight_,
		       const vector<Xapian::MatchSpy *> & matchspies_,
		       bool have_sorter, bool have_mdecider)
	: db(db_), query(query_),
	  collapse_max(collapse_max_), collapse_key(collapse_key_),
	  percent_cutoff(percent_cutoff_), weight_cutoff(weight_cutoff_),
	  order(order_),
	  sort_key(sort_key_), sort_by(sort_by_),
	  sort_value_forward(sort_value_forward_),
	  errorhandler(errorhandler_), weight(weight_),
	  is_remote(db.internal.size()),
	  matchspies(matchspies_)
{
    LOGCALL_CTOR(MATCH, "MultiMatch", db_ | query_ | qlen | omrset | collapse_max_ | collapse_key_ | percent_cutoff_ | weight_cutoff_ | int(order_) | sort_key_ | int(sort_by_) | sort_value_forward_ | errorhandler_ | stats | weight_ | matchspies_ | have_sorter | have_mdecider);

    if (!query) return;
    query->validate_query();

    Xapian::doccount number_of_subdbs = db.internal.size();
    vector<Xapian::RSet> subrsets;
    split_rset_by_db(omrset, number_of_subdbs, subrsets);

    for (size_t i = 0; i != number_of_subdbs; ++i) {
	Xapian::Database::Internal *subdb = db.internal[i].get();
	Assert(subdb);
	Xapian::Internal::RefCntPtr<SubMatch> smatch;
	try {
	    // There is currently only one special case, for network databases.
#ifdef XAPIAN_HAS_REMOTE_BACKEND
	    RemoteDatabase *rem_db = subdb->as_remotedatabase();
	    if (rem_db) {
		if (have_sorter) {
		    throw Xapian::UnimplementedError("Xapian::KeyMaker not supported for the remote backend");
		}
		if (have_mdecider) {
		    throw Xapian::UnimplementedError("Xapian::MatchDecider not supported for the remote backend");
		}
		rem_db->set_query(query, qlen, collapse_max, collapse_key,
				  order, sort_key, sort_by, sort_value_forward,
				  percent_cutoff, weight_cutoff, weight,
				  subrsets[i], matchspies);
		bool decreasing_relevance =
		    (sort_by == REL || sort_by == REL_VAL);
		smatch = new RemoteSubMatch(rem_db, decreasing_relevance, matchspies);
		is_remote[i] = true;
	    } else {
		smatch = new LocalSubMatch(subdb, query, qlen, subrsets[i], weight);
	    }
#else
	    // Avoid unused parameter warnings.
	    (void)have_sorter;
	    (void)have_mdecider;
	    smatch = new LocalSubMatch(subdb, query, qlen, subrsets[i], weight);
#endif /* XAPIAN_HAS_REMOTE_BACKEND */
	} catch (Xapian::Error & e) {
	    if (!errorhandler) throw;
	    LOGLINE(EXCEPTION, "Calling error handler for creation of a SubMatch from a database and query.");
	    (*errorhandler)(e);
	    // Continue match without this sub-postlist.
	    smatch = NULL;
	}
	leaves.push_back(smatch);
    }

    stats.mark_wanted_terms(*query);
    prepare_sub_matches(leaves, errorhandler, stats);
    stats.set_bounds_from_db(db);
}

Xapian::weight
MultiMatch::getorrecalc_maxweight(PostList *pl)
{
    LOGCALL(MATCH, Xapian::weight, "MultiMatch::getorrecalc_maxweight", pl);
    Xapian::weight wt;
    if (recalculate_w_max) {
	LOGLINE(MATCH, "recalculating max weight");
	wt = pl->recalc_maxweight();
	recalculate_w_max = false;
    } else {
	wt = pl->get_maxweight();
	LOGLINE(MATCH, "pl = (" << pl->get_description() << ")");
	AssertEqDoubleParanoid(wt, pl->recalc_maxweight());
    }
    LOGLINE(MATCH, "max possible doc weight = " << wt);
    RETURN(wt);
}

void
MultiMatch::get_mset(Xapian::doccount first, Xapian::doccount maxitems,
		     Xapian::doccount check_at_least,
		     Xapian::MSet & mset,
		     const Xapian::Weight::Internal & stats,
		     const Xapian::MatchDecider *mdecider,
		     const Xapian::MatchDecider *matchspy_legacy,
		     const Xapian::KeyMaker *sorter)
{
    LOGCALL_VOID(MATCH, "MultiMatch::get_mset", first | maxitems | check_at_least | Literal("mset") | stats | Literal("mdecider") | Literal("matchspy_legacy") | Literal("sorter"));
    AssertRel(check_at_least,>=,maxitems);

    if (!query) {
	mset = Xapian::MSet(new Xapian::MSet::Internal());
	mset.internal->firstitem = first;
	return;
    }

    Assert(!leaves.empty());

#ifdef XAPIAN_HAS_REMOTE_BACKEND
    // If there's only one database and it's remote, we can just unserialise
    // its MSet and return that.
    if (leaves.size() == 1 && is_remote[0]) {
	RemoteSubMatch * rem_match;
	rem_match = static_cast<RemoteSubMatch*>(leaves[0].get());
	rem_match->start_match(first, maxitems, check_at_least, stats);
	rem_match->get_mset(mset);
	return;
    }
#endif

    // Start matchers.
    {
	vector<Xapian::Internal::RefCntPtr<SubMatch> >::iterator leaf;
	for (leaf = leaves.begin(); leaf != leaves.end(); ++leaf) {
	    if (!(*leaf).get()) continue;
	    try {
		(*leaf)->start_match(0, first + maxitems,
				     first + check_at_least, stats);
	    } catch (Xapian::Error & e) {
		if (!errorhandler) throw;
		LOGLINE(EXCEPTION, "Calling error handler for "
				   "start_match() on a SubMatch.");
		(*errorhandler)(e);
		// Continue match without this sub-match.
		*leaf = NULL;
	    }
	}
    }

    // Get postlists and term info
    vector<PostList *> postlists;
    map<string, Xapian::MSet::Internal::TermFreqAndWeight> termfreqandwts;
    map<string, Xapian::MSet::Internal::TermFreqAndWeight> * termfreqandwts_ptr;
    termfreqandwts_ptr = &termfreqandwts;

    Xapian::termcount total_subqs = 0;
    // Keep a count of matches which we know exist, but we won't see.  This
    // occurs when a submatch is remote, and returns a lower bound on the
    // number of matching documents which is higher than the number of
    // documents it returns (because it wasn't asked for more documents).
    Xapian::doccount definite_matches_not_seen = 0;
    for (size_t i = 0; i != leaves.size(); ++i) {
	PostList *pl;
	try {
	    pl = leaves[i]->get_postlist_and_term_info(this,
						       termfreqandwts_ptr,
						       &total_subqs);
	    if (termfreqandwts_ptr && !termfreqandwts.empty())
		termfreqandwts_ptr = NULL;
	    if (is_remote[i]) {
		if (pl->get_termfreq_min() > first + maxitems) {
		    LOGLINE(MATCH, "Found " <<
				   pl->get_termfreq_min() - (first + maxitems)
				   << " definite matches in remote submatch "
				   "which aren't passed to local match");
		    definite_matches_not_seen += pl->get_termfreq_min();
		    definite_matches_not_seen -= first + maxitems;
		}
	    }
	} catch (Xapian::Error & e) {
	    if (!errorhandler) throw;
	    LOGLINE(EXCEPTION, "Calling error handler for "
			       "get_term_info() on a SubMatch.");
	    (*errorhandler)(e);
	    // FIXME: check if *ALL* the remote servers have failed!
	    // Continue match without this sub-match.
	    leaves[i] = NULL;
	    pl = new EmptyPostList;
	}
	postlists.push_back(pl);
    }
    Assert(!postlists.empty());

    ValueStreamDocument vsdoc(db);
    ++vsdoc.ref_count;
    Xapian::Document doc(&vsdoc);

    // Get a single combined postlist
    AutoPtr<PostList> pl;
    if (postlists.size() == 1) {
	pl.reset(postlists.front());
    } else {
	pl.reset(new MergePostList(postlists, this, vsdoc, errorhandler));
    }

    LOGLINE(MATCH, "pl = (" << pl->get_description() << ")");

#ifdef XAPIAN_DEBUG_LOG
    {
	map<string, Xapian::MSet::Internal::TermFreqAndWeight>::const_iterator tfwi;
	for (tfwi = termfreqandwts.begin(); tfwi != termfreqandwts.end(); ++tfwi) {
	    LOGLINE(MATCH, "termfreqandwts[" << tfwi->first << "] = " << tfwi->second.termfreq << ", " << tfwi->second.termweight);
	}
    }
#endif

    // Empty result set
    Xapian::doccount docs_matched = 0;
    Xapian::weight greatest_wt = 0;
    Xapian::termcount greatest_wt_subqs_matched = 0;
#ifdef XAPIAN_HAS_REMOTE_BACKEND
    unsigned greatest_wt_subqs_db_num = UINT_MAX;
#endif
    vector<Xapian::Internal::MSetItem> items;

    // maximum weight a document could possibly have
    const Xapian::weight max_possible = pl->recalc_maxweight();

    LOGLINE(MATCH, "pl = (" << pl->get_description() << ")");
    recalculate_w_max = false;

    Xapian::doccount matches_upper_bound = pl->get_termfreq_max();
    Xapian::doccount matches_lower_bound = 0;
    Xapian::doccount matches_estimated   = pl->get_termfreq_est();

    if (mdecider == NULL && matchspy_legacy == NULL) {
	// If we have a matcher decider or match spy, the lower bound must be
	// set to 0 as we could discard all hits.  Otherwise set it to the
	// minimum number of entries which the postlist could return.
	matches_lower_bound = pl->get_termfreq_min();
    }

    // Prepare the matchspy
    Xapian::MatchSpy *matchspy = NULL;
    MultipleMatchSpy multispy(matchspies);
    if (!matchspies.empty()) {
	if (matchspies.size() == 1) {
	    matchspy = matchspies[0];
	} else {
	    matchspy = &multispy;
	}
    }

    // Check if any results have been asked for (might just be wanting
    // maxweight).
    if (check_at_least == 0) {
	pl.reset(NULL);
	Xapian::doccount uncollapsed_lower_bound = matches_lower_bound;
	if (collapse_max) {
	    // Lower bound must be set to no more than collapse_max, since it's
	    // possible that all matching documents have the same collapse_key
	    // value and so are collapsed together.
	    if (matches_lower_bound > collapse_max)
		matches_lower_bound = collapse_max;
	}

	mset = Xapian::MSet(new Xapian::MSet::Internal(
					   first,
					   matches_upper_bound,
					   matches_lower_bound,
					   matches_estimated,
					   matches_upper_bound,
					   uncollapsed_lower_bound,
					   matches_estimated,
					   max_possible, greatest_wt, items,
					   termfreqandwts,
					   0));
	return;
    }

    // Number of documents considered by a decider or matchspy_legacy.
    Xapian::doccount decider_considered = 0;
    // Number of documents denied by the decider or matchspy_legacy.
    Xapian::doccount decider_denied = 0;

    // Set max number of results that we want - this is used to decide
    // when to throw away unwanted items.
    Xapian::doccount max_msize = first + maxitems;
    items.reserve(max_msize + 1);

    // Tracks the minimum item currently eligible for the MSet - we compare
    // candidate items against this.
    Xapian::Internal::MSetItem min_item(0.0, 0);

    // Minimum weight an item must have to be worth considering.
    Xapian::weight min_weight = weight_cutoff;

    // Factor to multiply maximum weight seen by to get the cutoff weight.
    Xapian::weight percent_cutoff_factor = percent_cutoff / 100.0;
    // Corresponding correction to that in omenquire.cc to account for excess
    // precision on x86.
    percent_cutoff_factor -= DBL_EPSILON;

    // Object to handle collapsing.
    Collapser collapser(collapse_key, collapse_max);

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
	bool pushback;

	if (rare(recalculate_w_max)) {
	    if (min_weight > 0.0) {
		if (rare(getorrecalc_maxweight(pl.get()) < min_weight)) {
		    LOGLINE(MATCH, "*** TERMINATING EARLY (1)");
		    break;
		}
	    }
	}

	PostList * pl_copy = pl.get();
	if (rare(next_handling_prune(pl_copy, min_weight, this))) {
	    (void)pl.release();
	    pl.reset(pl_copy);
	    LOGLINE(MATCH, "*** REPLACING ROOT");

	    if (min_weight > 0.0) {
		// No need for a full recalc (unless we've got to do one
		// because of a prune elsewhere) - we're just switching to a
		// subtree.
		if (rare(getorrecalc_maxweight(pl.get()) < min_weight)) {
		    LOGLINE(MATCH, "*** TERMINATING EARLY (2)");
		    break;
		}
	    }
	}

	if (rare(pl->at_end())) {
	    LOGLINE(MATCH, "Reached end of potential matches");
	    break;
	}

	// Only calculate the weight if we need it for mcmp, or there's a
	// percentage or weight cutoff in effect.  Otherwise we calculate it
	// below if we haven't already rejected this candidate.
	Xapian::weight wt = 0.0;
	bool calculated_weight = false;
	if (sort_by != VAL || min_weight > 0.0) {
	    wt = pl->get_weight();
	    if (wt < min_weight) {
		LOGLINE(MATCH, "Rejecting potential match due to insufficient weight");
		continue;
	    }
	    calculated_weight = true;
	}

	Xapian::docid did = pl->get_docid();
	vsdoc.set_document(did);
	LOGLINE(MATCH, "Candidate document id " << did << " wt " << wt);
	Xapian::Internal::MSetItem new_item(wt, did);
	if (sort_by != REL) {
	    if (sorter) {
		new_item.sort_key = (*sorter)(doc);
	    } else {
		new_item.sort_key = vsdoc.get_value(sort_key);
	    }

	    // We're sorting by value (in part at least), so compare the item
	    // against the lowest currently in the proto-mset.  If sort_by is
	    // VAL, then new_item.wt won't yet be set, but that doesn't
	    // matter since it's not used by the sort function.
	    if (!mcmp(new_item, min_item)) {
		if (mdecider == NULL && !collapser && matchspy_legacy == NULL) {
		    // Document was definitely suitable for mset - no more
		    // processing needed.
		    LOGLINE(MATCH, "Making note of match item which sorts lower than min_item");
		    ++docs_matched;
		    if (!calculated_weight) wt = pl->get_weight();
		    if (matchspy) {
			matchspy->operator()(doc, wt);
		    }
		    if (wt > greatest_wt) goto new_greatest_weight;
		    continue;
		}
		if (docs_matched >= check_at_least) {
		    // We've seen enough items - we can drop this one.
		    LOGLINE(MATCH, "Dropping candidate which sorts lower than min_item");
		    // FIXME: hmm, match decider might have rejected this...
		    if (!calculated_weight) wt = pl->get_weight();
		    if (wt > greatest_wt) goto new_greatest_weight;
		    continue;
		}
		// We can't drop the item, because we need to show it
		// to the matchspy_legacy, test whether the mdecider would
		// accept it, and/or test whether it would be collapsed.
		LOGLINE(MATCH, "Keeping candidate which sorts lower than min_item for further investigation");
	    }
	}

	// Use the match spy and/or decision functors (if specified).
	if (matchspy != NULL || mdecider != NULL || matchspy_legacy != NULL) {
	    const unsigned int multiplier = db.internal.size();
	    Assert(multiplier != 0);
	    Xapian::doccount n = (did - 1) % multiplier; // which actual database
	    // If the results are from a remote database, then the functor will
	    // already have been applied there so we can skip this step.
	    if (!is_remote[n]) {
		++decider_considered;
		if (matchspy_legacy && !matchspy_legacy->operator()(doc)) {
		    ++decider_denied;
		    continue;
		}
		if (mdecider && !mdecider->operator()(doc)) {
		    ++decider_denied;
		    continue;
		}
		if (matchspy) {
		    if (!calculated_weight) {
			wt = pl->get_weight();
			new_item.wt = wt;
			calculated_weight = true;
		    }
		    matchspy->operator()(doc, wt);
		}
	    }
	}

	if (!calculated_weight) {
	    // we didn't calculate the weight above, but now we will need it
	    wt = pl->get_weight();
	    new_item.wt = wt;
	}

	pushback = true;

	// Perform collapsing on key if requested.
	if (collapser) {
	    collapse_result res;
	    res = collapser.process(new_item, pl.get(), vsdoc, mcmp);
	    if (res == REJECTED) {
		// If we're sorting by relevance primarily, then we throw away
		// the lower weighted document anyway.
		if (sort_by != REL && sort_by != REL_VAL) {
		    if (wt > greatest_wt) goto new_greatest_weight;
		}
		continue;
	    }

	    if (res == REPLACED) {
		// There was a previous item in the collapse tab so
		// the MSet can't be empty.
		Assert(!items.empty());

		const Xapian::Internal::MSetItem & old_item =
		    collapser.old_item;
		// This is one of the best collapse_max potential MSet entries
		// with this key which we've seen so far.  Check if the
		// entry with this key which it displaced might still be in the
		// proto-MSet.  If it might be, we need to check through for
		// it.
		Xapian::weight old_wt = old_item.wt;
		if (old_wt >= min_weight && mcmp(old_item, min_item)) {
		    // Scan through (unsorted) MSet looking for entry.
		    // FIXME: more efficient way than just scanning?
		    Xapian::docid olddid = old_item.did;
		    vector<Xapian::Internal::MSetItem>::iterator i;
		    for (i = items.begin(); i != items.end(); ++i) {
			if (i->did == olddid) {
			    LOGLINE(MATCH, "collapse: removing " <<
					   olddid << ": " <<
					   new_item.collapse_key);
			    // We can replace an arbitrary element in O(log N)
			    // but have to do it by hand (in this case the new
			    // elt is bigger, so we just swap down the tree).
			    // FIXME: implement this, and clean up is_heap
			    // handling
			    *i = new_item;
			    pushback = false;
			    is_heap = false;
			    break;
			}
		    }
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
		    make_heap(items.begin(), items.end(), mcmp);
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
			if (sort_by == REL) {
			    // We're done if this is a forward boolean match
			    // with only one database (bodgetastic, FIXME
			    // better if we can!)
			    if (rare(max_possible == 0 && sort_forward)) {
				// In the multi database case, MergePostList
				// currently processes each database
				// sequentially (which actually may well be
				// more efficient) so the docids in general
				// won't arrive in order.
				if (leaves.size() == 1) break;
			    }
			}
			if (min_item.wt > min_weight) {
			    LOGLINE(MATCH, "Setting min_weight to " <<
				    min_item.wt << " from " << min_weight);
			    min_weight = min_item.wt;
			}
		    }
		}
		if (rare(getorrecalc_maxweight(pl.get()) < min_weight)) {
		    LOGLINE(MATCH, "*** TERMINATING EARLY (3)");
		    break;
		}
	    } else {
		items.push_back(new_item);
		is_heap = false;
		if (sort_by == REL && items.size() == max_msize) {
		    if (docs_matched >= check_at_least) {
			// We're done if this is a forward boolean match
			// with only one database (bodgetastic, FIXME
			// better if we can!)
			if (rare(max_possible == 0 && sort_forward)) {
			    // In the multi database case, MergePostList
			    // currently processes each database
			    // sequentially (which actually may well be
			    // more efficient) so the docids in general
			    // won't arrive in order.
			    if (leaves.size() == 1) break;
			}
		    }
		}
	    }
	}

	// Keep a track of the greatest weight we've seen.
	if (wt > greatest_wt) {
new_greatest_weight:
	    greatest_wt = wt;
#ifdef XAPIAN_HAS_REMOTE_BACKEND
	    const unsigned int multiplier = db.internal.size();
	    unsigned int db_num = (did - 1) % multiplier;
	    if (is_remote[db_num]) {
		// Note that the greatest weighted document came from a remote
		// database, and which one.
		greatest_wt_subqs_db_num = db_num;
	    } else
#endif
	    {
		greatest_wt_subqs_matched = pl->count_matching_subqs();
#ifdef XAPIAN_HAS_REMOTE_BACKEND
		greatest_wt_subqs_db_num = UINT_MAX;
#endif
	    }
	    if (percent_cutoff) {
		Xapian::weight w = wt * percent_cutoff_factor;
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
#ifdef XAPIAN_ASSERTIONS_PARANOID
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
    pl.reset(NULL);

    double percent_scale = 0;
    if (!items.empty() && greatest_wt > 0) {
#ifdef XAPIAN_HAS_REMOTE_BACKEND
	if (greatest_wt_subqs_db_num != UINT_MAX) {
	    const unsigned int n = greatest_wt_subqs_db_num;
	    RemoteSubMatch * rem_match;
	    rem_match = static_cast<RemoteSubMatch*>(leaves[n].get());
	    percent_scale = rem_match->get_percent_factor() / 100.0;
	} else
#endif
	{
	    percent_scale = greatest_wt_subqs_matched / double(total_subqs);
	    percent_scale /= greatest_wt;
	}
	Assert(percent_scale > 0);
	if (percent_cutoff) {
	    // FIXME: better to sort and binary chop maybe?
	    // Or we could just use a linear scan here instead.

	    // trim the mset to the correct answer...
	    Xapian::weight min_wt = percent_cutoff_factor / percent_scale;
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
#ifdef XAPIAN_ASSERTIONS_PARANOID
	    vector<Xapian::Internal::MSetItem>::const_iterator j;
	    for (j = items.begin(); j != items.end(); ++j) {
		Assert(j->wt >= min_wt);
	    }
#endif
	}
    }

    LOGLINE(MATCH,
	    "docs_matched = " << docs_matched <<
	    ", definite_matches_not_seen = " << definite_matches_not_seen <<
	    ", matches_lower_bound = " << matches_lower_bound <<
	    ", matches_estimated = " << matches_estimated <<
	    ", matches_upper_bound = " << matches_upper_bound);

    // Adjust docs_matched to take account of documents which matched remotely
    // but weren't sent across.
    docs_matched += definite_matches_not_seen;

    Xapian::doccount uncollapsed_lower_bound = matches_lower_bound;
    Xapian::doccount uncollapsed_upper_bound = matches_upper_bound;
    Xapian::doccount uncollapsed_estimated = matches_estimated;
    if (items.size() < max_msize) {
	// We have fewer items in the mset than we tried to get for it, so we
	// must have all the matches in it.
	LOGLINE(MATCH, "items.size() = " << items.size() <<
		", max_msize = " << max_msize << ", setting bounds equal");
	Assert(definite_matches_not_seen == 0);
	Assert(percent_cutoff || docs_matched == items.size());
	matches_lower_bound = matches_upper_bound = matches_estimated
	    = items.size();
	if (collapser && matches_lower_bound > uncollapsed_lower_bound)
	    uncollapsed_lower_bound = matches_lower_bound;
    } else if (!collapser && docs_matched < check_at_least) {
	// We have seen fewer matches than we checked for, so we must have seen
	// all the matches.
	LOGLINE(MATCH, "Setting bounds equal");
	matches_lower_bound = matches_upper_bound = matches_estimated
	    = docs_matched;
	if (collapser && matches_lower_bound > uncollapsed_lower_bound)
	   uncollapsed_lower_bound = matches_lower_bound;
    } else {
	AssertRel(matches_estimated,>=,matches_lower_bound);
	AssertRel(matches_estimated,<=,matches_upper_bound);

	// We can end up scaling the estimate more than once, so collect
	// the scale factors and apply them in one go to avoid rounding
	// more than once.
	double estimate_scale = 1.0;
	double unique_rate = 1.0;

	if (collapser) {
	    LOGLINE(MATCH, "Adjusting bounds due to collapse_key");
	    matches_lower_bound = collapser.get_matches_lower_bound();
	    if (matches_lower_bound > uncollapsed_lower_bound)
	       uncollapsed_lower_bound = matches_lower_bound;

	    // The estimate for the number of hits can be modified by
	    // multiplying it by the rate at which we've been finding
	    // unique documents.
	    Xapian::doccount docs_considered = collapser.get_docs_considered();
	    Xapian::doccount dups_ignored = collapser.get_dups_ignored();
	    if (docs_considered > 0) {
		double unique = double(docs_considered - dups_ignored);
		unique_rate = unique / double(docs_considered);
	    }

	    // We can safely reduce the upper bound by the number of duplicates
	    // we've ignored.
	    matches_upper_bound -= dups_ignored;

	    LOGLINE(MATCH, "matches_lower_bound=" << matches_lower_bound <<
		    ", matches_estimated=" << matches_estimated <<
		    "*" << estimate_scale << "*" << unique_rate <<
		    ", matches_upper_bound=" << matches_upper_bound);
	}

	if (mdecider || matchspy_legacy) {
	    if (!percent_cutoff) {
		if (!collapser) {
		    // We're not collapsing or doing a percentage cutoff, so
		    // docs_matched is a lower bound on the total number of
		    // matches.
		    matches_lower_bound = max(docs_matched, matches_lower_bound);
		}
	    }

	    // The estimate for the number of hits can be modified by
	    // multiplying it by the rate at which the match decider has
	    // been accepting documents.
	    if (decider_considered > 0) {
		double accept = double(decider_considered - decider_denied);
		double accept_rate = accept / double(decider_considered);
		estimate_scale *= accept_rate;
	    }

	    // If a document is denied by a match decider, it is not possible
	    // for it to found to be a duplicate, so it is safe to also reduce
	    // the upper bound by the number of documents denied by a match
	    // decider.
	    matches_upper_bound -= decider_denied;
	    if (collapser) uncollapsed_upper_bound -= decider_denied;
	}

	if (percent_cutoff) {
	    estimate_scale *= (1.0 - percent_cutoff_factor);
	    // another approach:
	    // Xapian::doccount new_est = items.size() * (1 - percent_cutoff_factor) / (1 - min_weight / greatest_wt);
	    // and another: items.size() + (1 - greatest_wt * percent_cutoff_factor / min_weight) * (matches_estimated - items.size());

	    // Very likely an underestimate, but we can't really do better
	    // without checking further matches...  Only possibility would be
	    // to track how many docs made the min weight test but didn't make
	    // the candidate set since the last greatest_wt change, which we
	    // could use if the top documents matched all the prob terms.
	    matches_lower_bound = items.size();
	    if (collapser) uncollapsed_lower_bound = matches_lower_bound;

	    // matches_upper_bound could be reduced by the number of documents
	    // which fail the min weight test (FIXME)

	    LOGLINE(MATCH, "Adjusted bounds due to percent_cutoff (" <<
		    percent_cutoff << "): now have matches_estimated=" <<
		    matches_estimated << ", matches_lower_bound=" <<
		    matches_lower_bound);
	}

	if (collapser && estimate_scale != 1.0) {
	    uncollapsed_estimated =
		Xapian::doccount(uncollapsed_estimated * estimate_scale + 0.5);
	}

	estimate_scale *= unique_rate;

	if (estimate_scale != 1.0) {
	    matches_estimated =
		Xapian::doccount(matches_estimated * estimate_scale + 0.5);
	    if (matches_estimated < matches_lower_bound)
	       	matches_estimated = matches_lower_bound;
	}

	if (collapser || mdecider || matchspy_legacy) {
	    LOGLINE(MATCH, "Clamping estimate between bounds: "
		    "matches_lower_bound = " << matches_lower_bound <<
		    ", matches_estimated = " << matches_estimated <<
		    ", matches_upper_bound = " << matches_upper_bound);
	    AssertRel(matches_lower_bound,<=,matches_upper_bound);
	    matches_estimated = max(matches_estimated, matches_lower_bound);
	    matches_estimated = min(matches_estimated, matches_upper_bound);
	} else if (!percent_cutoff) {
	    AssertRel(docs_matched,<=,matches_upper_bound);
	    if (docs_matched > matches_lower_bound)
		matches_lower_bound = docs_matched;
	    if (docs_matched > matches_estimated)
		matches_estimated = docs_matched;
	}

	if (collapser && !mdecider && !percent_cutoff && !matchspy_legacy) {
	    AssertRel(docs_matched,<=,uncollapsed_upper_bound);
	    if (docs_matched > uncollapsed_lower_bound)
		uncollapsed_lower_bound = docs_matched;
	}
    }

    if (collapser) {
	AssertRel(uncollapsed_lower_bound,<=,uncollapsed_upper_bound);
	if (uncollapsed_estimated < uncollapsed_lower_bound) {
	    uncollapsed_estimated = uncollapsed_lower_bound;
	} else if (uncollapsed_estimated > uncollapsed_upper_bound) {
	    uncollapsed_estimated = uncollapsed_upper_bound;
	}
    } else {
	// We're not collapsing, so the bounds must be the same.
	uncollapsed_lower_bound = matches_lower_bound;
	uncollapsed_upper_bound = matches_upper_bound;
	uncollapsed_estimated = matches_estimated;
    }

    LOGLINE(MATCH, items.size() << " items in potential mset");

    if (first > 0) {
	// Remove unwanted leading entries
	if (items.size() <= first) {
	    items.clear();
	} else {
	    LOGLINE(MATCH, "finding " << first << "th");
	    // We perform nth_element() on reverse iterators so that the
	    // unwanted elements end up at the end of items, which means
	    // that the call to erase() to remove them doesn't have to copy
	    // any elements.
	    vector<Xapian::Internal::MSetItem>::reverse_iterator nth;
	    nth = items.rbegin() + first;
	    nth_element(items.rbegin(), nth, items.rend(), mcmp);
	    // Erase the trailing ``first'' elements
	    items.erase(items.begin() + items.size() - first, items.end());
	}
    }

    LOGLINE(MATCH, "sorting " << items.size() << " entries");

    // Need a stable sort, but this is provided by comparison operator
    sort(items.begin(), items.end(), mcmp);

    if (!items.empty()) {
	LOGLINE(MATCH, "min weight in mset = " << items.back().wt);
	LOGLINE(MATCH, "max weight in mset = " << items[0].wt);
    }

    AssertRel(matches_estimated,>=,matches_lower_bound);
    AssertRel(matches_estimated,<=,matches_upper_bound);

    AssertRel(uncollapsed_estimated,>=,uncollapsed_lower_bound);
    AssertRel(uncollapsed_estimated,<=,uncollapsed_upper_bound);

    // We may need to qualify any collapse_count to see if the highest weight
    // collapsed item would have qualified percent_cutoff
    // We WILL need to restore collapse_count to the mset by taking from
    // collapse_tab; this is what comes of copying around whole objects
    // instead of taking references, we find it hard to update collapse_count
    // of an item that has already been pushed-back as we don't know where it
    // is any more.  If we keep or find references we won't need to mess with
    // is_heap so much maybe?
    if (!items.empty() && collapser && !collapser.empty()) {
	// Nicked this formula from above.
	Xapian::weight min_wt = 0.0;
	if (percent_scale > 0.0)
	    min_wt = percent_cutoff_factor / percent_scale;
	Xapian::doccount entries = collapser.entries();
	vector<Xapian::Internal::MSetItem>::iterator i;
	for (i = items.begin(); i != items.end(); ++i) {
	    // Skip entries without a collapse key.
	    if (i->collapse_key.empty()) continue;

	    // Set collapse_count appropriately.
	    i->collapse_count = collapser.get_collapse_count(i->collapse_key, percent_cutoff, min_wt);
	    if (--entries == 0) {
		// Stop once we've processed all items with collapse keys.
		break;
	    }
	}
    }

    mset = Xapian::MSet(new Xapian::MSet::Internal(
				       first,
				       matches_upper_bound,
				       matches_lower_bound,
				       matches_estimated,
				       uncollapsed_upper_bound,
				       uncollapsed_lower_bound,
				       uncollapsed_estimated,
				       max_possible, greatest_wt, items,
				       termfreqandwts,
				       percent_scale * 100.0));
}
