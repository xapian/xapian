/* leafmatch.h: class for performing the match calculations on postlists
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

#ifndef OM_HGUARD_LEAFMATCH_H
#define OM_HGUARD_LEAFMATCH_H


#include "omassert.h"
#include "om/omenquire.h"

#include "match.h"
#include "stats.h"
#include "irweight.h"

class IRDatabase;
class LeafDocument;

#include <stack>
#include <vector>
#include <map>

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
 *  Match objects merge or transmit the results of LeafMatch objects.
 */
class LeafMatch : public Match
{
    private:
        IRDatabase *database;
	StatsLeaf statsleaf;

        int min_weight_percent;
        om_weight max_weight;

	PostList * query;
	vector<IRWeight *> weights;

	/// RSet to be used (affects weightings)
	RSet *rset;

	/// Weighting scheme to use
	IRWeight::weight_type wt_type;

	/// Whether to perform collapse operation
	bool do_collapse;

	/// Key to collapse on, if desired
	om_keyno collapse_key;

        bool recalculate_maxweight;

	/// Make a postlist from a query object
	PostList * postlist_from_query(const OmQueryInternal * query_);

	/// Make a postlist from a vector of query objects (AND or OR)
	PostList * postlist_from_queries(
				 om_queryop op,
				 const vector<OmQueryInternal *> & queries);

	/// Open a postlist
	LeafPostList * mk_postlist(const om_termname& tname,
				 RSet * rset);

	/// Make a weight
	IRWeight * mk_weight(om_doclength querysize_,
			     om_termname tname_,
			     const RSet * rset_);

	/// Internal method to perform the collapse operation
	bool perform_collapse(vector<OmMSetItem> &mset,
			      map<OmKey, OmMSetItem> &collapse_table,
			      om_docid did,
			      const OmMSetItem &new_item,
			      const MSetCmp &mcmp,
			      const OmMSetItem &min_item,
			      const LeafDocument *irdoc);

	// disallow copies
	LeafMatch(const LeafMatch &);
	void operator=(const LeafMatch &);
    public:
        LeafMatch(IRDatabase * database_,
		  StatsGatherer * gatherer_);
        ~LeafMatch();

	///////////////////////////////////////////////////////////////////
	// Implement these virtual methods 

	void set_query(const OmQueryInternal * query_);

        void set_rset(RSet * rset_);
	void set_weighting(IRWeight::weight_type wt_type_);
        void set_min_weight_percent(int pcent);
	void set_collapse_key(om_keyno key);
	void set_no_collapse();

        om_weight get_max_weight();

	void match(om_doccount first,
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

///////////////////////////////
// Inline method definitions //
///////////////////////////////

inline void
LeafMatch::set_collapse_key(om_keyno key)
{
    do_collapse = true;
    collapse_key = key;
}

inline void
LeafMatch::set_no_collapse()
{
    do_collapse = false;
}

inline void
LeafMatch::set_rset(RSet *rset_)
{
    Assert(query == NULL);
    rset = rset_;
}

inline void
LeafMatch::set_weighting(IRWeight::weight_type wt_type_)
{
    Assert(query == NULL);
    wt_type = wt_type_;
}

inline void
LeafMatch::set_min_weight_percent(int pcent)
{
    min_weight_percent = pcent;
}

inline om_weight
LeafMatch::get_max_weight()
{
    return max_weight;
}

#endif /* OM_HGUARD_LEAFMATCH_H */
