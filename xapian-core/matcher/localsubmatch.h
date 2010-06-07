/** @file localmatch.h
 *  @brief SubMatch class for a local database.
 */
/* Copyright (C) 2006,2007,2009,2010 Olly Betts
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

#ifndef XAPIAN_INCLUDED_LOCALMATCH_H
#define XAPIAN_INCLUDED_LOCALMATCH_H

#include "database.h"
#include "omqueryinternal.h"
#include "submatch.h"
#include "xapian/enquire.h"
#include "xapian/weight.h"

#include <map>

class LocalSubMatch : public SubMatch {
    /// Don't allow assignment.
    void operator=(const LocalSubMatch &);

    /// Don't allow copying.
    LocalSubMatch(const LocalSubMatch &);

    /// The statistics for the collection.
    const Xapian::Weight::Internal * stats;

    /// The original query before any rearrangement.
    Xapian::Query::Internal orig_query;

    /// The query length (used by some weighting schemes).
    Xapian::termcount qlen;

    /// The (sub-)Database we're searching.
    const Xapian::Database::Internal *db;

    /** The RSet (used to calculate R and r).
     *
     *  R and r are used in probabilistic weighting formulae.
     */
    Xapian::RSet omrset;

    /// Weight object (used as a factory by calling create on it).
    const Xapian::Weight * wt_factory;

    /// The termfreqs and weights of terms used in orig_query, or NULL.
    std::map<string, Xapian::MSet::Internal::TermFreqAndWeight> * term_info;

  public:
    /// Constructor.
    LocalSubMatch(const Xapian::Database::Internal *db,
		  const Xapian::Query::Internal * query,
		  Xapian::termcount qlen,
		  const Xapian::RSet & omrset_,
		  const Xapian::Weight *wt_factory);

    /// Fetch and collate statistics.
    bool prepare_match(bool nowait, Xapian::Weight::Internal & total_stats);

    /// Start the match.
    void start_match(Xapian::doccount first,
		     Xapian::doccount maxitems,
		     Xapian::doccount check_at_least,
		     const Xapian::Weight::Internal & total_stats);

    /// Get PostList and term info.
    PostList * get_postlist_and_term_info(MultiMatch *matcher,
	std::map<string, Xapian::MSet::Internal::TermFreqAndWeight> *termfreqandwts,
	Xapian::termcount * total_subqs_ptr);

    /** Convert a postlist into a synonym postlist.
     */
    PostList * make_synonym_postlist(PostList * or_pl, MultiMatch * matcher,
				     double factor);

    /** Convert an OP_LEAF query to a PostList.
     *
     *  This is called by QueryOptimiser when it reaches an OP_LEAF query.
     */
    PostList * postlist_from_op_leaf_query(const Xapian::Query::Internal *query,
					   double factor);
};

#endif /* XAPIAN_INCLUDED_LOCALMATCH_H */
