/* expandweight.h
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003,2007 Olly Betts
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

#ifndef OM_HGUARD_EXPANDWEIGHT_H
#define OM_HGUARD_EXPANDWEIGHT_H

#include <string>

#include <xapian/database.h>
#include "database.h"

using namespace std;

/** Information which is passed up through tree of termlists to calculate
 *  term weightings.
 */
class OmExpandBits {
    friend class OmExpandWeight;
    private:
	/// Multiplier to apply to get expand weight.
	Xapian::weight multiplier;

	/// Number of relevant docs indexed by term.
	Xapian::doccount rtermfreq;

	/// Term frequency (may be within a subset of whole database).
	Xapian::doccount termfreq;

	/// Size of db subset to which termfreq applies (0 if not multidb).
	Xapian::doccount dbsize;
    public:
	OmExpandBits(Xapian::weight multiplier_new,
		   Xapian::termcount termfreq_new,
		   Xapian::doccount dbsize_new)
		: multiplier(multiplier_new),
		  rtermfreq(1),
		  termfreq(termfreq_new),
		  dbsize(dbsize_new)
		  { }

	friend OmExpandBits operator+(const OmExpandBits &, const OmExpandBits &);
};

/** Abstract base class for expand weighting schemes
 */
class OmExpandWeight {
    protected:
	const Xapian::Database root; // Root database
	Xapian::doccount dbsize;        // Size of whole collection
	Xapian::doccount rsize;         // Size of RSet

	/** Average length of a document in whole collection.
	 */
	Xapian::doclength average_length;

	/** If true, the exact term frequency will be requested from the
	 *  root database, rather than an approximation made, when expand
	 *  across a multi-database is being performed.
	 *
	 *  The approximation is to guess the term frequency based on the
	 *  term frequency in the current database (as returned by the
	 *  termlist), and the proportion of the documents in the total
	 *  collection which that sub-database represents.
	 */
	bool use_exact_termfreq;

	/// Parameter used by expand weighting formula
	double expand_k;

    public:

	OmExpandWeight(const Xapian::Database &root_,
		       Xapian::doccount rsetsize_,
		       bool use_exact_termfreq_,
		       double expand_k_ );

	OmExpandBits get_bits(Xapian::termcount wdf,
			      Xapian::doclength len,
			      Xapian::doccount termfreq,
			      Xapian::doccount dbsize_) const;

	Xapian::weight get_weight(const OmExpandBits & bits,
			     const string & tname) const;

	Xapian::weight get_maxweight() const;
};

#endif /* OM_HGUARD_EXPANDWEIGHT_H */
