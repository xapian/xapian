/* multimatch.h: class for performing a match
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

#ifndef OM_HGUARD_MULTIMATCH_H
#define OM_HGUARD_MULTIMATCH_H

#include "match.h"
#include "stats.h"

#include <vector>
#include <memory>  // auto_ptr

class MultiDatabase;
class SingleMatch;

/** Class for performing a match over multiple SingleMatch objects.
 */
class MultiMatch
{
    private:
	/// Vector of the items 
	vector<SingleMatch *> leaves;
	
	/// The database
	MultiDatabase *database;

	/// stats gatherer
	auto_ptr<StatsGatherer> gatherer;

	/// Saved RSet (so that it can delete it properly)
	auto_ptr<RSet> rset;

	/// Comparison functor for sorting MSet
	OmMSetCmp mcmp;

#ifdef MUS_DEBUG
	bool allow_add_singlematch;
#endif /* MUS_DEBUG */

	/// Construct a SingleMatch object from an IRDatabase
	auto_ptr<SingleMatch> make_match_from_database(IRDatabase *db);

	/** Change all the document IDs in the given mset to be valid
	 *  globally, rather than within the sub-match which produced them.
	 *
	 *  @param mset              The MSet to alter.
	 *  @param number_of_leaves  The number of sub-matches that exist.
	 *  @param leaf_number       The sub-match which this mset comes from.
	 *  			     (Starting at 1)
	 */
	void change_docids_to_global(vector<OmMSetItem> & mset,
				     om_doccount number_of_leaves,
				     om_doccount leaf_number);

        /** Merge two msets together.
	 *
	 *  @param mset      The mset to put the results in.
	 *  @param more_mset
	 */
	void merge_msets(vector<OmMSetItem> &mset,
			 vector<OmMSetItem> &more_mset,
			 om_doccount lastitem);

	/** Add the next mset available to the current mset, if it is available.
	 *
	 *  FIXME: this method should be refactored to reduce the number
	 *  of parameters.
	 */
	bool add_next_sub_mset(vector<SingleMatch *>::iterator leaf,
			       om_doccount number_of_leaves,
			       om_doccount leaf_number,
			       om_doccount lastitem,
			       const OmMatchDecider *mdecider,
			       vector<bool> & mset_received,
			       vector<SingleMatch *>::size_type *msets_received,
			       om_doccount * tot_mbound,
			       om_weight   * tot_greatest_wt,
			       vector<OmMSetItem> & mset,
			       bool nowait);

	/** Prepare all the sub matchers.
	 *
	 *  This calls prepare_match() on all the sub matchers until they
	 *  all return true to signal that they are prepared.
	 */
	void prepare_matchers();

	/// Copying is not permitted.
	MultiMatch(const MultiMatch &);

	/// Assignment is not permitted.
	void operator=(const MultiMatch &);
    public:
	/** MultiMatch constructor.
	 *
	 *  Caller is responsible for ensuring that the MultiDatabase
	 *  object pointed to remains valid until the multimatch object
	 *  is destroyed, and for deallocating the object afterwards.
	 *
	 *  @param database_ A pointer to the database to use.
	 *
	 *  @param gatherer_ An auto_ptr to a StatsGatherer instance.
	 *                   The MultiMatch takes ownership of the
	 *                   StatsGatherer.  This argument is optional;
	 *                   the default is to use a LocalStatsGatherer,
	 *                   suitable for non-network use.
	 */
	MultiMatch(MultiDatabase *database_,
		   auto_ptr<StatsGatherer> gatherer_
		       = auto_ptr<StatsGatherer>(new LocalStatsGatherer()));
	~MultiMatch();

	void set_query(const OmQueryInternal * query);
	void set_rset(auto_ptr<RSet> rset_);
	void set_weighting(IRWeight::weight_type wt_type);
	void set_options(const OmMatchOptions & moptions_);

	om_weight get_max_weight();

	void match(om_doccount first,
		   om_doccount maxitems,
		   OmMSet & mset,
		   const OmMatchDecider *mdecider
		  );
};

#endif /* OM_HGUARD_MULTIMATCH_H */
