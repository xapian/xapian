/** @file matcher.cc
 * @brief Matcher class
 */
/* Copyright (C) 2006,2008,2009,2010,2011,2017,2018,2019 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "matcher.h"

#include "api/enquireinternal.h"
#include "api/msetinternal.h"
#include "api/rsetinternal.h"
#include "backends/multi/multi_database.h"
#include "deciderpostlist.h"
#include "localsubmatch.h"
#include "msetcmp.h"
#include "omassert.h"
#include "postlisttree.h"
#include "protomset.h"
#include "spymaster.h"
#include "valuestreamdocument.h"
#include "weight/weightinternal.h"

#include <xapian/version.h> // For XAPIAN_HAS_REMOTE_BACKEND

#ifdef XAPIAN_HAS_REMOTE_BACKEND
# include "backends/remote/remote-database.h"
# include "remotesubmatch.h"
#endif

#include <algorithm>
#include <cerrno>
#include <cfloat> // For DBL_EPSILON.
#include <vector>

#ifdef HAVE_POLL_H
# include <poll.h>
#else
# include "safesysselect.h"
#endif

using namespace std;
using Xapian::Internal::opt_intrusive_ptr;

static constexpr auto DOCID = Xapian::Enquire::Internal::DOCID;
static constexpr auto REL = Xapian::Enquire::Internal::REL;
static constexpr auto REL_VAL = Xapian::Enquire::Internal::REL_VAL;
static constexpr auto VAL = Xapian::Enquire::Internal::VAL;
static constexpr auto VAL_REL = Xapian::Enquire::Internal::VAL_REL;

#ifdef XAPIAN_HAS_REMOTE_BACKEND
[[noreturn]]
static void unimplemented(const char* msg)
{
    throw Xapian::UnimplementedError(msg);
}

template<typename Action>
inline void
Matcher::for_all_remotes(Action action)
{
#ifdef HAVE_POLL
    size_t n_remotes = remotes.size();
    if (n_remotes <= 1) {
	// We only need to use poll() when there are at least 2 remote
	// databases we need to wait for.
	if (n_remotes == 1) {
	    // Just execute action and block if it's not ready.
	    action(remotes[0].get());
	}
	return;
    }

    unique_ptr<struct pollfd[]> fds(new struct pollfd[n_remotes]);
    for (size_t i = 0; i != n_remotes; ++i) {
	fds[i].fd = remotes[i]->get_read_fd();
	fds[i].events = POLLIN;
	fds[i].revents = 0;
    }
    do {
	int r = poll(fds.get(), n_remotes, -1);
	if (r <= 0) {
	    // We shouldn't get a timeout, but if we do retry.
	    if (r == 0 || errno == EINTR || errno == EAGAIN) {
		continue;
	    }
	    throw Xapian::NetworkError("poll() failed waiting for remotes",
				       errno);
	}
	size_t i = 0;
	while (i != n_remotes) {
	    if (fds[i].revents) {
		action(remotes[i].get());
		// Swap such that entries we still need to handle are first.
		swap(remotes[i], remotes[--n_remotes]);
		fds[i] = fds[n_remotes];
		// r is number of ready fds.
		if (--r == 0) break;
	    } else {
		++i;
	    }
	}
    } while (n_remotes > 1);

    // If there's only one remote left just execute action and block if it's
    // not ready.
    if (n_remotes == 1) {
	action(remotes[0].get());
    }
#else
#ifndef __WIN32__
    size_t n_remotes = first_oversize;
    fd_set fds;
    while (n_remotes > 1) {
	int nfds = 0;
	FD_ZERO(&fds);
	for (size_t i = 0; i != n_remotes; ++i) {
	    int fd = remotes[i]->get_read_fd();
	    FD_SET(fd, &fds);
	    if (fd >= nfds) nfds = fd + 1;
	}

	int r = select(nfds, &fds, NULL, NULL, NULL);
	if (r <= 0) {
	    int eno = socket_errno();
	    // We shouldn't get a timeout, but if we do retry.
	    if (r == 0 || eno == EINTR || eno == EAGAIN) {
		continue;
	    }
	    throw Xapian::NetworkError("select() failed waiting for remotes",
				       eno);
	}
	size_t i = 0;
	while (i != n_remotes) {
	    int fd = remotes[i]->get_read_fd();
	    if (FD_ISSET(fd, &fds)) {
		action(remotes[i].get());
		// Swap such that entries we still need to handle are first.
		swap(remotes[i], remotes[--n_remotes]);
		// r is number of ready fds.
		if (--r == 0) break;
	    } else {
		++i;
	    }
	}
    }

    // If there's only one remote left just execute action and block if it's
    // not ready.
    if (n_remotes == 1) {
	action(remotes[0].get());
    }
#endif

    // Handle any remotes with fd >= FD_SETSIZE
    for (size_t i = first_oversize; i != remotes.size(); ++i) {
	action(remotes[i].get());
    }
#endif
}
#endif

Matcher::Matcher(const Xapian::Database& db_,
		 bool full_db_has_positions_,
		 const Xapian::Query& query_,
		 Xapian::termcount query_length,
		 const Xapian::RSet* rset,
		 Xapian::Weight::Internal& stats,
		 const Xapian::Weight& wtscheme,
		 bool have_mdecider,
		 Xapian::valueno collapse_key,
		 Xapian::doccount collapse_max,
		 int percent_threshold,
		 double weight_threshold,
		 Xapian::Enquire::docid_order order,
		 Xapian::valueno sort_key,
		 Xapian::Enquire::Internal::sort_setting sort_by,
		 bool sort_val_reverse,
		 double time_limit,
		 const vector<opt_intrusive_ptr<Xapian::MatchSpy>>& matchspies)
    : db(db_), query(query_), full_db_has_positions(full_db_has_positions_)
{
    // An empty query should get handled higher up.
    Assert(!query.empty());

    Xapian::doccount n_shards = db.internal->size();
    vector<Xapian::RSet> subrsets;
    if (rset && rset->internal.get()) {
	rset->internal->shard(n_shards, subrsets);
    } else {
	subrsets.resize(n_shards);
    }

    for (Xapian::doccount i = 0; i != n_shards; ++i) {
	const Xapian::Database::Internal *subdb = db.internal.get();
	if (n_shards > 1) {
	    auto multidb = static_cast<const MultiDatabase*>(subdb);
	    subdb = multidb->shards[i];
	}
	Assert(subdb);
#ifdef XAPIAN_HAS_REMOTE_BACKEND
	if (subdb->get_backend_info(NULL) == BACKEND_REMOTE) {
	    auto as_rem = static_cast<const RemoteDatabase*>(subdb);
	    if (have_mdecider) {
		unimplemented("Xapian::MatchDecider not supported by the "
			      "remote backend");
	    }
	    as_rem->set_query(query, query_length,
			      collapse_key, collapse_max,
			      order, sort_key, sort_by, sort_val_reverse,
			      time_limit,
			      n_shards == 1 ? percent_threshold : 0,
			      weight_threshold,
			      wtscheme,
			      subrsets[i], matchspies,
			      full_db_has_positions);
	    remotes.emplace_back(new RemoteSubMatch(as_rem, i));
	    continue;
	}
#else
	// Avoid unused parameter warnings.
	(void)have_mdecider;
	(void)collapse_key;
	(void)collapse_max;
	(void)percent_threshold;
	(void)weight_threshold;
	(void)order;
	(void)sort_key;
	(void)sort_by;
	(void)sort_val_reverse;
	(void)time_limit;
	(void)matchspies;
#endif /* XAPIAN_HAS_REMOTE_BACKEND */
	if (locals.size() != i)
	    locals.resize(i);
	locals.emplace_back(new LocalSubMatch(subdb, query, query_length,
					      wtscheme,
					      i,
					      full_db_has_positions));
	subdb->readahead_for_query(query);
    }

    if (!locals.empty() && locals.size() != n_shards)
	locals.resize(n_shards);

#ifdef XAPIAN_HAS_REMOTE_BACKEND
# ifndef HAVE_POLL
#  ifndef __WIN32__
    {
	// Unfortunately select() can't monitor fds >= FD_SETSIZE, so swap those to
	// the end here and then handle those last letting them just block if not
	// ready.
	first_oversize = remotes.size();
	size_t i = 0;
	while (i != first_oversize) {
	    int fd = remotes[i]->get_read_fd();
	    if (fd >= FD_SETSIZE) {
		swap(remotes[i], remotes[--first_oversize]);
	    } else {
		++i;
	    }
	}
    }
#  else
    // We can only use select() on sockets under __WIN32__ and with the remote
    // prog backend the fds aren't sockets so just avoid using select() for
    // now.
    //
    // FIXME: perhaps we should use WaitForMultipleObjects(), but that seems a
    // bit tricky to hook up as it probably needs an async ReadFile() to be
    // active.
    first_oversize = 0;
#  endif
# endif
#endif

    stats.set_query(query);

    /* To improve overall performance in the case of searches over a mix of
     * local and remote shards we set the queries for remote shards above,
     * then prepare local shards here, then finish preparing remote shards
     * below.
     */

    if (!locals.empty()) {
	// Prepare local matches.
	for (Xapian::doccount i = 0; i != n_shards; ++i) {
	    auto submatch = locals[i].get();
	    if (submatch) {
		submatch->prepare_match(subrsets[i], stats);
	    }
	}
    }

#ifdef XAPIAN_HAS_REMOTE_BACKEND
    for_all_remotes(
	[&](RemoteSubMatch* submatch) {
	    submatch->prepare_match(stats);
	});
#endif

    stats.set_bounds_from_db(db);
}

Xapian::MSet
Matcher::get_local_mset(Xapian::doccount first,
			Xapian::doccount maxitems,
			Xapian::doccount check_at_least,
			const Xapian::Weight& wtscheme,
			const Xapian::MatchDecider* mdecider,
			const Xapian::KeyMaker* sorter,
			Xapian::valueno collapse_key,
			Xapian::doccount collapse_max,
			int percent_threshold,
			double percent_threshold_factor,
			double weight_threshold,
			Xapian::Enquire::docid_order order,
			Xapian::valueno sort_key,
			Xapian::Enquire::Internal::sort_setting sort_by,
			bool sort_val_reverse,
			double time_limit,
			const vector<opt_ptr_spy>& matchspies)
{
    Assert(!locals.empty());

    ValueStreamDocument vsdoc(db);
    ++vsdoc._refs;
    Xapian::Document doc(&vsdoc);

    vector<PostList*> postlists;
    postlists.reserve(locals.size());
    PostListTree pltree(vsdoc, db, wtscheme);
    Xapian::termcount total_subqs = 0;
    try {
	bool all_null = true;
	for (size_t i = 0; i != locals.size(); ++i) {
	    if (!locals[i].get()) {
		postlists.push_back(NULL);
		continue;
	    }
	    // Pick the highest total subqueries answer amongst the
	    // subdatabases, as the query to postlist conversion doesn't
	    // recurse into positional queries for shards that don't have
	    // positional data when at least one other shard does.
	    Xapian::termcount total_subqs_i = 0;
	    PostList* pl = locals[i]->get_postlist(&pltree, &total_subqs_i);
	    total_subqs = max(total_subqs, total_subqs_i);
	    if (pl != NULL) {
		all_null = false;
		if (mdecider) {
		    pl = new DeciderPostList(pl, mdecider, &vsdoc, &pltree);
		}
	    }
	    postlists.push_back(pl);
	}
	Assert(!postlists.empty());

	if (all_null) {
	    vector<Result> dummy;
	    return Xapian::MSet(new Xapian::MSet::Internal(first, 0, 0, 0, 0,
							   0, 0, 0.0, 0.0,
							   std::move(dummy),
							   0));
	}
    } catch (...) {
	for (auto pl : postlists) delete pl;
	throw;
    }

    Xapian::doccount n_shards = postlists.size();
    pltree.set_postlists(&postlists[0], n_shards);

    // The highest weight a document could get in this match.
    const double max_possible = pltree.recalc_maxweight();

    if (max_possible == 0.0) {
	// All the weights are zero.
	if (sort_by == REL) {
	    // We're only sorting by DOCID.
	    sort_by = DOCID;
	} else if (sort_by == REL_VAL || sort_by == VAL_REL) {
	    // Normalise REL_VAL and VAL_REL to VAL, to avoid needlessly
	    // fetching and comparing weights.
	    sort_by = VAL;
	}
	// All percentages will be 100% so turn off any percentage cut-off.
	percent_threshold = 0;
	percent_threshold_factor = 0.0;
    }

    Xapian::doccount matches_lower_bound = pltree.get_termfreq_min();
    Xapian::doccount matches_estimated = pltree.get_termfreq_est();
    Xapian::doccount matches_upper_bound = pltree.get_termfreq_max();

    // Check if any results have been asked for (might just be wanting
    // maxweight).
    if (check_at_least == 0) {
	Xapian::doccount uncollapsed_lower_bound = matches_lower_bound;
	if (collapse_max) {
	    // Lower bound must be set to no more than collapse_max, since it's
	    // possible that all matching documents have the same collapse_key
	    // value and so are collapsed together.
	    if (matches_lower_bound > collapse_max)
		matches_lower_bound = collapse_max;
	}

	vector<Result> dummy;
	return Xapian::MSet(new Xapian::MSet::Internal(first,
						       matches_upper_bound,
						       matches_lower_bound,
						       matches_estimated,
						       matches_upper_bound,
						       uncollapsed_lower_bound,
						       matches_estimated,
						       max_possible,
						       0.0,
						       std::move(dummy),
						       0));
    }

    SpyMaster spymaster(&matchspies);

    bool sort_forward = (order != Xapian::Enquire::DESCENDING);
    auto mcmp = get_msetcmp_function(sort_by, sort_forward, sort_val_reverse);

    // Can we stop once the ProtoMSet is full?
    bool stop_once_full = (sort_forward &&
			   n_shards == 1 &&
			   sort_by == DOCID);

    ProtoMSet proto_mset(first, maxitems, check_at_least,
			 mcmp, sort_by, total_subqs,
			 pltree,
			 collapse_key, collapse_max,
			 percent_threshold, percent_threshold_factor,
			 max_possible,
			 stop_once_full,
			 time_limit);
    proto_mset.set_new_min_weight(weight_threshold);

    while (true) {
	double min_weight = proto_mset.get_min_weight();
	if (!pltree.next(min_weight)) {
	    break;
	}

	// The weight calculation can be expensive enough that it's worth being
	// lazy and only calculating it once we know we need to.  If sort_by
	// is DOCID then all weights are zero.
	double weight = 0.0;
	bool calculated_weight = (sort_by == DOCID);
	if (!calculated_weight) {
	    if (sort_by != VAL || min_weight > 0.0) {
		weight = pltree.get_weight();
		if (weight < min_weight) {
		    continue;
		}
		calculated_weight = true;
	    }
	}

	Xapian::docid did = pltree.get_docid();
	vsdoc.set_document(did);
	Result new_item(weight, did);

	if (sort_by != DOCID && sort_by != REL) {
	    if (sorter) {
		new_item.set_sort_key((*sorter)(doc));
	    } else {
		new_item.set_sort_key(vsdoc.get_value(sort_key));
	    }

	    if (proto_mset.early_reject(new_item, calculated_weight, spymaster,
					doc))
		continue;
	}

	// Apply any MatchSpy objects.
	if (spymaster) {
	    if (!calculated_weight) {
		weight = pltree.get_weight();
		new_item.set_weight(weight);
		calculated_weight = true;
	    }
	    spymaster(doc, weight);
	}

	if (!calculated_weight) {
	    weight = pltree.get_weight();
	    new_item.set_weight(weight);
	}

	if (!proto_mset.process(std::move(new_item), vsdoc))
	    break;
    }

    return proto_mset.finalise(mdecider,
			       matches_lower_bound,
			       matches_estimated,
			       matches_upper_bound);
}

Xapian::MSet
Matcher::get_mset(Xapian::doccount first,
		  Xapian::doccount maxitems,
		  Xapian::doccount check_at_least,
		  Xapian::Weight::Internal& stats,
		  const Xapian::Weight& wtscheme,
		  const Xapian::MatchDecider* mdecider,
		  const Xapian::KeyMaker* sorter,
		  Xapian::valueno collapse_key,
		  Xapian::doccount collapse_max,
		  int percent_threshold,
		  double weight_threshold,
		  Xapian::Enquire::docid_order order,
		  Xapian::valueno sort_key,
		  Xapian::Enquire::Internal::sort_setting sort_by,
		  bool sort_val_reverse,
		  double time_limit,
		  const vector<opt_intrusive_ptr<Xapian::MatchSpy>>& matchspies)
{
    AssertRel(check_at_least, >=, first + maxitems);

    Assert(!query.empty());

#ifdef XAPIAN_HAS_REMOTE_BACKEND
    if (locals.empty() && remotes.size() == 1) {
	// Short cut for a single remote database.
	Assert(remotes[0].get());
	remotes[0]->start_match(first, maxitems, check_at_least, sorter,
				stats);
	return remotes[0]->get_mset(matchspies);
    }
#endif

    // Factor to multiply maximum weight seen by to get the minimum weight we
    // need to consider.
    double percent_threshold_factor = percent_threshold / 100.0;
    // Corresponding correction to that in api/mset.cc to account for excess
    // precision on x86.
    percent_threshold_factor -= DBL_EPSILON;

#ifdef XAPIAN_HAS_REMOTE_BACKEND
    for (auto&& submatch : remotes) {
	Assert(submatch.get());
	// We need to fetch the first "first" results too, as merging may push
	// those down into the part of the merged MSet we care about.
	Xapian::doccount remote_maxitems = first + maxitems;
	if (collapse_max != 0) {
	    // If collapsing we need to fetch all check_at_least items in order
	    // to satisfy the requirement that if there are <= check_at_least
	    // results then then estimated number of matches is exact.
	    AssertRel(check_at_least, >=, first + maxitems);
	    remote_maxitems = check_at_least;
	}
	submatch->start_match(0, remote_maxitems, check_at_least, sorter,
			      stats);
    }
#endif

    Xapian::MSet local_mset;
    if (!locals.empty()) {
	for (auto&& submatch : locals) {
	    if (submatch.get())
		submatch->start_match(stats);
	}

	Xapian::doccount local_first = first;
	Xapian::doccount local_maxitems = maxitems;
	double local_percent_threshold_factor = percent_threshold_factor;
#ifdef XAPIAN_HAS_REMOTE_BACKEND
	if (!remotes.empty()) {
	    // We need to fetch the first "first" results too, as merging may
	    // push those down into the part of the merged MSet we care about.
	    local_first = 0;
	    local_maxitems = first + maxitems;
	    if (collapse_max != 0) {
		// If collapsing we need to fetch all check_at_least items in
		// order to satisfy the requirement that if there are <=
		// check_at_least results then then estimated number of matches
		// is exact.  FIXME: Can we avoid this for the local shard by
		// making use of information in the Collapser?
		AssertRel(check_at_least, >=, first + maxitems);
		local_maxitems = check_at_least;
	    }
	    local_percent_threshold_factor = 0.0;
	}
#endif
	local_mset = get_local_mset(local_first, local_maxitems, check_at_least,
				    wtscheme, mdecider,
				    sorter, collapse_key, collapse_max,
				    percent_threshold,
				    local_percent_threshold_factor,
				    weight_threshold, order, sort_key, sort_by,
				    sort_val_reverse, time_limit, matchspies);
    }

#ifdef XAPIAN_HAS_REMOTE_BACKEND
    if (remotes.empty()) {
	// Another easy case - only local databases.
	return local_mset;
    }

    // We need to merge MSet objects.  We only need the number of remote shards
    // + 1 if there are any local shards, so reserving n_shards may be more
    // than we need.
    vector<pair<Xapian::MSet, Xapian::doccount>> msets;
    Xapian::MSet merged_mset;
    if (!locals.empty()) {
	if (!local_mset.empty())
	    msets.push_back({local_mset, 0});
	merged_mset.internal->merge_stats(local_mset.internal.get(),
					  collapse_max != 0);
    }

    for_all_remotes(
	[&](RemoteSubMatch* submatch) {
	    Xapian::MSet remote_mset = submatch->get_mset(matchspies);
	    merged_mset.internal->merge_stats(remote_mset.internal.get(),
					      collapse_max != 0);
	    if (remote_mset.empty()) {
		return;
	    }
	    remote_mset.internal->unshard_docids(submatch->get_shard(),
						 db.internal->size());
	    msets.push_back({remote_mset, 0});
	});

    if (merged_mset.internal->max_possible == 0.0) {
	// All the weights are zero.
	if (sort_by == REL) {
	    // We're only sorting by DOCID.
	    sort_by = DOCID;
	} else if (sort_by == REL_VAL || sort_by == VAL_REL) {
	    // Normalise REL_VAL and VAL_REL to VAL, to avoid needlessly
	    // fetching and comparing weights.
	    sort_by = VAL;
	}
	// All percentages will be 100% so turn off any percentage cut-off.
	percent_threshold = 0;
	percent_threshold_factor = 0.0;
    }

    bool sort_forward = (order != Xapian::Enquire::DESCENDING);
    auto mcmp = get_msetcmp_function(sort_by, sort_forward, sort_val_reverse);
    auto heap_cmp =
	[&](const pair<Xapian::MSet, Xapian::doccount>& a,
	    const pair<Xapian::MSet, Xapian::doccount>& b) {
	    return mcmp(b.first.internal->items[b.second],
			a.first.internal->items[a.second]);
	};

    Heap::make(msets.begin(), msets.end(), heap_cmp);

    double min_weight = 0.0;
    if (percent_threshold) {
	min_weight = percent_threshold_factor * 100.0 /
		     merged_mset.internal->percent_scale_factor;
    }

    CollapserLite collapser(collapse_max);
    merged_mset.internal->first = first;
    while (!msets.empty() && merged_mset.size() != maxitems) {
	auto& front = msets.front();
	auto& result = front.first.internal->items[front.second];
	if (percent_threshold) {
	    if (result.get_weight() < min_weight) {
		// FIXME: This will need adjusting if we ever support
		// percentage thresholds when sorting primarily by value.
		break;
	    }
	}
	if (!collapser || collapser.add(result.get_collapse_key())) {
	    if (first) {
		// Skip the first "first" results from the merge - we had to
		// also fetch the first "first" results from each shard, as
		// merging may push those down into the part of the merged MSet
		// we care about.
		--first;
	    } else {
		merged_mset.internal->items.push_back(std::move(result));
	    }
	}
	auto n = front.second + 1;
	if (n == front.first.size()) {
	    Heap::pop(msets.begin(), msets.end(), heap_cmp);
	    msets.resize(msets.size() - 1);
	} else {
	    front.second = n;
	    Heap::replace(msets.begin(), msets.end(), heap_cmp);
	}
    }

    if (collapser) {
	auto todo = check_at_least - maxitems;
	if (merged_mset.size() != maxitems) {
	    todo = 0;
	}
	while (!msets.empty() && todo--) {
	    auto& front = msets.front();
	    auto& result = front.first.internal->items[front.second];
	    if (percent_threshold) {
		if (result.get_weight() < min_weight) {
		    // FIXME: This will need adjusting if we ever support
		    // percentage thresholds when sorting primarily by value.
		    break;
		}
	    }
	    (void)collapser.add(result.get_collapse_key());
	    auto n = front.second + 1;
	    if (n == front.first.size()) {
		Heap::pop(msets.begin(), msets.end(), heap_cmp);
		msets.resize(msets.size() - 1);
	    } else {
		front.second = n;
		Heap::replace(msets.begin(), msets.end(), heap_cmp);
	    }
	}

	auto mseti = merged_mset.internal;
	collapser.finalise(mseti->items, percent_threshold);

	if (check_at_least > 0) {
	    // Each input MSet object to the merge has already been collapsed
	    // and merge_stats() above will have set mset->matches_lower_bound
	    // to the maximum matches_lower_bound of any input, which provides
	    // a lower bound.
	    //
	    // In some cases, the collapser can provide a better lower bound.
	    auto collapser_lb = collapser.get_matches_lower_bound();
	    if (mseti->matches_upper_bound <= check_at_least) {
		mseti->matches_lower_bound = collapser_lb;
		mseti->matches_estimated = collapser_lb;
		mseti->matches_upper_bound = collapser_lb;
		return merged_mset;
	    }

	    mseti->matches_lower_bound = max(mseti->matches_lower_bound,
					     collapser_lb);
	}

	double unique_rate = 1.0;

	Xapian::doccount docs_considered = collapser.get_docs_considered();
	Xapian::doccount dups_ignored = collapser.get_dups_ignored();
	if (docs_considered > 0) {
	    // Scale the estimate by the rate at which we've been finding
	    // unique documents while merging MSet objects.
	    double unique = double(docs_considered - dups_ignored);
	    unique_rate = unique / double(docs_considered);
	}

	// We can safely reduce the upper bound by the number of duplicates
	// we've seen while merging MSet objects.
	mseti->matches_upper_bound -= collapser.get_dups_ignored();

	double estimate_scale = unique_rate;

	if (estimate_scale != 1.0) {
	    auto l = mseti->matches_lower_bound;
	    auto u = mseti->matches_upper_bound;
	    auto e = l + Xapian::doccount((u - l) * estimate_scale + 0.5);
	    mseti->matches_estimated = e;
	}

	// Clamp the estimate the range given by the bounds.
	AssertRel(mseti->matches_lower_bound, <=, mseti->matches_upper_bound);
	mseti->matches_estimated = STD_CLAMP(mseti->matches_estimated,
					     mseti->matches_lower_bound,
					     mseti->matches_upper_bound);
    }

    return merged_mset;
#else
    return local_mset;
#endif
}
