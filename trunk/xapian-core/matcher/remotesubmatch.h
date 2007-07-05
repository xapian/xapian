/** @file remotesubmatch.h
 *  @brief SubMatch class for a remote database.
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

#ifndef XAPIAN_INCLUDED_REMOTESUBMATCH_H
#define XAPIAN_INCLUDED_REMOTESUBMATCH_H

#include "submatch.h"
#include "remote-database.h"
#include "networkstats.h"
#include "stats.h"

/// Class for performing matching on a remote database.
class RemoteSubMatch : public SubMatch {
    /// Don't allow assignment.
    void operator=(const RemoteSubMatch &);

    /// Don't allow copying.
    RemoteSubMatch(const RemoteSubMatch &);

    /// The remote database.
    RemoteDatabase *db;

    /// The StatsGatherer object, used to access database statistics.
    StatsGatherer * gatherer;

    /// The NetworkStatsSource object handles statistics for the remote db.
    NetworkStatsSource stats_source;

    /** Is the sort order such the relevance decreases down the MSet?
     *
     *  This is true for sort_by_relevance and sort_by_relevance_then_value.
     */
    bool decreasing_relevance;

  public:
    /// Constructor.
    RemoteSubMatch(RemoteDatabase *db_, StatsGatherer *gatherer_, bool decreasing_relevance_)
	: db(db_), gatherer(gatherer_), stats_source(gatherer_, db_),
	  decreasing_relevance(decreasing_relevance_)
    {
    }

    /// Fetch and collate statistics.
    bool prepare_match(bool nowait);

    /// Start the match.
    void start_match(Xapian::doccount maxitems,
		     Xapian::doccount check_at_least);

    /// Get PostList and term info.
    PostList * get_postlist_and_term_info(MultiMatch *matcher,
	map<string, Xapian::MSet::Internal::TermFreqAndWeight> *termfreqandwts);
};

#endif /* XAPIAN_INCLUDED_REMOTESUBMATCH_H */
