/* match.h: base class for matchers
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

#ifndef OM_HGUARD_MATCH_H
#define OM_HGUARD_MATCH_H

#include "database.h"
#include "om/omdocument.h"
#include "om/omenquire.h"
#include "irweight.h"
#include "refcnt.h"

#include <vector>

////////////////////////////////////////////////////////////////////////////
// Comparison functions to determine the order of elements in the MSet
// Return true if a should be listed before b
// (By default, equally weighted items will be returned in reverse
// document id number.)

typedef bool (* mset_cmp)(const OmMSetItem &, const OmMSetItem &);
bool msetcmp_forward(const OmMSetItem &, const OmMSetItem &);
bool msetcmp_reverse(const OmMSetItem &, const OmMSetItem &);

/// Compare an OmMSetItem, using a custom function
class OmMSetCmp {
    public:
	bool (* fn)(const OmMSetItem &a, const OmMSetItem &b);
	OmMSetCmp(bool (* fn_)(const OmMSetItem &a, const OmMSetItem &b))
		: fn(fn_) {}
	bool operator()(const OmMSetItem &a, const OmMSetItem &b) const {
	    return fn(a, b);
	}
};

class StatsGatherer;

/// Base class for single matchers
class SingleMatch : public RefCntBase
{
    friend class MultiMatch;
    private:
	// disallow copies
	SingleMatch(const SingleMatch &);
	void operator=(const SingleMatch &);

	///////////////////////////////////////////////////////////////////
	// Link to a multimatch object (used by
	// MultiMatch::add_singlematch)
	virtual void link_to_multi(StatsGatherer *gatherer) = 0;

    protected:
	/// Flag to remember whether we have prepared to run a query yet
	bool is_prepared;

    public:
	SingleMatch() : is_prepared(false) {};
	virtual ~SingleMatch() = 0;

	///////////////////////////////////////////////////////////////////
	// Set the terms and operations which comprise the query
	// =====================================================

	/** Sets query to use. */
	virtual void set_query(const OmQueryInternal * query_) = 0;

	///////////////////////////////////////////////////////////////////
	// Set additional options for performing the query
	// ===============================================

	/** Set relevance information.
	 */
        virtual void set_rset(const OmRSet & omrset) = 0;

	/** Set the match options. */
	virtual void set_options(const OmSettings & moptions_) = 0;

	///////////////////////////////////////////////////////////////////
	// Prepare to do the match
	// =======================

	/** Prepare to perform the match operation.
	 *  This must be called before get_mset().  It can be called more
	 *  than once.  If nowait is true, the operation has only succeeded
	 *  when the return value is true.
	 *
	 *  @param nowait	If true, then return as soon as
	 *  			possible even if the operation hasn't
	 *  			been completed.  If it hasn't, then
	 *  			the return value will be false.  The
	 *  			caller should retry until prepare_match
	 *  			returns true, or throws an exception to
	 *  			indicate an error.
	 *
	 *  @return  If nowait is true, and the match is being performed
	 *           over a network connection, and the result isn't
	 *           immediately available, this method returns false.
	 *           In all other circumstances it will return true.
	 */
	virtual bool prepare_match(bool nowait) = 0;

	///////////////////////////////////////////////////////////////////
	// Get information about result
	// ============================

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
	 *  @param first          First item to return (start at 0)
	 *  @param maxitems       Maximum number of items to return
	 *  @param mset           Results will be put here
	 *  @param mdecider       Optional decision functor
	 *  @param nowait         If true, then work asynchronously -
	 *                        see prepare_match()
	 *
	 *  @return  If nowait is true, and the mset is being calculated
	 *           over a network connection, and the result isn't
	 *           immediately available, this method returns false.
	 *           In all other circumstances it will return true.
	 *
	 *  @exception OmInvalidArgumentError is thrown if the query has
	 *             not been set appropriately.
	 */
	virtual bool get_mset(om_doccount first,
			      om_doccount maxitems,
			      OmMSet &mset,
			      const OmMatchDecider *mdecider,
			      bool nowait) = 0;
};

///////////////////////////////
// Inline method definitions //
///////////////////////////////

inline SingleMatch::~SingleMatch() {}

#endif /* OM_HGUARD_MATCH_H */
