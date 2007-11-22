/* networkstats.h: Handling of statistics needed for network searches.
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2006 Olly Betts
 * Copyright 2007 Lemur Consulting Ltd
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef OM_HGUARD_NETWORKSTATS_H
#define OM_HGUARD_NETWORKSTATS_H

#include "remote-database.h"
#include "stats.h"

/** A "slave" StatsGatherer used for the remote matcher
 */
class NetworkStatsGatherer : public StatsGatherer {
    private:
	/// Flag indicating that the global stats are uptodate.
	mutable bool have_global_stats;

    public:
	NetworkStatsGatherer();

	/** See StatsGatherer::get_stats()
	 */
	const Stats *get_stats() const;

	/** Gather and return the local statistics (ready to send
	 *  to the remote end).
	 */
	Stats get_local_stats() const;

	/** Set the global statistics to store in this gatherer.
	 *
	 *  This should be called with the global statistics received from the
	 *  remote end.
	 *
	 *  This overrides any local statistics which are already stored in the
	 *  gatherer - they should be included in the global statistics
	 *  supplied.
	 */
	void set_global_stats(const Stats & stats) const;

	/** Ignore the rset size - we need to get it from the
	 *  remote end.
	 */
	virtual void set_global_stats(Xapian::doccount /*rset_size*/) {}
};

#endif /* OM_HGUARD_NETWORKSTATS_H */
