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
#include "om/omsettings.h"
#include "omqueryinternal.h"
#include "match.h"
#include "stats.h"

class IRWeight;
class Database;
class LeafDocument;
class PostList;

#include <vector>
#include <map>
#include "autoptr.h"

class SubMatch {
    public:
	Database *db;
	LocalStatsSource statssource;
	PostList *postlist;

	SubMatch(Database *db_) : db(db_) {
	    statssource.my_collection_size_is(db->get_doccount());
	    statssource.my_average_length_is(db->get_avlength());
	}

	void register_term(const om_termname &tname) {
	    statssource.my_termfreq_is(tname, db->get_termfreq(tname));
	}

	void link_to_multi(StatsGatherer *gatherer) {
	    statssource.connect_to_gatherer(gatherer);
	}


	PostList * open_post_list(const om_termname & tname, IRWeight *wt) {
	    LeafPostList * pl = db->open_post_list(tname);
	    pl->set_termweight(wt);
	    return pl;
	}

};   

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
	SubMatch submatch;    

	/// Query to be run
	OmQueryInternal users_query;

	/// RSet to be used (affects weightings)
	AutoPtr<RSet> rset;

	/// Stored match options object
	OmSettings mopts;

	/** Name of weighting scheme to use.
	 *  This may differ from the requested scheme if, for example,
	 *  the query is pure boolean.
	 */
	string weighting_scheme;

	/** Optional limit on number of terms to OR together.
	 *  If zero, there is no limit.
	 */
	om_termcount max_or_terms;

	/** The weights and termfreqs of terms in the query.
	 */
	std::map<om_termname, OmMSet::TermFreqAndWeight> term_info;

	/// Comparison functor for sorting MSet
	OmMSetCmp mcmp;

	/** Internal flag to note that w_max needs to be recalculated
	 *  while query is running.
	 */
        bool recalculate_w_max;

	/// The size of the query (passed to IRWeight objects)
	om_doclength querysize;

	/// Calculate the statistics for the query
	void gather_query_statistics();

	/// Make a postlist from a query object
	PostList *postlist_from_query(const OmQueryInternal * query);

	/// Make a postlist from a vector of query objects (AND or OR)
	PostList *postlist_from_queries(
				OmQuery::op op,
				const std::vector<OmQueryInternal *> & queries,
				om_termcount window);

	/// Make a weight - default argument is used for finding extra_weight
	IRWeight * mk_weight(const OmQueryInternal *query = NULL);

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
        LocalMatch(Database * database_);

	///////////////////////////////////////////////////////////////////
	// Implement these virtual methods
	void link_to_multi(StatsGatherer *gatherer);

	void set_query(const OmQueryInternal * query);

        void set_rset(const OmRSet & omrset);

	void set_options(const OmSettings & mopts_);

	bool prepare_match(bool nowait);

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
