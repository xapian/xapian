/** @file queryoptimiser.h
 * @brief Convert a Xapian::Query::Internal tree into an optimal PostList tree.
 */
/* Copyright (C) 2007,2008,2009,2010 Olly Betts
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

#include "xapian/query.h"

#include "database.h"
#include "localsubmatch.h"
#include "omenquireinternal.h"
#include "postlist.h"

#include <list>
#include <vector>

class MultiMatch;
struct PosFilter;

class QueryOptimiser {
    const Xapian::Database::Internal & db;

    Xapian::doccount db_size;

    LocalSubMatch & localsubmatch;

    MultiMatch * matcher;

    /** How many leaf subqueries there are.
     *
     *  Used for scaling percentages when the highest weighted document doesn't
     *  "match all terms".
     */
    Xapian::termcount total_subqs;

    /** Optimise a Xapian::Query::Internal subtree into a PostList subtree.
     *
     *  @param query	The subtree to optimise.
     *  @param factor	How much to scale weights for this subtree by.
     *
     *  @return		A PostList subtree.
     */
    PostList * do_subquery(const Xapian::Query::Internal * query,
			   double factor);

    /** Optimise an AND-like Xapian::Query::Internal subtree into a PostList
     *  subtree.
     *
     *  @param query	The subtree to optimise.
     *  @param factor	How much to scale weights for this subtree by.
     *
     *  @return		A PostList subtree.
     */
    PostList * do_and_like(const Xapian::Query::Internal *query, double factor);

    /** Optimise an AND-like Xapian::Query::Internal subtree into a PostList
     *  subtree.
     *
     *  @param query	    The subtree to optimise.
     *  @param factor	    How much to scale weights for this subtree by.
     *  @param and_plists   Append new PostList subtrees to be combined with
     *			    AND to this vector.
     *  @param pos_filters  Append any positional filters to be applied to the
     *                      tree to this list.
     */
    void do_and_like(const Xapian::Query::Internal *query, double factor,
		     std::vector<PostList *> & and_plists,
		     std::list<PosFilter> & pos_filters);

    /** Optimise an OR-like Xapian::Query::Internal subtree into a PostList
     *  subtree.
     *
     *  @param query	The subtree to optimise.
     *  @param factor	How much to scale weights for this subtree by.
     *
     *  @return		A PostList subtree.
     */
    PostList * do_or_like(const Xapian::Query::Internal *query, double factor);

    /** Optimise a synonym Xapian::Query::Internal subtree into a PostList
     *
     *  @param query	The subtree to optimise.
     *  @param factor	How much to scale weights for this subtree by.
     *
     *  @return		A PostList subtree.
     */
    PostList * do_synonym(const Xapian::Query::Internal *query, double factor);

  public:
    QueryOptimiser(const Xapian::Database::Internal & db_,
		   LocalSubMatch & localsubmatch_,
		   MultiMatch * matcher_)
	: db(db_), db_size(db.get_doccount()), localsubmatch(localsubmatch_),
	  matcher(matcher_), total_subqs(0) { }

    PostList * optimise_query(const Xapian::Query::Internal * query) {
	return do_subquery(query, 1.0);
    }

    Xapian::termcount get_total_subqueries() const { return total_subqs; }
};

#endif // XAPIAN_INCLUDED_QUERYOPTIMISER_H
