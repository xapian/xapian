/** @file queryoptimiser.h
 * @brief Details passed around while building PostList tree from Query tree
 */
/* Copyright (C) 2007,2008,2009,2010,2011,2013,2014,2015 Olly Betts
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
namespace Xapian {
class Weight;
}

class QueryOptimiser {
    /// Prevent assignment.
    void operator=(const QueryOptimiser &);

    /// Prevent copying.
    QueryOptimiser(const QueryOptimiser &);

    LocalSubMatch & localsubmatch;

    /** How many leaf subqueries there are.
     *
     *  Used for scaling percentages when the highest weighted document doesn't
     *  "match all terms".
     */
    Xapian::termcount total_subqs;

    LeafPostList * hint;

  public:
    bool need_positions;

    const Xapian::Database::Internal & db;

    Xapian::doccount db_size;

    MultiMatch * matcher;

    QueryOptimiser(const Xapian::Database::Internal & db_,
		   LocalSubMatch & localsubmatch_,
		   MultiMatch * matcher_)
	: localsubmatch(localsubmatch_), total_subqs(0), hint(0),
	  need_positions(false), db(db_), db_size(db.get_doccount()),
	  matcher(matcher_) { }

    void inc_total_subqs() { ++total_subqs; }

    Xapian::termcount get_total_subqs() const { return total_subqs; }

    void set_total_subqs(Xapian::termcount n) { total_subqs = n; }

    LeafPostList * open_post_list(const std::string& term,
				  Xapian::termcount wqf,
				  double factor) {
	return localsubmatch.open_post_list(term, wqf, factor, need_positions,
					    &hint, false);
    }

    LeafPostList * open_lazy_post_list(const std::string& term,
				       Xapian::termcount wqf,
				       double factor) {
	return localsubmatch.open_post_list(term, wqf, factor, false, &hint, true);
    }

    PostList * make_synonym_postlist(PostList * pl, double factor) {
	return localsubmatch.make_synonym_postlist(pl, matcher, factor);
    }
};

#endif // XAPIAN_INCLUDED_QUERYOPTIMISER_H
