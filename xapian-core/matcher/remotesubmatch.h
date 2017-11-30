/** @file remotesubmatch.h
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

#include "backends/remote/remote-database.h"
#include "xapian/weight.h"

namespace Xapian {
    class MatchSpy;
}

/// Class for performing matching on a remote database.
class RemoteSubMatch {
    /// Don't allow assignment.
    RemoteSubMatch& operator=(const RemoteSubMatch &) = delete;

    /// Don't allow copying.
    RemoteSubMatch(const RemoteSubMatch &) = delete;

    /// The remote database.
    const RemoteDatabase *db;

    typedef Xapian::Internal::opt_intrusive_ptr<Xapian::MatchSpy> opt_ptr_spy;

    /// The matchspies to use.
    const std::vector<opt_ptr_spy>& matchspies;

  public:
    /// Constructor.
    RemoteSubMatch(const RemoteDatabase *db_,
		   const std::vector<opt_ptr_spy>& matchspies);

    /** Fetch and collate statistics.
     *
     *  Before we can calculate term weights we need to fetch statistics from
     *  each database involved and collate them.
     *
     *  @param block	A RemoteSubMatch may not be able to report statistics
     *			when first asked.  If block is false, it will return
     *			false in this situation allowing the matcher to prepare
     *			other submatches.  If block is true, then this method
     *			will block until statistics are available.
     *
     *  @param total_stats A stats object to which the statistics should be
     *			added.
     *
     *  @return		If block is false and results aren't available yet
     *			then false will be returned and this method must be
     *			called again before the match can proceed.  If results
     *			are available or block is true, then this method
     *			returns true.
     */
    bool prepare_match(bool block, Xapian::Weight::Internal& total_stats);

    /** Start the match.
     *
     *  @param first          The first item in the result set to return.
     *  @param maxitems       The maximum number of items to return.
     *  @param check_at_least The minimum number of items to check.
     *  @param total_stats    The total statistics for the collection.
     */
    void start_match(Xapian::doccount first,
		     Xapian::doccount maxitems,
		     Xapian::doccount check_at_least,
		     Xapian::Weight::Internal& total_stats);

    /// Get MSet.
    Xapian::MSet get_mset() { return db->get_mset(matchspies); }
};

#endif /* XAPIAN_INCLUDED_REMOTESUBMATCH_H */
