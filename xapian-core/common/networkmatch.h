/* networkmatch.h: class for communicating with remote match processes
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

#ifndef OM_HGUARD_NETWORKMATCH_H
#define OM_HGUARD_NETWORKMATCH_H

#include "match.h"
#include "stats.h"

/** Class for performing match calculations over a network interface.
 */
class NetworkMatch : public SingleMatch
{
    private:
	// disallow copies
	NetworkMatch(const NetworkMatch &);
	void operator=(const NetworkMatch &);

	// the database object
	IRDatabase *database;

	// network socket state
	// FIXME: encapsulate into an object.
	int to;
	int from;
    public:
        NetworkMatch(IRDatabase * database_);
        ~NetworkMatch();

	///////////////////////////////////////////////////////////////////
	// Implement these virtual methods 
	void link_to_multi(StatsGatherer *gatherer);

	void set_query(const OmQueryInternal * query_);

        void set_rset(RSet * rset_);
	void set_weighting(IRWeight::weight_type wt_type_);
        void set_min_weight_percent(int pcent);
	void set_collapse_key(om_keyno key);
	void set_no_collapse();

	void prepare_match();

        om_weight get_max_weight();
	void get_mset(om_doccount first,
		      om_doccount maxitems,
		      vector<OmMSetItem> & mset,
		      mset_cmp cmp,
		      om_doccount * mbound,
		      om_weight * greatest_wt,
		      const OmMatchDecider *mdecider
		     );

	///////////////////////////////////////////////////////////////////
	// Miscellaneous
	// =============

	/** Called by postlists to indicate that they've rearranged themselves
	 *  and the maxweight now possible is smaller.
	 */
        void recalc_maxweight();
};

#endif /* OM_HGUARD_NETWORKMATCH_H */
