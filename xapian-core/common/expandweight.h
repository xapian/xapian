/* expandweight.h
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

#ifndef OM_HGUARD_EXPANDWEIGHT_H
#define OM_HGUARD_EXPANDWEIGHT_H

#include "om/omtypes.h"
#include "database.h"

class RSet;

/** Information which is passed up through tree of termlists to calculate
 *  term weightings.
 */
class OmExpandBits {
    friend class OmExpandWeight;
    private:
	om_weight multiplier;   // Multiplier to apply to get expand weight
	om_doccount rtermfreq; // Number of relevant docs indexed by term
	om_doccount termfreq;  // Term frequency (may be within a subset of whole database)
	om_doccount dbsize;     // Size of database to which termfreq applies
    public:
	OmExpandBits(om_weight multiplier_new,
		   om_termcount termfreq_new,
		   om_doccount dbsize_new)
		: multiplier(multiplier_new),
		  rtermfreq(1),
		  termfreq(termfreq_new),
		  dbsize(dbsize_new)
		  { return; }

	friend OmExpandBits operator+(const OmExpandBits &, const OmExpandBits &);
};

/** Abstract base class for expand weighting schemes
 */
class OmExpandWeight {
    protected:
	const IRDatabase *root; // Root database
	om_doccount dbsize;        // Size of whole collection
	om_doccount rsize;         // Size of RSet

	/** Average length of a document in whole collection.
	 */
	om_doclength average_length;

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
    public:


	OmExpandWeight(const IRDatabase *root_,
		       om_doccount rsetsize_,
		       bool use_exact_termfreq_);

	OmExpandBits get_bits(om_termcount wdf,
			      om_doclength len,
			      om_doccount termfreq,
			      om_doccount dbsize) const;

	om_weight get_weight(const OmExpandBits & bits,
			     const om_termname & tname) const;

	om_weight get_maxweight() const;
};

const double k = 1;

#endif /* OM_HGUARD_EXPANDWEIGHT_H */
