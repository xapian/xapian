/* match.h: base class for matchers
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

#ifndef OM_HGUARD_MATCH_H
#define OM_HGUARD_MATCH_H

#include "database.h"
#include "omassert.h"
#include "om/omdocument.h"
#include "om/omenquire.h"
#include "irweight.h"

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

/// Compare an OmMSetItem, using a custom function
class MSetCmp {
    public:
	bool (* fn)(const OmMSetItem &a, const OmMSetItem &b);
	MSetCmp(bool (* fn_)(const OmMSetItem &a, const OmMSetItem &b))
		: fn(fn_) {}
	bool operator()(const OmMSetItem &a, const OmMSetItem &b) const {
	    return fn(a, b);
	}
};

/// Base class for matchers
class Match
{
    private:
	// disallow copies
	Match(const Match &);
	void operator=(const Match &);
    public:
	Match() {};
	virtual ~Match() = 0;

	///////////////////////////////////////////////////////////////////
	// Set the terms and operations which comprise the query
	// =====================================================

	/** Sets query to use. */
	virtual void set_query(const OmQueryInternal * query_) = 0;

	///////////////////////////////////////////////////////////////////
	// Set additional options for performing the query
	// ===============================================

	/** Set relevance information - the RSet object should not be
	 *  altered after this call.
	 */
	virtual void set_rset(RSet * rset_) = 0;

	/** Set weighting scheme.
	 */
	virtual void set_weighting(IRWeight::weight_type wt_type) = 0;
	
	/** Set cutoff at min percentage - defaults to -1, which means no
	 *  cutoff.
	 */
	virtual void set_min_weight_percent(int pcent) = 0;

	/** Add a key number to collapse by.  Each key value will appear only
	 *  once in the result set.  Collapsing can only be done on one key
	 *  number.
	 */
	virtual void set_collapse_key(om_keyno key) = 0;

	/** Remove the collapse key. */
	virtual void set_no_collapse() = 0;

	///////////////////////////////////////////////////////////////////
	// Get information about result
	// ============================

	/** Get an upper bound on the possible weights.
	 *  (This bound is unlikely to be attained.)
	 */
	virtual om_weight get_max_weight() = 0;

	/** Perform the match operation, and get the matching items.
	 *  Also returns a lower bound on the number of matching records in
	 *  the database (mbound).  Because of some of the optimisations
	 *  performed, this is likely to be much lower than the actual
	 *  number of matching records, but it is expensive to do the
	 *  exact calculation.
	 * 
	 *  It is generally considered that presenting the mbound to users
	 *  causes them to worry about the large number of results, rather
	 *  than how useful those at the top of the mset are, and is thus
	 *  undesirable.
	 *
	 *  @param first       First item to return (start at 0)
	 *  @param maxitems    Maximum number of items to return
	 *  @param mset        Results will be put here
	 *  @param cmp         Comparison operator to sort by
	 *  @param mbound      Mbound will returned here
	 *  @param greatest_wt Gets set to max weight attained
	 *  @param mdecider    Optional decision functor
	 */
	virtual void match(om_doccount first,
			   om_doccount maxitems,
			   vector<OmMSetItem> & mset,
			   mset_cmp cmp,
			   om_doccount * mbound,
			   om_weight * greatest_wt,
			   const OmMatchDecider *mdecider
			  ) = 0;
};

#endif /* OM_HGUARD_MATCH_H */
