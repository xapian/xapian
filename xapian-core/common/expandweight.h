/* expandweight.h
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

#ifndef _expandweight_h_
#define _expandweight_h_

#include "omtypes.h"
#include "omassert.h"
#include "database.h"

class RSet;

// Information which is passed up through tree of termlists to calculate
// term weightings
class OMExpandBits {
    friend class OMExpandWeight;
    private:
	om_weight multiplier;   // Multiplier to apply to get expand weight
	om_doccount rtermfreq; // Number of relevant docs indexed by term
	om_doccount termfreq;  // Term frequency (may be within a subset of whole database)
	om_doccount dbsize;     // Size of database to which termfreq applies
    public:
	OMExpandBits(om_weight multiplier_new,
		   om_termcount termfreq_new,
		   om_doccount dbsize_new)
		: multiplier(multiplier_new),
		  rtermfreq(1),
		  termfreq(termfreq_new),
		  dbsize(dbsize_new)
		  { return; }

	friend OMExpandBits operator+(const OMExpandBits &, const OMExpandBits &);
};

// Abstract base class for weighting schemes
class OMExpandWeight {
    protected:
	const IRDatabase *root; // Root database
	om_doccount dbsize;        // Size of whole collection
	om_doccount rsize;         // Size of RSet
    public:
	OMExpandWeight(const IRDatabase *root_, om_doccount rsetsize_);

	OMExpandBits get_bits(om_termcount wdf, om_doclength len,
			      om_doccount termfreq, om_doccount dbsize) const;
	om_weight get_weight(const OMExpandBits & bits,
			     const om_termname & tname) const;
	om_weight get_maxweight() const;
};

///////////////////////////////
// Inline method definitions //
///////////////////////////////

inline
OMExpandWeight::OMExpandWeight(const IRDatabase *root_,
			   om_doccount rsetsize_)
	: root(root_),
	  rsize(rsetsize_)
{
    dbsize = root->get_doccount();
    return;
}

const double k = 1;

inline OMExpandBits
OMExpandWeight::get_bits(om_termcount wdf,
		       om_doclength len,
		       om_doccount termfreq,
		       om_doccount dbsize) const
{
    om_weight multiplier = 1.0;

    if(wdf > 0) {
	// FIXME -- use alpha, document importance
	// FIXME -- lots of repeated calculation here - have a weight for each
	// termlist, so can cache results?
	multiplier = (k + 1) * wdf / (k * len + wdf);
#if 0
	DebugMsg("Using (wdf, len) = (" << wdf << ", " << len <<
		 ") => multiplier = " << multiplier << endl);
    } else {
	DebugMsg("No wdf information => multiplier = " << multiplier << endl);
#endif
    }
    return OMExpandBits(multiplier, termfreq, dbsize);
}

#endif /* _expandweight_h_ */
