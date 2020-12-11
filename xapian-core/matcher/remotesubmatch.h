/** @file
 *  @brief SubMatch class for a remote database.
 */
/* Copyright (C) 2006,2007,2009,2011,2014,2015,2018,2019 Olly Betts
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

    /// Index of this subdatabase.
    Xapian::doccount shard;

  public:
    /// Constructor.
    RemoteSubMatch(const RemoteDatabase* db_, Xapian::doccount shard_)
	: db(db_), shard(shard_) {}

    int get_read_fd() const {
	return db->get_read_fd();
    }

    /** Fetch and collate statistics.
     *
     *  Before we can calculate term weights we need to fetch statistics from
     *  each database involved and collate them.
     *
     *  @param total_stats A stats object to which the statistics should be
     *			added.
     */
    void prepare_match(Xapian::Weight::Internal& total_stats);

    /** Start the match.
     *
     *  @param first          The first item in the result set to return.
     *  @param maxitems       The maximum number of items to return.
     *  @param check_at_least The minimum number of items to check.
     *  @param sorter	      KeyMaker for sort keys (NULL for none).
     *  @param total_stats    The total statistics for the collection.
     */
    void start_match(Xapian::doccount first,
		     Xapian::doccount maxitems,
		     Xapian::doccount check_at_least,
		     const Xapian::KeyMaker* sorter,
		     Xapian::Weight::Internal& total_stats);

    typedef Xapian::Internal::opt_intrusive_ptr<Xapian::MatchSpy> opt_ptr_spy;

    /** Get MSet.
     *
     *  @param matchspies   The matchspies to use.
     */
    Xapian::MSet get_mset(const std::vector<opt_ptr_spy>& matchspies) {
	return db->get_mset(matchspies);
    }

    /// Return the index of the corresponding Database shard.
    Xapian::doccount get_shard() const { return shard; }
};

#endif /* XAPIAN_INCLUDED_REMOTESUBMATCH_H */
