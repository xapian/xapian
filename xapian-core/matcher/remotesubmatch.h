/** @file
 *  @brief SubMatch class for a remote database.
 */
/* Copyright (C) 2006,2007,2009,2011,2014,2015 Olly Betts
 * Copyright (C) 2007,2008 Lemur Consulting Ltd
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
#include "backends/remote/remote-database.h"
#include "xapian/weight.h"

namespace Xapian {
    class MatchSpy;
}

/// Class for performing matching on a remote database.
class RemoteSubMatch : public SubMatch {
    /// Don't allow assignment.
    void operator=(const RemoteSubMatch &);

    /// Don't allow copying.
    RemoteSubMatch(const RemoteSubMatch &);

    /// The remote database.
    RemoteDatabase *db;

    /** Is the sort order such the relevance decreases down the MSet?
     *
     *  This is true for sort_by_relevance and sort_by_relevance_then_value.
     */
    bool decreasing_relevance;

    /// uncollapsed_upper_bound from the remote MSet.
    Xapian::doccount uncollapsed_upper_bound;

    /// The factor to use to convert weights to percentages.
    double percent_factor;

    /// The matchspies to use.
    const vector<Xapian::Internal::opt_intrusive_ptr<Xapian::MatchSpy>> & matchspies;

  public:
    /// Constructor.
    RemoteSubMatch(RemoteDatabase *db_,
		   bool decreasing_relevance_,
		   const vector<Xapian::Internal::opt_intrusive_ptr<Xapian::MatchSpy>> & matchspies);

    /// Fetch and collate statistics.
    bool prepare_match(bool nowait, Xapian::Weight::Internal & total_stats);

    /// Start the match.
    void start_match(Xapian::doccount first,
		     Xapian::doccount maxitems,
		     Xapian::doccount check_at_least,
		     Xapian::Weight::Internal & total_stats);

    /// Get PostList.
    PostList * get_postlist(MultiMatch * matcher,
			    Xapian::termcount* total_subqs_ptr,
			    Xapian::Weight::Internal& total_stats);

    /// Get uncollapsed_upper_bound from the remote MSet.
    Xapian::doccount get_uncollapsed_upper_bound() const {
	return uncollapsed_upper_bound;
    }

    /// Get percentage factor - only valid after get_postlist().
    double get_percent_factor() const { return percent_factor; }

    /// Short-cut for single remote match.
    void get_mset(Xapian::MSet & mset) { db->get_mset(mset, matchspies); }
};

#endif /* XAPIAN_INCLUDED_REMOTESUBMATCH_H */
