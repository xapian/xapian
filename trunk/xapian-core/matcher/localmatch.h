/** @file localmatch.h
 *  @brief SubMatch class for a local database.
 */
/* Copyright (C) 2006,2007 Olly Betts
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

#ifndef XAPIAN_INCLUDED_LOCALMATCH_H
#define XAPIAN_INCLUDED_LOCALMATCH_H

#include "database.h"
#include "omqueryinternal.h"
#include "rset.h"
#include "stats.h"
#include "submatch.h"

namespace Xapian { class Weight; }

#include <map>
#include <vector>

class LocalSubMatch : public SubMatch {
    /// Don't allow assignment.
    void operator=(const LocalSubMatch &);

    /// Don't allow copying.
    LocalSubMatch(const LocalSubMatch &);

    /// Our StatsSource.
    LocalStatsSource statssource;

    /// The original query before any rearrangement.
    Xapian::Query::Internal orig_query;

    /// The query length (used by some weighting schemes).
    Xapian::termcount qlen;

    /// The (sub-)Database we're searching.
    const Xapian::Database::Internal *db;

    /** The (sub-)RSet (used to calculate R and r).
     *
     *  R and r are used in probabilistic weighting formulae.
     */
    RSetI rset;

    /// Weight object (used as a factory by calling create on it).
    const Xapian::Weight * wt_factory;

    /// The termfreqs and weights of terms used in orig_query.
    std::map<string, Xapian::MSet::Internal::TermFreqAndWeight> term_info;


    /// Build an optimised AndPostList tree from a vector of PostLists.
    PostList * build_and_tree(std::vector<PostList *> &PostLists,
			      MultiMatch *matcher);

    /// Build an optimised OrPostList tree from a vector of PostLists.
    PostList * build_or_tree(std::vector<PostList *> &PostLists,
			     MultiMatch *matcher);

    /// Build an optimised XorPostList tree from a vector of PostLists.
    PostList * build_xor_tree(std::vector<PostList *> &postlists,
			      MultiMatch *matcher);

    /** Convert the sub-queries of a Query into an optimised PostList tree.
     *
     *  We take the sub-queries from @a query, but use @op instead of
     *  query->op (this allows us to convert OP_PHRASE and OP_NEAR to OP_AND
     *  if there's no positional information.
     *
     *  @param op	@a op must be either OP_AND, OP_OR, OP_XOR,
     *			OP_PHRASE, OP_NEAR, or OP_ELITE_SET.
     *  @param query	Pointer to the query to process.
     *  @param matcher	Pointer to the matcher.
     *  @param is_bool	Is this a boolean part of the query?  E.g. the right
     *			branch of an AND_NOT or FILTER.
     *  @return		The root of the PostList tree.
     */
    PostList *postlist_from_queries(Xapian::Query::Internal::op_t op,
				    const Xapian::Query::Internal *query,
				    MultiMatch *matcher, bool is_bool);

    /** Convert a Query into an optimised PostList tree.
     *
     *  @param query	Pointer to the query to process.
     *  @param matcher	Pointer to the matcher.
     *  @param is_bool	Is this a boolean part of the query?  E.g. the right
     *			branch of an AND_NOT or FILTER.
     *  @return		The root of the PostList tree.
     */
    PostList *postlist_from_query(const Xapian::Query::Internal * query,
				  MultiMatch *matcher, bool is_bool);

    /// Register term @a tname with the StatsSource.
    void register_term(const string &tname);

  public:
    /// Constructor.
    LocalSubMatch(const Xapian::Database::Internal *db,
		  const Xapian::Query::Internal * query,
		  Xapian::termcount qlen,
		  const Xapian::RSet & omrset,
		  StatsGatherer *gatherer,
		  const Xapian::Weight *wt_factory);

    /// Fetch and collate statistics.
    bool prepare_match(bool nowait);

    /// Start the match.
    void start_match(Xapian::doccount maxitems,
		     Xapian::doccount check_at_least);

    /// Get PostList and term info.
    PostList * get_postlist_and_term_info(MultiMatch *matcher,
	std::map<string, Xapian::MSet::Internal::TermFreqAndWeight> *termfreqandwts);
};

#endif /* XAPIAN_INCLUDED_LOCALMATCH_H */
