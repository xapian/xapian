/** @file multimatch.h
 * @brief class for performing a match
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2005,2006,2007,2009,2011,2013,2014,2015,2016 Olly Betts
 * Copyright 2009 Lemur Consulting Ltd
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef OM_HGUARD_MULTIMATCH_H
#define OM_HGUARD_MULTIMATCH_H

#include "submatch.h"

#include <vector>

#include "xapian/query.h"
#include "xapian/weight.h"

class MultiMatch
{
    private:
	/// Vector of the items.
	std::vector<Xapian::Internal::intrusive_ptr<SubMatch> > leaves;

	const Xapian::Database db;

	Xapian::Query query;

	Xapian::doccount collapse_max;

	Xapian::valueno collapse_key;

	int percent_cutoff;

	double weight_cutoff;

	Xapian::Enquire::docid_order order;

	Xapian::valueno sort_key;

	Xapian::Enquire::Internal::sort_setting sort_by;

	bool sort_value_forward;

	double time_limit;

	/// Weighting scheme
	const Xapian::Weight * weight;

	/** Internal flag to note that w_max needs to be recalculated
	 *  while query is running.
	 */
	bool recalculate_w_max;

	/** Is each sub-database remote? */
	vector<bool> is_remote;

	/// The matchspies to use.
	const vector<Xapian::Internal::opt_intrusive_ptr<Xapian::MatchSpy>> & matchspies;

	/** get the maxweight that the postlist pl may return, calling
	 *  recalc_maxweight if recalculate_w_max is set, and unsetting it.
	 *  Must only be called on the top of the postlist tree.
	 */
	double getorrecalc_maxweight(PostList *pl);

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
	 *  @param omrset    The relevance set (or NULL for no RSet)
	 *  @param time_limit_ Seconds to reduce check_at_least after (or <= 0
	 *                     for no limit)
	 *  @param stats     The stats object to add our stats to.
	 *  @param wtscheme  Weighting scheme
	 *  @param matchspies_ Any the MatchSpy objects in use.
	 *  @param have_sorter Is there a sorter in use?
	 *  @param have_mdecider Is there a Xapian::MatchDecider in use?
	 */
	MultiMatch(const Xapian::Database &db_,
		   const Xapian::Query & query,
		   Xapian::termcount qlen,
		   const Xapian::RSet * omrset,
		   Xapian::doccount collapse_max_,
		   Xapian::valueno collapse_key_,
		   int percent_cutoff_,
		   double weight_cutoff_,
		   Xapian::Enquire::docid_order order_,
		   Xapian::valueno sort_key_,
		   Xapian::Enquire::Internal::sort_setting sort_by_,
		   bool sort_value_forward_,
		   double time_limit_,
		   Xapian::Weight::Internal & stats,
		   const Xapian::Weight *wtscheme,
		   const vector<Xapian::Internal::opt_intrusive_ptr<Xapian::MatchSpy>> & matchspies_,
		   bool have_sorter, bool have_mdecider);

	/** Run the match and generate an MSet object.
	 *
	 *  @param sorter    Xapian::KeyMaker functor (or NULL for no KeyMaker)
	 */
	void get_mset(Xapian::doccount first,
		      Xapian::doccount maxitems,
		      Xapian::doccount check_at_least,
		      Xapian::MSet & mset,
		      Xapian::Weight::Internal & stats,
		      const Xapian::MatchDecider * mdecider,
		      const Xapian::KeyMaker * sorter);

	/** Called by postlists to indicate that they've rearranged themselves
	 *  and the maxweight now possible is smaller.
	 */
	void recalc_maxweight() {
	    recalculate_w_max = true;
	}

	bool full_db_has_positions() const {
	    return db.has_positions();
	}
};

#endif /* OM_HGUARD_MULTIMATCH_H */
