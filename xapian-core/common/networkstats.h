/* networkstats.h: Handling of statistics needed for network searches.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#ifndef OM_HGUARD_NETWORKSTATS_H
#define OM_HGUARD_NETWORKSTATS_H

#include "stats.h"

// forward declaration for NetworkStatsGatherer
class NetServer;

/** A "slave" StatsGatherer used for the remote matcher
 */
class NetworkStatsGatherer : public StatsGatherer {
    private:
	/// Flag indicating that the global stats are uptodate.
	mutable bool have_global_stats;

	/** The NetServer object using us.
	 *  It is used to communicate with the remote statistics
	 *  node.
	 */
	NetServer *nserv;

	/** Fetch the global statistics, once we have all the
	 *  local ones.
	 *  The object will use the NetServer to do the exchange.
	 */
	void fetch_global_stats() const;

	/** Gather all the local statistics.
	 */
	void fetch_local_stats() const;

    public:
	NetworkStatsGatherer(NetServer *nserv);

	/// See StatsGatherer::get_stats()
	const Stats *get_stats() const;

	/// Gather and return the local statistics (ready to send
	/// to the remote end)
	Stats get_local_stats() const;

	/// Ignore the rset size - we need to get it from the
	/// remote end.
	virtual void set_global_stats(om_doccount rset_size) {};
};

class NetClient;

/** Network StatsSource: a virtual StatsSource which is part of the glue
 *  between a StatsGatherer and the remote matching process.
 */
class NetworkStatsSource : public StatsSource {
    private:
	/// The NetClient object used for communications.
	OmRefCntPtr<NetClient> nclient;

	/// A flag indicating whether or not we have the remote
	/// statistics yet.
	bool have_remote_stats;
    public:
	/// Constructor
	NetworkStatsSource(OmRefCntPtr<NetClient> nclient_);

	/// Destructor
	~NetworkStatsSource();

	/// Contribute all the statistics that don't depend on global
	/// stats.  Used by StatsGatherer.
	void contrib_my_stats();

	void take_remote_stats(Stats stats);
};

#endif /* OM_HGUARD_NETWORKSTATS_H */
