/* expandweight.cc: C++ class for weight calculation routines
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

#include <math.h>

#include "expandweight.h"
#include "omassert.h"
#include "config.h"

OMExpandBits
operator+(const OMExpandBits &bits1, const OMExpandBits &bits2)
{   
    OMExpandBits sum(bits1);
    sum.multiplier += bits2.multiplier;
    sum.rtermfreq += bits2.rtermfreq;

    // FIXME - try to share this information rather than pick half of it
    if(bits2.dbsize > sum.dbsize) {
	DebugMsg("OMExpandBits::operator+ using second operand: " <<
		 bits2.termfreq << "/" << bits2.dbsize << " instead of " <<
		 bits1.termfreq << "/" << bits1.dbsize << endl);
	sum.termfreq = bits2.termfreq;
	sum.dbsize = bits2.dbsize;
    }
    else if(bits2.dbsize < sum.dbsize){
	DebugMsg("OMExpandBits::operator+ using first operand: " << 
		bits1.termfreq << "/" << bits1.dbsize << " instead of " <<
		bits2.termfreq << "/" << bits2.dbsize << endl);
    }
    return sum;
}

om_weight
OMExpandWeight::get_weight(const OMExpandBits &bits,
			   const om_termname &tname) const
{
    double termfreq = (double)bits.termfreq;
    if(bits.dbsize != dbsize) {
	if(bits.dbsize > 0) {
	    termfreq *= (double) dbsize / (double)bits.dbsize;
	    DebugMsg("Approximating termfreq of `" << tname << "': " <<
		     bits.termfreq << " * " << dbsize << " / " <<
		     bits.dbsize << " = " << termfreq << " (true value is:" <<
		     root->get_termfreq(tname) << ")" << endl);
	} else {
	    termfreq = root->get_termfreq(tname);
	    DebugMsg("Asking database for termfreq of `" << tname << "': " <<
		     termfreq << endl);
	}
    }

#if 0
    DebugMsg("OMExpandWeight::get_weight("
	     "N=" << dbsize << ", "
	     "n=" << termfreq << ", "
	     "R=" << rsize << ", "
	     "r=" << bits.rtermfreq << ", "
	     "mult=" << bits.multiplier << ")");
#endif

    double rtermfreq = bits.rtermfreq;
    
    om_weight tw;
    tw = (rtermfreq + 0.5) * (dbsize - rsize - termfreq + rtermfreq + 0.5) /
	    ((rsize - rtermfreq + 0.5) * (termfreq - rtermfreq + 0.5));

    // FIXME - this is c&pasted from tradweight.  Inherit instead.
    // FIXME This is to guarantee nice properties (monotonic increase) of the
    // weighting function.
    // Check whether this actually helps / whether it hinders efficiency
    if (tw < 2) {
	// if size and/or termfreq is estimated we can get tw <= 0
	// so handle this gracefully
	if (tw <= 1e-6) tw = 1e-6;
	tw = tw / 2 + 1;
    }
    tw = log(tw);

#if 0
    DebugMsg(" => Term weight = " << tw <<
	     " Expand weight = " << bits.multiplier * tw << endl);
#endif

    return(bits.multiplier * tw);
}

// Provide an upper bound on the values which may be returned as weights 
om_weight
OMExpandWeight::get_maxweight() const
{
    // FIXME - check the maths behind this.
    return(log(4.0 * (rsize + 0.5) * (dbsize - rsize + 0.5)) * rsize);
}
