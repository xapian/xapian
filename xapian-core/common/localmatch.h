/* localmatch.h: class for performing the match calculations on postlists
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#ifndef OM_HGUARD_LOCALMATCH_H
#define OM_HGUARD_LOCALMATCH_H

#include "om/omenquire.h"
#include "omqueryinternal.h"
#include "match.h"
#include "stats.h"

class IRWeight;
class IRDatabase;
class LeafDocument;
class PostList;

#include <vector>
#include <map>
#include <memory>

////////////////////////////////////////////////////////////////////////////
// Comparison functions to determine the order of elements in the MSet
// Return true if a should be listed before b
// (By default, equally weighted items will be returned in reverse
// document id number.)

typedef bool (* mset_cmp)(const OmMSetItem &, const OmMSetItem &);
bool msetcmp_forward(const OmMSetItem &, const OmMSetItem &);
bool msetcmp_reverse(const OmMSetItem &, const OmMSetItem &);

/** Class for performing the best match calculations on a database.
 *  This is the Match class which performs the main calculation: other
 *  Match objects merge or transmit the results of LocalMatch objects.
 */
class LocalMatch : public SingleMatch
{
    private:
        IRDatabase *database;
	LocalStatsSource statssource;

        int min_weight_percent;

	/// Whether max weights have been calculated
	bool max_weight_needs_calc;

	/// Max weight that an item could have
        om_weight max_weight;

	/** Max "extra weight" that an item can get (ie, not from the
	 *  postlist tree.
	 */
	om_weight max_extra_weight;

	/// Root postlist of query tree.  0 if not built yet.
	PostList * query;

	/// Query to be run
	OmQueryInternal users_query;

	/** Extra weight object - used to calculate part of doc weight which
	 *  doesn't come from the sum.
	 */
	IRWeight * extra_weight;

	/// vector of weights.  This is just so that they can be deleted
	std::vector<IRWeight *> weights;

	/// RSet to be used (affects weightings)
	std::auto_ptr<RSet> rset;

	/// Weighting scheme which has been requested.
	IRWeight::weight_type requested_weighting;

	/** Weighting scheme to use.
	 *  This may differ from the requested_type if, for example,
	 *  the query is pure boolean.
	 */
	IRWeight::weight_type actual_weighting;

	/// Whether to perform collapse operation
	bool do_collapse;

	/// Key to collapse on, if desired
	om_keyno collapse_key;

	/** Optional limit on number of terms to OR together.
	 *  If zero, there is no limit.
	 */
	om_termcount max_or_terms;

	/** The weights of terms in the query: used to select the top ones.
	 */
	std::map<om_termname, om_weight> term_weights;

	/** The termfreqs of terms in the query.
	 */
	std::map<om_termname, om_doccount> term_frequencies;

	/// Comparison functor for sorting MSet
	OmMSetCmp mcmp;

	/** Internal flag to note that maxweight needs to be recalculated
	 *  while query is running.
	 */
        bool recalculate_maxweight;

	/// Build the query tree, if it isn't already built.
	void build_query_tree();

	/// Select the terms in the requested query to use in the actual query
	void select_query_terms();

	/// Calculate the statistics for the query
	void gather_query_statistics();

	/// Make a postlist from a query object
	PostList *postlist_from_query(
				const OmQueryInternal * query_);

	/// Make a postlist from a vector of query objects (AND or OR)
	PostList *postlist_from_queries(
				om_queryop op,
				const std::vector<OmQueryInternal *> & queries,
				om_termcount window);

	/// Open a postlist
	PostList * mk_postlist(const om_termname& tname);

	/// Make the extra weight object if needed
	void mk_extra_weight();

	/// Make a weight
	IRWeight * mk_weight(om_doclength querysize_,
			     om_termname tname_);

	/// Clear the query tree (and the associated weights)
	void del_query_tree();

	/// Internal method to perform the collapse operation
	bool perform_collapse(std::vector<OmMSetItem> &mset,
			      std::map<OmKey, OmMSetItem> &collapse_table,
			      om_docid did,
			      const OmMSetItem &new_item,
			      const OmMSetItem &min_item);

	// disallow copies
	LocalMatch(const LocalMatch &);
	void operator=(const LocalMatch &);

	PostList * build_and_tree(std::vector<PostList *> &postlists);
	PostList * build_or_tree(std::vector<PostList *> &postlists);
    public:
        LocalMatch(IRDatabase * database_);
        ~LocalMatch();

	///////////////////////////////////////////////////////////////////
	// Implement these virtual methods
	void link_to_multi(StatsGatherer *gatherer);

	void set_query(const OmQueryInternal * query_);

        void set_rset(const OmRSet & omrset);
	void set_weighting(IRWeight::weight_type wt_type_);
	void set_options(const OmSettings & moptions_);

	bool prepare_match(bool nowait);

        om_weight get_max_weight();
	bool get_mset(om_doccount first,
		      om_doccount maxitems,
		      OmMSet & mset,
		      const OmMatchDecider *mdecider,
		      bool nowait
		     );

	///////////////////////////////////////////////////////////////////
	// Miscellaneous
	// =============

	/** Called by postlists to indicate that they've rearranged themselves
	 *  and the maxweight now possible is smaller.
	 */
        void recalc_maxweight();
};

#endif /* OM_HGUARD_LOCALMATCH_H */
