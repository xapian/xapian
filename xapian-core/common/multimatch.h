/* multimatch.h: class for performing a match
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003 Olly Betts
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

#ifndef OM_HGUARD_MULTIMATCH_H
#define OM_HGUARD_MULTIMATCH_H

#include "match.h"
#include "stats.h"
#include "refcnt.h"

#include "omqueryinternal.h"

#include <vector>
#include <map>
#include "autoptr.h"  // auto_ptr

class SubMatch;
class Xapian::ErrorHandler;
class SocketServer;
class OmWeight;

class MultiMatch
{
    private:
	/// stats gatherer
	AutoPtr<StatsGatherer> gatherer;

	/// Vector of the items.  This MUST be destroyed before "gatherer"
	std::vector<RefCntPtr<SubMatch> > leaves;

	const OmDatabase db;

	const OmQuery::Internal *query;

	om_valueno collapse_key;

	int percent_cutoff;

	om_weight weight_cutoff;

	bool sort_forward;

	om_valueno sort_key;

	int sort_bands;

	time_t bias_halflife;

	om_weight bias_weight;
	
	/// ErrorHandler
	Xapian::ErrorHandler * errorhandler;

	/// Weighting scheme
	const OmWeight * weight;

	/** Internal flag to note that w_max needs to be recalculated
	 *  while query is running.
	 */
        bool recalculate_w_max;

	/** Prepare all the sub matchers.
	 *
	 *  This calls prepare_match() on all the sub matchers until they
	 *  all return true to signal that they are prepared.
	 */
	void prepare_matchers();

	/// get the collapse key
	string get_collapse_key(PostList *pl, const OmDatabase &db,
				om_docid did, om_valueno keyno,
				RefCntPtr<Document> &doc);

	/** get the maxweight that the postlist pl may return, calling
	 *  recalc_maxweight if recalculate_w_max is set, and unsetting it.
	 *  Must only be called on the top of the postlist tree.
	 */
        om_weight getorrecalc_maxweight(PostList *pl);

	/// Copying is not permitted.
	MultiMatch(const MultiMatch &);

	/// Assignment is not permitted.
	void operator=(const MultiMatch &);

    public:
	/** MultiMatch constructor.
	 *
	 *  @param db_       The database to use.
	 *  @param query     The query
	 *  @param omrset    The relevance set
	 *  @param errorhandler Errorhandler object
	 *  @param gatherer_ An auto_ptr to a StatsGatherer instance.
	 *                   The MultiMatch takes ownership of the
	 *                   StatsGatherer.
	 *  @param wtischeme Weighting scheme
	 */
	MultiMatch(const OmDatabase &db_,
		   const OmQuery::Internal * query,
		   const OmRSet & omrset,
		   om_valueno collapse_key_,
		   int percent_cutoff_,
		   om_weight weight_cutoff_,
		   bool sort_forward_,
		   om_valueno sort_key_,
		   int sort_bands_,
		   time_t bias_halflife_,
		   om_weight bias_weight_,
		   Xapian::ErrorHandler * errorhandler,
		   AutoPtr<StatsGatherer> gatherer_,
		   const OmWeight *wtscheme);
	~MultiMatch();

	void get_mset(om_doccount first,
		      om_doccount maxitems,
		      OmMSet & mset,
		      const OmMatchDecider * mdecider);

	/** Called by postlists to indicate that they've rearranged themselves
	 *  and the maxweight now possible is smaller.
	 */
        void recalc_maxweight();
};

#endif /* OM_HGUARD_MULTIMATCH_H */
