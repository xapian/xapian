/** @file
 * @brief Details passed around while building PostList tree from Query tree
 */
/* Copyright (C) 2007-2026 Olly Betts
 * Copyright (C) 2008 Lemur Consulting Ltd
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_QUERYOPTIMISER_H
#define XAPIAN_INCLUDED_QUERYOPTIMISER_H

#include "backends/databaseinternal.h"
#include "backends/leafpostlist.h"
#include "backends/postlist.h"
#include "localsubmatch.h"

class LeafPostList;
class PostListTree;

namespace Xapian {
namespace Internal {

class QueryOptimiser {
    /// Prevent assignment.
    void operator=(const QueryOptimiser &);

    /// Prevent copying.
    QueryOptimiser(const QueryOptimiser &);

    LocalSubMatch & localsubmatch;

    /** How many weighted leaf subqueries there are.
     *
     *  Used for scaling percentages when the highest weighted document doesn't
     *  "match all terms".
     */
    Xapian::termcount total_subqs = 0;

    LeafPostList* hint = nullptr;

    bool hint_owned = false;

    bool no_estimates = false;

  public:
    bool need_positions = false;

    bool compound_weight = false;

    Xapian::doccount shard_index;

    const Xapian::Database::Internal & db;

    Xapian::doccount db_size;

    PostListTree * matcher;

    QueryOptimiser(const Xapian::Database::Internal & db_,
		   LocalSubMatch & localsubmatch_,
		   PostListTree * matcher_,
		   Xapian::doccount shard_index_)
	: localsubmatch(localsubmatch_),
	  shard_index(shard_index_),
	  db(db_), db_size(db.get_doccount()),
	  matcher(matcher_) { }

    ~QueryOptimiser() {
	if (hint_owned) delete hint;
    }

    void inc_total_subqs() { ++total_subqs; }

    Xapian::termcount get_total_subqs() const { return total_subqs; }

    void set_total_subqs(Xapian::termcount n) { total_subqs = n; }

    bool get_no_estimates() const { return no_estimates; }

    void set_no_estimates(bool f) { no_estimates = f; }

    /** Create a PostList object for @a term.
     *
     *  @param[out] termfreqs If not NULL, the pointed to object is set to the
     *		TermFreqs for @a term.  If the database is sharded, these will
     *		be for the whole database not just the current shard.  This
     *		is used to estimate TermFreqs for an OP_SYNONYM.
     */
    PostListAndEstimate
    open_post_list(const std::string& term,
		   Xapian::termcount wqf,
		   double factor,
		   TermFreqs* termfreqs) {
	return localsubmatch.open_post_list(term, wqf, factor, need_positions,
					    compound_weight, this, false,
					    termfreqs);
    }

    PostListAndEstimate
    open_lazy_post_list(const std::string& term,
			Xapian::termcount wqf,
			double factor) {
	return localsubmatch.open_post_list(term, wqf, factor, need_positions,
					    compound_weight, this, true,
					    NULL);
    }

    /** Register a lazily-created LeafPostList for stats.
     *
     *  @param pl   An object previously returned by open_lazy_post_list().
     *  @param[out] termfreqs If not NULL, the pointed to object is set to the
     *		TermFreqs for @a term.  If the database is sharded, these will
     *		be for the whole database not just the current shard.  This
     *		is used to estimate TermFreqs for an OP_SYNONYM.
     */
    void register_lazy_postlist_for_stats(LeafPostList* pl,
					  TermFreqs* termfreqs) {
	localsubmatch.register_lazy_postlist_for_stats(pl, termfreqs);
    }

    /** Create a SynonymPostList object.
     *
     *  @param or_pl	    An unweighted BoolOrPostList or OrPostList of the
     *			    PostList objects for children of the OP_SYNONYM.
     *  @param termfreqs    Estimated TermFreqs for @a or_pl.
     */
    PostListAndEstimate make_synonym_postlist(PostListAndEstimate or_pl,
					      double factor,
					      const TermFreqs& termfreqs) {
	return localsubmatch.make_synonym_postlist(matcher, std::move(or_pl),
						   factor, termfreqs);
    }

    const LeafPostList * get_hint_postlist() const { return hint; }

    void set_hint_postlist(LeafPostList * new_hint) {
	if (hint_owned) {
	    hint_owned = false;
	    delete hint;
	}
	hint = new_hint;
    }

    void own_hint_postlist() { hint_owned = true; }

    void destroy_postlist(PostList* pl) {
	if (!pl) return;
	if (pl == static_cast<PostList*>(hint)) {
	    hint_owned = true;
	} else {
	    if (!hint_owned) {
		// The hint could be a subpostlist of pl, but we can't easily
		// tell so we have to do the safe thing and reset it.
		//
		// This isn't ideal, but it's much better than use-after-free
		// bugs.
		hint = nullptr;
	    }
	    delete pl;
	}
    }

    bool need_wdf_for_compound_weight() const {
	return compound_weight && !localsubmatch.weight_needs_wdf();
    }

    const Xapian::Weight::Internal* get_stats() const {
	return localsubmatch.get_stats();
    }
};

}
}

using Xapian::Internal::QueryOptimiser;

#endif // XAPIAN_INCLUDED_QUERYOPTIMISER_H
