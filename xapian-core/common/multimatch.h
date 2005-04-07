/* multimatch.h: class for performing a match
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

#ifndef OM_HGUARD_MULTIMATCH_H
#define OM_HGUARD_MULTIMATCH_H

#include "match.h"
#include "stats.h"

#include "omqueryinternal.h"

#include <vector>
#include <map>
#include "autoptr.h"  // auto_ptr

class SubMatch;

class MultiMatch
{
    private:
	/// stats gatherer
	AutoPtr<StatsGatherer> gatherer;

	/// Vector of the items.  This MUST be destroyed before "gatherer"
	std::vector<Xapian::Internal::RefCntPtr<SubMatch> > leaves;

	const Xapian::Database db;

	const Xapian::Query::Internal *query;

	Xapian::valueno collapse_key;

	int percent_cutoff;

	Xapian::weight weight_cutoff;

	Xapian::Enquire::docid_order order;

	Xapian::valueno sort_key;

	int sort_bands;

	bool sort_by_relevance;

	bool sort_value_forward;

	time_t bias_halflife;

	Xapian::weight bias_weight;
	
	/// ErrorHandler
	Xapian::ErrorHandler * errorhandler;

	/// Weighting scheme
	const Xapian::Weight * weight;

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
	string get_collapse_key(PostList *pl,
				Xapian::docid did, Xapian::valueno keyno,
				Xapian::Internal::RefCntPtr<Xapian::Document::Internal> &doc);

	/** get the maxweight that the postlist pl may return, calling
	 *  recalc_maxweight if recalculate_w_max is set, and unsetting it.
	 *  Must only be called on the top of the postlist tree.
	 */
        Xapian::weight getorrecalc_maxweight(PostList *pl);

	/// Copying is not permitted.
	MultiMatch(const MultiMatch &);

	/// Assignment is not permitted.
	void operator=(const MultiMatch &);

    public:
	/** MultiMatch constructor.
	 *
	 *  @param db_       The database to use.
	 *  @param query     The query
	 *  @param qlen      The query length
	 *  @param omrset    The relevance set
	 *  @param errorhandler Errorhandler object
	 *  @param gatherer_ A pointer to a StatsGatherer instance.
	 *                   The MultiMatch takes ownership of the
	 *                   StatsGatherer.
	 *  @param wtischeme Weighting scheme
	 */
	MultiMatch(const Xapian::Database &db_,
		   const Xapian::Query::Internal * query,
		   Xapian::termcount qlen,
		   const Xapian::RSet & omrset,
		   Xapian::valueno collapse_key_,
		   int percent_cutoff_,
		   Xapian::weight weight_cutoff_,
		   Xapian::Enquire::docid_order order_,
		   Xapian::valueno sort_key_,
		   int sort_bands_,
		   bool sort_by_relevance_,
		   bool sort_value_forward_,
		   time_t bias_halflife_,
		   Xapian::weight bias_weight_,
		   Xapian::ErrorHandler * errorhandler,
		   StatsGatherer * gatherer_,
		   const Xapian::Weight *wtscheme);
	~MultiMatch();

	void get_mset(Xapian::doccount first,
		      Xapian::doccount maxitems,
		      Xapian::doccount check_at_least,
		      Xapian::MSet & mset,
		      const Xapian::MatchDecider * mdecider);

	/** Called by postlists to indicate that they've rearranged themselves
	 *  and the maxweight now possible is smaller.
	 */
        void recalc_maxweight();
};

#endif /* OM_HGUARD_MULTIMATCH_H */
