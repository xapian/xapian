/** @file localsubmatch.h
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

#ifndef XAPIAN_INCLUDED_LOCALSUBMATCH_H
#define XAPIAN_INCLUDED_LOCALSUBMATCH_H

#include "database.h"
#include "debuglog.h"
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
    const Xapian::Query::Internal * query;

    /// The query length (used by some weighting schemes).
    Xapian::termcount qlen;

    /// The (sub-)Database we're searching.
    const Xapian::Database::Internal *db;

    /** The RSet (used to calculate R and r).
     *
     *  R and r are used in probabilistic weighting formulae.
     */
    Xapian::RSet rset;

    /// Weight object (used as a factory by calling create on it).
    const Xapian::Weight * wt_factory;

    /// The termfreqs and weights of terms used in orig_query, or NULL.
    std::map<std::string,
	     Xapian::MSet::Internal::TermFreqAndWeight> * term_info;

  public:
    /// Constructor.
    LocalSubMatch(const Xapian::Database::Internal *db_,
		  const Xapian::Query::Internal * query_,
		  Xapian::termcount qlen_,
		  const Xapian::RSet & rset_,
		  const Xapian::Weight *wt_factory_)
	: stats(NULL), query(query_), qlen(qlen_), db(db_), rset(rset_),
	  wt_factory(wt_factory_), term_info(NULL)
    {
	LOGCALL_CTOR(MATCH, "LocalSubMatch", db_ | query_ | qlen_ | rset_ | wt_factory_);
    }

    /// Fetch and collate statistics.
    bool prepare_match(bool nowait, Xapian::Weight::Internal & total_stats);

    /// Start the match.
    void start_match(Xapian::doccount first,
		     Xapian::doccount maxitems,
		     Xapian::doccount check_at_least,
		     const Xapian::Weight::Internal & total_stats);

    /// Get PostList and term info.
    PostList * get_postlist_and_term_info(MultiMatch *matcher,
	std::map<std::string,
		 Xapian::MSet::Internal::TermFreqAndWeight> *termfreqandwts,
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

#endif /* XAPIAN_INCLUDED_LOCALSUBMATCH_H */
