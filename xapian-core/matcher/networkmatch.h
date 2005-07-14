/* networkmatch.h: class for communicating with remote match processes
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2005 Olly Betts
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#ifndef OM_HGUARD_NETWORKMATCH_H
#define OM_HGUARD_NETWORKMATCH_H

#include "match.h"
#include "stats.h"

class PendingMSetPostList;
class NetworkStatsSource;

/// Class for performing match calculations remotely
class RemoteSubMatch : public SubMatch {
    private:
	bool is_prepared;

	const NetworkDatabase *db;

	PendingMSetPostList *postlist; // FIXME used in get_term_info() - do this better

	/// RSetI to be used (affects weightings)
	AutoPtr<RSetI> rset;

	/// A pointer to the gatherer, to access the statistics.
	StatsGatherer *gatherer;

	NetworkStatsSource * statssource;

	/// the statistics object
	Stats remote_stats;

	// disallow copies
	RemoteSubMatch(const RemoteSubMatch &);
	void operator=(const RemoteSubMatch &);

	/// Prepare the stats object with contributed
	/// statistics from the remote end.
	void finish_query();

	/// Make a weight - default argument is used for finding extra_weight
	Xapian::Weight * mk_weight(const Xapian::Query::Internal *query = NULL);

    public:
	RemoteSubMatch(const NetworkDatabase *db_,
		       const Xapian::Query::Internal * query,
		       Xapian::termcount qlen,
		       const Xapian::RSet & omrset,
		       Xapian::valueno collapse_key,
		       Xapian::Enquire::docid_order order,
		       Xapian::valueno sort_key,
		       bool sort_by_relevance, bool sort_value_forward,
		       int percent_cutoff, Xapian::weight weight_cutoff,
		       StatsGatherer *gatherer_, const Xapian::Weight *wtscheme);

	~RemoteSubMatch();

	/// Calculate the statistics for the query
	bool prepare_match(bool nowait);

	/// Start the remote match going
	void start_match(Xapian::doccount maxitems);

	PostList * get_postlist(Xapian::doccount maxitems, MultiMatch *matcher);

	const std::map<string, Xapian::MSet::Internal::TermFreqAndWeight> get_term_info() const;
};

#endif /* OM_HGUARD_NETWORKMATCH_H */
