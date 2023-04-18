/* multimatch.cc
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2011,2013,2014,2015,2016 Olly Betts
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
#include "api/omenquireinternal.h"
#include "realtime.h"

#include "api/emptypostlist.h"
#include "branchpostlist.h"
#include "mergepostlist.h"

#include "backends/document.h"

#include "msetcmp.h"

#include "valuestreamdocument.h"
#include "weight/weightinternal.h"

#include <xapian/error.h>
#include <xapian/matchspy.h>
#include <xapian/version.h> // For XAPIAN_HAS_REMOTE_BACKEND

#ifdef XAPIAN_HAS_REMOTE_BACKEND
#include "remotesubmatch.h"
#include "backends/remote/remote-database.h"
#endif /* XAPIAN_HAS_REMOTE_BACKEND */

#include <algorithm>
#include <cfloat> // For DBL_EPSILON.
#include <climits> // For UINT_MAX.
#include <vector>
#include <map>
#include <set>

#ifdef HAVE_TIMER_CREATE
#include <signal.h>
#include <time.h>
#include "safeunistd.h" // For _POSIX_* feature test macros.

extern "C" {

static void
set_timeout_flag(union sigval sv)
{
    *(reinterpret_cast<volatile bool*>(sv.sival_ptr)) = true;
}

}

// The monotonic clock is the better basis for timeouts, but not always
// available.

#ifndef _POSIX_MONOTONIC_CLOCK
const clockid_t TIMEOUT_CLOCK = CLOCK_REALTIME;
#elif defined __sun
// Solaris defines CLOCK_MONOTONIC, but "man timer_create" doesn't mention it
// and when using it timer_create() fails with "EPERM" (perhaps you need to be
// root to use it?  I can't test that).
//
// Solaris defines _POSIX_MONOTONIC_CLOCK so we need to special case.
const clockid_t TIMEOUT_CLOCK = CLOCK_REALTIME;
#elif defined __CYGWIN__
// https://cygwin.com/cygwin-api/std-notes.html currently (2023-04-18) says:
//
//     clock_nanosleep currently supports only CLOCK_REALTIME and
//     CLOCK_MONOTONIC.  clock_setres, clock_settime, and timer_create
//     currently support only CLOCK_REALTIME.
//
// So CLOCK_MONOTONIC is defined, but not supported by timer_create().
const clockid_t TIMEOUT_CLOCK = CLOCK_REALTIME;
#else
const clockid_t TIMEOUT_CLOCK = CLOCK_MONOTONIC;
#endif

class TimeOut {
    struct sigevent sev;
    timer_t timerid;
    volatile bool expired;

  public:
    explicit TimeOut(double limit) : expired(false) {
	if (limit > 0) {
	    sev.sigev_notify = SIGEV_THREAD;
	    sev.sigev_notify_function = set_timeout_flag;
	    sev.sigev_notify_attributes = NULL;
	    sev.sigev_value.sival_ptr =
		static_cast<void*>(const_cast<bool*>(&expired));
	    if (usual(timer_create(TIMEOUT_CLOCK, &sev, &timerid) == 0)) {
		struct itimerspec interval;
		interval.it_interval.tv_sec = 0;
		interval.it_interval.tv_nsec = 0;
		RealTime::to_timespec(limit, &interval.it_value);
		if (usual(timer_settime(timerid, 0, &interval, NULL) == 0)) {
		    // Timeout successfully set.
		    return;
		}
		timer_delete(timerid);
	    }
	}
	sev.sigev_notify = SIGEV_NONE;
    }

    ~TimeOut() {
	if (sev.sigev_notify != SIGEV_NONE) {
	    timer_delete(timerid);
	    sev.sigev_notify = SIGEV_NONE;
	}
    }

    bool timed_out() const { return expired; }
};
#else
class TimeOut {
  public:
    explicit TimeOut(double) { }
    bool timed_out() const { return false; }
};
#endif

using namespace std;
using Xapian::Internal::intrusive_ptr;

const Xapian::Enquire::Internal::sort_setting REL =
	Xapian::Enquire::Internal::REL;
const Xapian::Enquire::Internal::sort_setting REL_VAL =
	Xapian::Enquire::Internal::REL_VAL;
const Xapian::Enquire::Internal::sort_setting VAL =
	Xapian::Enquire::Internal::VAL;
#if 0 // VAL_REL isn't currently used so avoid compiler warnings.
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
    if (rset && !rset->empty()) {
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
prepare_sub_matches(vector<intrusive_ptr<SubMatch> > & leaves,
		    Xapian::Weight::Internal & stats)
{
    LOGCALL_STATIC_VOID(MATCH, "prepare_sub_matches", leaves | stats);
    // We use a vector<bool> to track which SubMatches we're already prepared.
    vector<bool> prepared;
    prepared.resize(leaves.size(), false);
    size_t unprepared = leaves.size();
    bool nowait = true;
    while (unprepared) {
	for (size_t leaf = 0; leaf < leaves.size(); ++leaf) {
	    if (prepared[leaf]) continue;
	    SubMatch * submatch = leaves[leaf].get();
	    if (!submatch || submatch->prepare_match(nowait, stats)) {
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
    const std::vector<Xapian::Internal::opt_intrusive_ptr<Xapian::MatchSpy>> & spies;

  public:
    explicit
    MultipleMatchSpy(const std::vector<Xapian::Internal::opt_intrusive_ptr<Xapian::MatchSpy>> & spies_)
	    : spies(spies_) {}

    /** Implementation of virtual operator().
     *
     *  This implementation calls all the spies in turn.
     */
    void operator()(const Xapian::Document &doc, double wt);
};

void
MultipleMatchSpy::operator()(const Xapian::Document &doc, double wt) {
    LOGCALL_VOID(MATCH, "MultipleMatchSpy::operator()", doc | wt);
    for (auto i : spies) {
	(*i)(doc, wt);
    }
}

////////////////////////////////////
// Initialisation and cleaning up //
////////////////////////////////////
MultiMatch::MultiMatch(const Xapian::Database &db_,
		       const Xapian::Query & query_,
		       Xapian::termcount qlen,
		       const Xapian::RSet * omrset,
		       Xapian::doccount collapse_max_,
		       Xapian::valueno collapse_key_,
		       int percent_cutoff_, double weight_cutoff_,
		       Xapian::Enquire::docid_order order_,
		       Xapian::valueno sort_key_,
		       Xapian::Enquire::Internal::sort_setting sort_by_,
		       bool sort_value_forward_,
		       double time_limit_,
		       Xapian::Weight::Internal & stats,
		       const Xapian::Weight * weight_,
		       const vector<Xapian::Internal::opt_intrusive_ptr<Xapian::MatchSpy>> & matchspies_,
		       bool have_sorter, bool have_mdecider)
	: db(db_), query(query_),
	  collapse_max(collapse_max_), collapse_key(collapse_key_),
	  percent_cutoff(percent_cutoff_), weight_cutoff(weight_cutoff_),
	  order(order_),
	  sort_key(sort_key_), sort_by(sort_by_),
	  sort_value_forward(sort_value_forward_),
	  time_limit(time_limit_),
	  weight(weight_),
	  is_remote(db.internal.size()),
	  matchspies(matchspies_)
{
    LOGCALL_CTOR(MATCH, "MultiMatch", db_ | query_ | qlen | omrset | collapse_max_ | collapse_key_ | percent_cutoff_ | weight_cutoff_ | int(order_) | sort_key_ | int(sort_by_) | sort_value_forward_ | time_limit_| stats | weight_ | matchspies_ | have_sorter | have_mdecider);

    if (query.empty()) return;

    Xapian::doccount number_of_subdbs = db.internal.size();
    vector<Xapian::RSet> subrsets;
    split_rset_by_db(omrset, number_of_subdbs, subrsets);

    for (size_t i = 0; i != number_of_subdbs; ++i) {
	Xapian::Database::Internal *subdb = db.internal[i].get();
	Assert(subdb);
	intrusive_ptr<SubMatch> smatch;

	// There is currently only one special case, for network databases.
#ifdef XAPIAN_HAS_REMOTE_BACKEND
	if (subdb->get_backend_info(NULL) == BACKEND_REMOTE) {
	    RemoteDatabase *rem_db = static_cast<RemoteDatabase*>(subdb);
	    if (have_sorter) {
		throw Xapian::UnimplementedError("Xapian::KeyMaker not supported for the remote backend");
	    }
	    if (have_mdecider) {
		throw Xapian::UnimplementedError("Xapian::MatchDecider not supported for the remote backend");
	    }
	    // FIXME: Remote handling for time_limit with multiple
	    // databases may need some work.
	    rem_db->set_query(query, qlen, collapse_max, collapse_key,
			      order, sort_key, sort_by, sort_value_forward,
			      time_limit,
			      percent_cutoff, weight_cutoff, weight,
			      subrsets[i], matchspies);
	    bool decreasing_relevance =
		(sort_by == REL || sort_by == REL_VAL);
	    smatch = new RemoteSubMatch(rem_db, decreasing_relevance, matchspies);
	    is_remote[i] = true;
	} else {
	    smatch = new LocalSubMatch(subdb, query, qlen, subrsets[i], weight,
				       i);
	    subdb->readahead_for_query(query);
	}
#else
	// Avoid unused parameter warnings.
	(void)have_sorter;
	(void)have_mdecider;
	smatch = new LocalSubMatch(subdb, query, qlen, subrsets[i], weight, i);
#endif /* XAPIAN_HAS_REMOTE_BACKEND */
	leaves.push_back(smatch);
    }

    stats.set_query(query);
    prepare_sub_matches(leaves, stats);
    stats.set_bounds_from_db(db);
}

double
MultiMatch::getorrecalc_maxweight(PostList *pl)
{
    LOGCALL(MATCH, double, "MultiMatch::getorrecalc_maxweight", pl);
    double wt;
    if (recalculate_w_max) {
	LOGLINE(MATCH, "recalculating max weight");
	wt = pl->recalc_maxweight();
	recalculate_w_max = false;
    } else {
	wt = pl->get_maxweight();
	LOGLINE(MATCH, "pl = (" << pl->get_description() << ")");
	// FIXME: This fails for hoistnotbug1 under multi_remoteprog_glass:
	// AssertionError: matcher/multimatch.cc:370: within_DBL_EPSILON(wt, pl->recalc_maxweight()) : values were 2.7075940282079162813 and 2.6966211268553141878
	//
	// AssertEqDoubleParanoid(wt, pl->recalc_maxweight());
	//
	// Not sure why - adding code to call recalc_maxweight() doesn't seem
	// to help.  But the max weight not being as low as it could be is
	// only a potential missed optimisation opportunity, not a correctness
	// issue.
    }
    LOGLINE(MATCH, "max possible doc weight = " << wt);
    RETURN(wt);
}

void
MultiMatch::get_mset(Xapian::doccount first, Xapian::doccount maxitems,
		     Xapian::doccount check_at_least,
		     Xapian::MSet & mset,
		     Xapian::Weight::Internal & stats,
		     const Xapian::MatchDecider *mdecider,
		     const Xapian::KeyMaker *sorter)
{
    LOGCALL_VOID(MATCH, "MultiMatch::get_mset", first | maxitems | check_at_least | Literal("mset") | stats | Literal("mdecider") | Literal("sorter"));
    AssertRel(check_at_least,>=,first + maxitems);

    if (query.empty()) {
	mset = Xapian::MSet();
	mset.internal->firstitem = first;
	return;
    }

    Assert(!leaves.empty());

    TimeOut timeout(time_limit);

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
    for (size_t i = 0; i != leaves.size(); ++i) {
	auto& leaf = leaves[i];
#ifdef XAPIAN_HAS_REMOTE_BACKEND
	if (is_remote[i]) {
	    Xapian::doccount remote_maxitems = first + maxitems;
	    if (collapse_max != 0) {
		// If collapsing we need to fetch all check_at_least items in
		// order to satisfy the requirement that if there are <=
		// check_at_least results then then estimated number of matches
		// is exact.
		remote_maxitems = check_at_least;
	    }
	    leaf->start_match(0, remote_maxitems, check_at_least, stats);
	    continue;
	}
#endif
	leaf->start_match(0, first + maxitems, check_at_least, stats);
    }

    // Get postlists and term info
    vector<PostList *> postlists;
    Xapian::termcount total_subqs = 0;
    // Keep a count of matches which we know exist, but we won't see.  This
    // occurs when a submatch is remote, and returns a lower bound on the
    // number of matching documents which is higher than the number of
    // documents it returns (because it wasn't asked for more documents).
    Xapian::doccount definite_matches_not_seen = 0;
#ifdef XAPIAN_HAS_REMOTE_BACKEND
    // Track these for calculating uncollapsed_upper_bound for the local.
    size_t n_remotes = 0;
    Xapian::doccount remote_uncollapsed_upper_bound = 0;
#endif
    try {
	for (size_t i = 0; i != leaves.size(); ++i) {
	    // Pick the highest total subqueries answer amongst the
	    // subdatabases, as the query to postlist conversion doesn't
	    // recurse into positional queries for shards that don't have
	    // positional data when at least one other shard does.
	    Xapian::termcount total_subqs_i = 0;
	    PostList* pl = leaves[i]->get_postlist(this, &total_subqs_i, stats);
	    total_subqs = max(total_subqs, total_subqs_i);
#ifdef XAPIAN_HAS_REMOTE_BACKEND
	    if (is_remote[i]) {
		++n_remotes;
		RemoteSubMatch* rem_match =
		    static_cast<RemoteSubMatch*>(leaves[i].get());
		remote_uncollapsed_upper_bound +=
		    rem_match->get_uncollapsed_upper_bound();
		if (collapse_max == 0 &&
		    pl->get_termfreq_min() > first + maxitems) {
		    LOGLINE(MATCH, "Found " <<
				   pl->get_termfreq_min() - (first + maxitems)
				   << " definite matches in remote submatch "
				   "which aren't passed to local match");
		    definite_matches_not_seen += pl->get_termfreq_min();
		    definite_matches_not_seen -= first + maxitems;
		}
	    }
#endif
	    postlists.push_back(pl);
	}
    } catch (...) {
	for (auto pl : postlists) delete pl;
	throw;
    }
    Assert(!postlists.empty());

    ValueStreamDocument vsdoc(db);
    ++vsdoc._refs;
    Xapian::Document doc(&vsdoc);

    // Get a single combined postlist
    AutoPtr<PostList> pl;
    if (postlists.size() == 1) {
	pl.reset(postlists.front());
    } else {
	pl.reset(new MergePostList(postlists, this, vsdoc));
    }

    LOGLINE(MATCH, "pl = (" << pl->get_description() << ")");

    // Empty result set
    Xapian::doccount docs_matched = 0;
    double greatest_wt = 0;
    Xapian::termcount greatest_wt_subqs_matched = 0;
#ifdef XAPIAN_HAS_REMOTE_BACKEND
    unsigned greatest_wt_subqs_db_num = UINT_MAX;
#endif
    vector<Xapian::Internal::MSetItem> items;

    // maximum weight a document could possibly have
    const double max_possible = pl->recalc_maxweight();

    LOGLINE(MATCH, "pl = (" << pl->get_description() << ")");
    recalculate_w_max = false;

    Xapian::doccount matches_upper_bound = pl->get_termfreq_max();
    Xapian::doccount matches_lower_bound = 0;
    Xapian::doccount matches_estimated   = pl->get_termfreq_est();

    if (mdecider == NULL) {
	// If we have a match decider, the lower bound must be
	// set to 0 as we could discard all hits.  Otherwise set it to the
	// minimum number of entries which the postlist could return.
	matches_lower_bound = pl->get_termfreq_min();
    }

    // Prepare the matchspy
    Xapian::MatchSpy *matchspy = NULL;
    MultipleMatchSpy multispy(matchspies);
    if (!matchspies.empty()) {
	if (matchspies.size() == 1) {
	    matchspy = matchspies[0].get();
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

	mset.internal = new Xapian::MSet::Internal(
					   first,
					   matches_upper_bound,
					   matches_lower_bound,
					   matches_estimated,
					   matches_upper_bound,
					   uncollapsed_lower_bound,
					   matches_estimated,
					   max_possible, greatest_wt, items,
					   0);
	return;
    }

    // Number of documents considered by a decider.
    Xapian::doccount decider_considered = 0;
    // Number of documents denied by the decider.
    Xapian::doccount decider_denied = 0;

    // Set max number of results that we want - this is used to decide
    // when to throw away unwanted items.
    Xapian::doccount max_msize = first + maxitems;
    items.reserve(max_msize + 1);

    // Tracks the minimum item currently eligible for the MSet - we compare
    // candidate items against this.
    Xapian::Internal::MSetItem min_item(0.0, 0);

    // Minimum weight an item must have to be worth considering.
    double min_weight = weight_cutoff;

    // Factor to multiply maximum weight seen by to get the cutoff weight.
    double percent_cutoff_factor = percent_cutoff / 100.0;
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
	double wt = 0.0;
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
	if (check_at_least > first + maxitems && timeout.timed_out()) {
	    check_at_least = first + maxitems;
	}

	if (sort_by != REL) {
	    const string * ptr = pl->get_sort_key();
	    if (ptr) {
		new_item.sort_key = *ptr;
	    } else if (sorter) {
		new_item.sort_key = (*sorter)(doc);
	    } else {
		new_item.sort_key = vsdoc.get_value(sort_key);
	    }

	    // We're sorting by value (in part at least), so compare the item
	    // against the lowest currently in the proto-mset.  If sort_by is
	    // VAL, then new_item.wt won't yet be set, but that doesn't
	    // matter since it's not used by the sort function.
	    if (!mcmp(new_item, min_item)) {
		if (mdecider == NULL && !collapser) {
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
		// We can't drop the item, because we need to test whether the
		// mdecider would accept it and/or test whether it would be
		// collapsed.
		LOGLINE(MATCH, "Keeping candidate which sorts lower than min_item for further investigation");
	    }
	}

	// Use the match spy and/or decision functors (if specified).
	if (matchspy != NULL || mdecider != NULL) {
	    const unsigned int multiplier = db.internal.size();
	    Assert(multiplier != 0);
	    Xapian::doccount n = (did - 1) % multiplier; // which actual database
	    // If the results are from a remote database, then the functor will
	    // already have been applied there so we can skip this step.
	    if (!is_remote[n]) {
		++decider_considered;
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
		double old_wt = old_item.wt;
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

		if (!items.empty()) {
		    // In the get_mset(0, 0, X) (X > 0) case just don't update min_item.
		    min_item = items.front();
		}
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
		double w = wt * percent_cutoff_factor;
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
	    double min_wt = percent_cutoff_factor / percent_scale;
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
#ifdef XAPIAN_HAS_REMOTE_BACKEND
    if (collapser && n_remotes) {
	// We need to adjust uncollapsed_upper_bound if there are multiple
	// shards and some or all are remote.  The lower bound and estimate
	// will be valid, though potentially could be better, but this
	// doesn't seem worth addressing in 1.4.x - the code on master
	// handles this correctly via merging MSet objects.
	if (n_remotes == leaves.size()) {
	    // All shards are remote so we just use the sum of
	    // uncollapsed_upper_bound over the remotes.
	    uncollapsed_upper_bound = remote_uncollapsed_upper_bound;
	} else {
	    // Sum and clamp to the number of documents.  This is crude but
	    // at least gives a valid answer.
	    Xapian::doccount num_docs = db.get_doccount();
	    uncollapsed_upper_bound += remote_uncollapsed_upper_bound;
	    if (uncollapsed_upper_bound < remote_uncollapsed_upper_bound ||
		uncollapsed_upper_bound > num_docs) {
		uncollapsed_upper_bound = num_docs;
	    }
	}
    }
#endif
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

	if (mdecider) {
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

	if (collapser || mdecider) {
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

	if (collapser && !mdecider && !percent_cutoff) {
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
	    // Erase the trailing "first" elements
	    items.erase(items.begin() + items.size() - first, items.end());
	}
    }

    LOGLINE(MATCH, "sorting " << items.size() << " entries");

    // We could use std::sort_heap if is_heap is true, but profiling
    // suggests that's actually slower.  Cast to void to suppress
    // compiler warnings that the last set value of is_heap is never
    // used.
    (void)is_heap;

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
	double min_wt = 0.0;
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

    mset.internal = new Xapian::MSet::Internal(
				       first,
				       matches_upper_bound,
				       matches_lower_bound,
				       matches_estimated,
				       uncollapsed_upper_bound,
				       uncollapsed_lower_bound,
				       uncollapsed_estimated,
				       max_possible, greatest_wt, items,
				       percent_scale * 100.0);
}
