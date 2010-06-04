/* rset.h
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003,2005,2006,2008,2009,2010 Olly Betts
 * Copyright 2007 Lemur Consulting Ltd
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

#ifndef OM_HGUARD_RSET_H
#define OM_HGUARD_RSET_H

#include <set>
#include "xapian/enquire.h"
#include "xapian/weight.h"

#include "omenquireinternal.h"

/** A relevance set.
 *
 * This is used internally, and performs the calculation and caching of
 * relevant term frequencies.
 */
class RSetI {
    private:
	// disallow copy
	RSetI(const RSetI &);
	void operator=(const RSetI &);

	const Xapian::Database::Internal *dbroot;

	std::map<std::string, Xapian::doccount> reltermfreqs;
	bool calculated_reltermfreqs;

	std::set<Xapian::docid> documents;

	/** Calculate the statistics.
	 * 
	 *  This should be called only once.
	 */
	void calculate_stats();
    public:

	/// Initialise with a Xapian::Database::Internal and a Xapian::RSet
	RSetI(const Xapian::Database::Internal *dbroot_, const Xapian::RSet & rset)
	: dbroot(dbroot_), calculated_reltermfreqs(false),
	  documents(rset.internal->get_items())
	{ }

	/** Mark a term for calculation of the reltermfreq.
	 * 
	 *  @param tname The term for which the reltermfreq is desired.
	 */
	void will_want_reltermfreq(const string & tname) {
	    reltermfreqs[tname] = 0;
	}

	/** Calculate the statistics, and add them to a stats object.
	 * 
	 *  This method must only be called once for a given RSet.
	 * 
	 *  @param stats The stats object to pass the weights to.
	 */
	void contribute_stats(Xapian::Weight::Internal & stats);
};

#endif /* OM_HGUARD_RSET_H */
