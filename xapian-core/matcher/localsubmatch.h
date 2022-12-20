/** @file
 *  @brief SubMatch class for a local database.
 */
/* Copyright (C) 2006,2007,2009,2010,2011,2013,2014,2015,2016,2018 Olly Betts
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

#include "backends/database.h"
#include "debuglog.h"
#include "api/leafpostlist.h"
#include "api/queryinternal.h"
#include "submatch.h"
#include "xapian/enquire.h"
#include "xapian/weight.h"

class LeafPostList;

class LocalSubMatch : public SubMatch {
    /// Don't allow assignment.
    void operator=(const LocalSubMatch &);

    /// Don't allow copying.
    LocalSubMatch(const LocalSubMatch &);

    /// The statistics for the collection.
    Xapian::Weight::Internal * stats;

    /// The query.
    Xapian::Query query;

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

    /// 0-based index for the subdatabase.
    Xapian::doccount shard_index;

  public:
    /// Constructor.
    LocalSubMatch(const Xapian::Database::Internal *db_,
		  const Xapian::Query & query_,
		  Xapian::termcount qlen_,
		  const Xapian::RSet & rset_,
		  const Xapian::Weight* wt_factory_,
		  Xapian::doccount shard_index_)
	: stats(NULL), query(query_), qlen(qlen_), db(db_), rset(rset_),
	  wt_factory(wt_factory_),
	  shard_index(shard_index_)
    {
	LOGCALL_CTOR(MATCH, "LocalSubMatch", db_ | query_ | qlen_ | rset_ | wt_factory_);
    }

    /// Fetch and collate statistics.
    bool prepare_match(bool nowait, Xapian::Weight::Internal & total_stats);

    /// Start the match.
    void start_match(Xapian::doccount first,
		     Xapian::doccount maxitems,
		     Xapian::doccount check_at_least,
		     Xapian::Weight::Internal & total_stats);

    /// Get PostList.
    PostList * get_postlist(MultiMatch *matcher,
			    Xapian::termcount* total_subqs_ptr,
			    Xapian::Weight::Internal& total_stats);

    /** Convert a postlist into a synonym postlist.
     */
    PostList * make_synonym_postlist(PostList * or_pl, MultiMatch * matcher,
				     double factor,
				     bool wdf_disjoint);

    LeafPostList * open_post_list(const std::string& term,
				  Xapian::termcount wqf,
				  double factor,
				  bool need_positions,
				  bool in_synonym,
				  QueryOptimiser * qopt,
				  bool lazy_weight);
};

#endif /* XAPIAN_INCLUDED_LOCALSUBMATCH_H */
