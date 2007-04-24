/* expandweight.cc: C++ class for weight calculation routines
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004 Olly Betts
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

#include <config.h>

#include <math.h>

#include "expandweight.h"
#include "omassert.h"
#include "omdebug.h"

OmExpandBits
operator+(const OmExpandBits &bits1, const OmExpandBits &bits2)
{
    OmExpandBits sum(bits1);
    sum.multiplier += bits2.multiplier;
    sum.rtermfreq += bits2.rtermfreq;

    // FIXME - try to share this information rather than pick half of it
    if (bits2.dbsize > sum.dbsize) {
	DEBUGLINE(WTCALC, "OmExpandBits::operator+ using second operand: " <<
		  bits2.termfreq << "/" << bits2.dbsize << " instead of " <<
		  bits1.termfreq << "/" << bits1.dbsize);
	sum.termfreq = bits2.termfreq;
	sum.dbsize = bits2.dbsize;
    } else {
	DEBUGLINE(WTCALC, "OmExpandBits::operator+ using first operand: " <<
		  bits1.termfreq << "/" << bits1.dbsize << " instead of " <<
		  bits2.termfreq << "/" << bits2.dbsize);
	// sum already contains the parts of the first operand
    }
    return sum;
}


OmExpandWeight::OmExpandWeight(const Xapian::Database &root_,
			       Xapian::doccount rsetsize_,
			       bool use_exact_termfreq_,
			       double expand_k_ )
	: root(root_),
	  rsize(rsetsize_),
	  use_exact_termfreq(use_exact_termfreq_),
	  expand_k(expand_k_)
{
    DEBUGCALL(MATCH, void, "OmExpandWeight", root_ << ", " << rsetsize_ << ", " << use_exact_termfreq_ << ", " << expand_k_);
    dbsize = root.get_doccount();
    average_length = root.get_avlength();
}

OmExpandBits
OmExpandWeight::get_bits(Xapian::termcount wdf,
			 Xapian::doclength document_length,
			 Xapian::doccount termfreq,
			 Xapian::doccount dbsize_) const
{
    DEBUGCALL(MATCH, OmExpandBits, "OmExpandWeight::get_bits", wdf << ", " << document_length << ", " << termfreq << ", " << dbsize_);
    Xapian::weight multiplier = 1.0;

    if (wdf > 0) {
	const Xapian::doclength norm_length = document_length / average_length;
	DEBUGLINE(WTCALC, "(doc_length, average_length) = (" <<
		  document_length << ", " << average_length <<
		  ") => norm_length = " << norm_length);

	// FIXME -- use alpha, document importance
	// FIXME -- lots of repeated calculation here - have a weight for each
	// termlist, so can cache results?
	multiplier = (expand_k + 1) * wdf / (expand_k * norm_length + wdf);
	DEBUGLINE(WTCALC, "Using (wdf, norm_length) = (" << wdf << ", " <<
		  norm_length << ") => multiplier = " << multiplier);
    } else {
	DEBUGLINE(WTCALC, "No wdf information => multiplier = " << multiplier);
    }
    return OmExpandBits(multiplier, termfreq, dbsize_);
}

Xapian::weight
OmExpandWeight::get_weight(const OmExpandBits &bits,
			   const string &tname) const
{
    DEBUGCALL(MATCH, Xapian::weight, "OmExpandWeight::get_weight", "[bits], " << tname);
    double termfreq = double(bits.termfreq);
    const double rtermfreq = bits.rtermfreq;

    if (bits.dbsize != dbsize) {
	if (bits.dbsize > 0 && !use_exact_termfreq) {
	    termfreq *= double(dbsize) / bits.dbsize;
	    DEBUGLINE(WTCALC, "Approximating termfreq of `" << tname << "': " <<
		      bits.termfreq << " * " << dbsize << " / " <<
		      bits.dbsize << " = " << termfreq << " (true value is:" <<
		      root.get_termfreq(tname) << ")");
	    // termfreq must be at least rtermfreq since there are at least
	    // rtermfreq documents indexed by this term.  And it can't be
	    // more than (dbsize - rsize + rtermfreq) since the number
	    // of relevant documents not indexed by this term can't be
	    // more than the number of documents not indexed by this term.
	    if (termfreq < rtermfreq) {
		termfreq = rtermfreq;
	    } else {
		const double upper_bound = dbsize - rsize + rtermfreq;
		if (termfreq > upper_bound) termfreq = upper_bound;
	    }
	} else {
	    termfreq = root.get_termfreq(tname);
	    DEBUGLINE(WTCALC, "Asked database for termfreq of `" << tname <<
		      "': " << termfreq);
	}
    }

    DEBUGMSG(WTCALC, "OmExpandWeight::get_weight: "
	     "N=" << dbsize << ", "
	     "n=" << termfreq << ", "
	     "R=" << rsize << ", "
	     "r=" << rtermfreq << ", "
	     "mult=" << bits.multiplier);

    Xapian::weight tw;
    tw = (rtermfreq + 0.5) * (dbsize - rsize - termfreq + rtermfreq + 0.5) /
	    ((rsize - rtermfreq + 0.5) * (termfreq - rtermfreq + 0.5));
    Assert(tw > 0);

    // FIXME This is to guarantee nice properties (monotonic increase) of the
    // weighting function.  Actually, I think the important point is that
    // it ensures that tw is positive.
    // Check whether this actually helps / whether it hinders efficiency
    if (tw < 2) {
	tw = tw / 2 + 1;
    }
    tw = log(tw);

    DEBUGLINE(WTCALC, " => Term weight = " << tw <<
	      " Expand weight = " << bits.multiplier * tw);

    //RETURN(rtermfreq * tw);
    RETURN(bits.multiplier * tw);
}

// Provide an upper bound on the values which may be returned as weights
Xapian::weight
OmExpandWeight::get_maxweight() const
{
    DEBUGCALL(MATCH, Xapian::weight, "OmExpandWeight::get_maxweight", "");
    // FIXME - check the maths behind this.
    RETURN(log(4.0 * (rsize + 0.5) * (dbsize - rsize + 0.5)) * rsize);
}
