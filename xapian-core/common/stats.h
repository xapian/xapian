/* stats.h: Handling of statistics needed for the search.
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

#ifndef OM_HGUARD_STATS_H
#define OM_HGUARD_STATS_H

#include "om/omtypes.h"
#include "omassert.h"

/** Class to hold statistics which are being passed around.
 */
class Stats {
    public:
	
};

/** Statistics collector: gathers statistics from sub-databases, puts them 
 *  together to build statistics for whole collection, and returns the
 *  overall statistics to each sub-database.
 */
class StatsGatherer {
};

/** Statistics leaf: gathers notifications of statistics which will be
 *  needed, and passes them on in bulk.  There is one of these for each
 *  LeafMatch.
 */
class StatsLeaf {
    private:
	const StatsGatherer * gatherer;
    public:
	StatsLeaf(const StatsGatherer * gatherer_,
		  om_doccount my_dbsize_
		  );

	/** Set the term frequency in the sub-database which this stats
	 *  object represents.  This is the number of documents in
	 *  the sub-database indexed by the given term.
	 */
	void my_termfreq_is(om_termname & tname, om_doccount doc);

	/** Set the relevant term-frequency in the sub-database which this
	 *  stats object represents.  This is the number of relevant
	 *  documents in the sub-database indexed by the given term.
	 */
	void my_reltermfreq_is(om_termname & tname, om_doccount doc);

	/** Get the term frequency over the whole collection, for the
	 *  given term.  This is "n_t", the number of documents in the
	 *  collection indexed by the given term.  (May be an approximation.)
	 */
	om_doccount get_total_termfreq(om_termname & tname);

	/** Get the relevant term-frequency over the whole collection, for
	 *  the given term.  This is "r_t", the number of relevant documents
	 *  in the collection indexed by the given term.  (May be an
	 *  approximation.)
	 */
	om_doccount get_total_reltermfreq(om_termname & tname);
};

///////////////////////////////
// Inline method definitions //
///////////////////////////////

#endif /* OM_HGUARD_STATS_H */
