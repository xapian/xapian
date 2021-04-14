/** @file
 * @brief Details passed around while building PostList tree from Query tree
 */
/* Copyright (C) 2007,2008,2009,2010,2011,2013,2014,2015,2016,2018 Olly Betts
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

#include "backends/database.h"
#include "localsubmatch.h"
#include "api/postlist.h"

class LeafPostList;
class MultiMatch;

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
    Xapian::termcount total_subqs;

    LeafPostList * hint;

    bool hint_owned;

  public:
    bool need_positions;

    bool in_synonym;

    Xapian::doccount shard_index;

    const Xapian::Database::Internal & db;

    Xapian::doccount db_size;

    MultiMatch * matcher;

    QueryOptimiser(const Xapian::Database::Internal & db_,
		   LocalSubMatch & localsubmatch_,
		   MultiMatch* matcher_,
		   Xapian::doccount shard_index_)
	: localsubmatch(localsubmatch_), total_subqs(0),
	  hint(0), hint_owned(false),
	  need_positions(false), in_synonym(false),
	  shard_index(shard_index_),
	  db(db_), db_size(db.get_doccount()),
	  matcher(matcher_) { }

    ~QueryOptimiser() {
	if (hint_owned) delete hint;
    }

    bool full_db_has_positions() const {
	return matcher->full_db_has_positions();
    }

    void inc_total_subqs() { ++total_subqs; }

    Xapian::termcount get_total_subqs() const { return total_subqs; }

    void set_total_subqs(Xapian::termcount n) { total_subqs = n; }

    LeafPostList * open_post_list(const std::string& term,
				  Xapian::termcount wqf,
				  double factor) {
	return localsubmatch.open_post_list(term, wqf, factor, need_positions,
					    in_synonym, this, false);
    }

    LeafPostList * open_lazy_post_list(const std::string& term,
				       Xapian::termcount wqf,
				       double factor) {
	return localsubmatch.open_post_list(term, wqf, factor, false,
					    in_synonym, this, true);
    }

    PostList * make_synonym_postlist(PostList * pl,
				     double factor,
				     bool wdf_disjoint) {
	return localsubmatch.make_synonym_postlist(pl, matcher, factor,
						   wdf_disjoint);
    }

    const LeafPostList * get_hint_postlist() const { return hint; }

    void set_hint_postlist(LeafPostList * new_hint) {
	if (hint_owned) {
	    hint_owned = false;
	    delete hint;
	}
	hint = new_hint;
    }

    void destroy_postlist(PostList* pl) {
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
};

#endif // XAPIAN_INCLUDED_QUERYOPTIMISER_H
