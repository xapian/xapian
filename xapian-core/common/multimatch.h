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
	StatsGatherer gatherer;

	/// Saved RSet (so that it can delete it properly)
	auto_ptr<RSet> rset;
#ifdef MUS_DEBUG
	bool allow_add_singlematch;
#endif /* MUS_DEBUG */

	/// Construct a SingleMatch object from an IRDatabase
	auto_ptr<SingleMatch> make_match_from_database(IRDatabase *db);

	// disallow copies
	MultiMatch(const MultiMatch &);
	void operator=(const MultiMatch &);
    public:
	/** MultiMatch constructor.
	 *
	 *  Caller is responsible for ensuring that the MultiDatabase
	 *  object pointed to remains valid until the multimatch object
	 *  is destroyed, and for deallocating the object afterwards.
	 *
	 *  @param database_ A pointer to the database to use.
	 */
	MultiMatch(MultiDatabase *database_);
	~MultiMatch();

	void set_query(const OmQueryInternal * query);
	void set_rset(auto_ptr<RSet> rset_);
	void set_weighting(IRWeight::weight_type wt_type);
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
};

#endif /* OM_HGUARD_MULTIMATCH_H */
