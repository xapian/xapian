/** @file
 *  @brief SubMatch class for a local database.
 */
/* Copyright (C) 2006-2022 Olly Betts
 * Copyright (C) 2007 Lemur Consulting Ltd
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

#ifndef XAPIAN_INCLUDED_LOCALSUBMATCH_H
#define XAPIAN_INCLUDED_LOCALSUBMATCH_H

#include "api/queryinternal.h"
#include "backends/databaseinternal.h"
#include "backends/leafpostlist.h"
#include "estimateop.h"
#include "weight/weightinternal.h"
#include "xapian/enquire.h"
#include "xapian/weight.h"

class PostListTree;

namespace Xapian {
namespace Internal {
class PostList;
}
}

using Xapian::Internal::PostList;

class LocalSubMatch {
    /// Don't allow assignment.
    LocalSubMatch& operator=(const LocalSubMatch &) = delete;

    /// Don't allow copying.
    LocalSubMatch(const LocalSubMatch &) = delete;

    /// The statistics for the collection.
    Xapian::Weight::Internal* total_stats = nullptr;

    /// The query.
    Xapian::Query query;

    /// The query length (used by some weighting schemes).
    Xapian::termcount qlen;

    /// The (sub-)Database we're searching.
    const Xapian::Database::Internal* db;

    /// Weight object (used as a factory by calling create on it).
    const Xapian::Weight& wt_factory;

    /// 0-based index for the subdatabase.
    Xapian::doccount shard_index;

    /** Stack of operations to calculate an Estimates object for this shard.
     *
     *  This allows the estimate to be calculated at the end of the match so
     *  that it can incorporate information about things such as how many
     *  documents were accepted and rejected by positional checks.
     *
     *  The stack is a forward-linked list.
     */
    EstimateOp* estimate_stack = nullptr;

    /** Orphaned EstimateOp objects.
     *
     *  These shouldn't get stats reported to them, but code bugs could
     *  mean they do so be robust and avoid use-after-free in this
     *  situation.
     *
     *  This is a forward-linked list.
     */
    EstimateOp* orphans = nullptr;

  public:
    /// Constructor.
    LocalSubMatch(const Xapian::Database::Internal* db_,
		  const Xapian::Query& query_,
		  Xapian::termcount qlen_,
		  const Xapian::Weight& wt_factory_,
		  Xapian::doccount shard_index_)
	: query(query_), qlen(qlen_), db(db_),
	  wt_factory(wt_factory_),
	  shard_index(shard_index_)
    {}

    ~LocalSubMatch() {
	EstimateOp* p = estimate_stack;
	while (p) {
	    EstimateOp* next = p->get_next();
	    delete p;
	    p = next;
	}
	p = orphans;
	while (p) {
	    EstimateOp* next = p->get_next();
	    delete p;
	    p = next;
	}
    }

    template<typename... Args>
    EstimateOp* add_op(Args... args) {
	estimate_stack = new EstimateOp(estimate_stack, args...);
	return estimate_stack;
    }

    void pop_op() {
	EstimateOp* first = estimate_stack;
	unsigned elements_to_pop = 1;
	EstimateOp* last = first;
	do {
	    // We may need to pop subqueries (recursively!)
	    elements_to_pop += estimate_stack->get_subquery_count();
	    last = estimate_stack;
	    estimate_stack = estimate_stack->get_next();
	} while (--elements_to_pop);
	last->set_next(orphans);
	orphans = first;
    }

    Estimates resolve() {
	Estimates result;
	if (rare(!estimate_stack)) {
	    result = Estimates(0, 0, 0);
	    return result;
	}
	auto db_size = db->get_doccount();
	// We shortcut an empty shard and avoid creating a postlist tree for
	// it so the estimate stack should be empty.
	Assert(db_size);
	Xapian::docid db_first, db_last;
	db->get_used_docid_range(db_first, db_last);
	result = estimate_stack->resolve(db_size, db_first, db_last);
	// After resolve(), estimate_stack should contain exactly one entry.
	// If not, that probably suggests something went wrong while building
	// it, or perhaps while resolving it.
	Assert(estimate_stack);
	Assert(!estimate_stack->get_next());
	return result;
    }

    /** Fetch and collate statistics.
     *
     *  Before we can calculate term weights we need to fetch statistics from
     *  each database involved and collate them.
     *
     *  @param rset	The RSet for this shard.
     *  @param stats	Weight::Internal object to add the statistics to.
     */
    void prepare_match(const Xapian::RSet& rset,
		       Xapian::Weight::Internal& stats)
    {
	stats.accumulate_stats(*db, rset);
    }

    /** Set the collated statistics.
     *
     *  These will be used when generating the PostList tree.
     */
    void start_match(Xapian::Weight::Internal& total_stats_)
    {
	total_stats = &total_stats_;
    }

    /// Get PostList.
    PostList* get_postlist(PostListTree* matcher,
			   Xapian::termcount* total_subqs_ptr);

    /** Convert a postlist into a synonym postlist.
     */
    PostList* make_synonym_postlist(PostListTree* pltree,
				    PostList* or_pl,
				    double factor,
				    const TermFreqs& termfreqs);

    LeafPostList* open_post_list(const std::string& term,
				 Xapian::termcount wqf,
				 double factor,
				 bool need_positions,
				 bool compound_weight,
				 Xapian::Internal::QueryOptimiser* qopt,
				 bool lazy_weight,
				 TermFreqs* termfreqs);

    void register_lazy_postlist_for_stats(LeafPostList* pl,
					  TermFreqs* termfreqs) {
	auto res = total_stats->termfreqs.emplace(pl->get_term(), TermFreqs());
	if (res.second) {
	    // Only register if the term isn't already registered - e.g. a term
	    // from a wildcard expansion which is also present in the query
	    // verbatim such as: foo* food
	    res.first->second.termfreq = pl->get_termfreq();
	    res.first->second.collfreq = pl->get_collfreq();
#ifdef XAPIAN_ASSERTIONS
	    Xapian::doccount tf;
	    Xapian::termcount cf;
	    db->get_freqs(pl->get_term(), &tf, &cf);
	    AssertEq(res.first->second.termfreq, tf);
	    AssertEq(res.first->second.collfreq, cf);
#endif
	}
	if (termfreqs) *termfreqs = res.first->second;
    }

    bool weight_needs_wdf() const {
	return wt_factory.get_sumpart_needs_wdf_();
    }

    const Xapian::Weight::Internal* get_stats() const {
	return total_stats;
    }
};

#endif /* XAPIAN_INCLUDED_LOCALSUBMATCH_H */
