/* expandweight.cc: C++ class for weight calculation routines */

#include <math.h>

#include "expandweight.h"
#include "omassert.h"
#include "config.h"

ExpandBits
operator+(const ExpandBits &bits1, const ExpandBits &bits2)
{   
    ExpandBits sum(bits1);
    sum.multiplier += bits2.multiplier;
    sum.rtermfreq += bits2.rtermfreq;

    // FIXME - try to share this information rather than pick half of it
    if(bits2.dbsize > sum.dbsize) {
#ifdef MUS_DEBUG_VERBOSE
	cout << "ExpandBits::operator+ using second operand: " <<
		bits2.termfreq << "/" << bits2.dbsize << " instead of " <<
		bits1.termfreq << "/" << bits1.dbsize << endl;
#endif /* MUS_DEBUG_VERBOSE */
	sum.termfreq = bits2.termfreq;
	sum.dbsize = bits2.dbsize;
    }
#ifdef MUS_DEBUG_VERBOSE
    else if(bits2.dbsize < sum.dbsize){
	cout << "ExpandBits::operator+ using first operand: " << 
		bits1.termfreq << "/" << bits1.dbsize << " instead of " <<
		bits2.termfreq << "/" << bits2.dbsize << endl;
    }
#endif /* MUS_DEBUG_VERBOSE */
    return sum;
}

weight
ExpandWeight::get_weight(const ExpandBits &bits, const termname &tname) const
{
    double termfreq = (double)bits.termfreq;
    if(bits.dbsize != dbsize) {
	if(bits.dbsize > 0) {
	    termfreq *= (double) dbsize / (double)bits.dbsize;
#ifdef MUS_DEBUG_VERBOSE
	    cout << "Approximating termfreq of `" << tname << "': " <<
		    bits.termfreq << " * " << dbsize << " / " <<
		    bits.dbsize << " = " << termfreq << endl;
#endif /* MUS_DEBUG_VERBOSE */
	} else {
	    termfreq = root->get_termfreq(tname);
#ifdef MUS_DEBUG_VERBOSE
	    cout << "Asking database for termfreq of `" << tname << "': " <<
		    termfreq << endl;
#endif /* MUS_DEBUG_VERBOSE */
	}
    }

#ifdef MUS_DEBUG_VERBOSE
    cout << "ExpandWeight::get_weight("
	    "N=" << dbsize << ", "
	    "n=" << termfreq << ", "
	    "R=" << rsize << ", "
	    "r=" << bits.rtermfreq << ", "
	    "mult=" << bits.multiplier << ")";
#endif /* MUS_DEBUG_VERBOSE */

    double rtermfreq = bits.rtermfreq;
    
    weight tw;
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

#ifdef MUS_DEBUG_VERBOSE
    cout << " => Term weight = " << tw <<
	    " Expand weight = " << bits.multiplier * tw << endl;
#endif /* MUS_DEBUG_VERBOSE */

    return(bits.multiplier * tw);
}

// Provide an upper bound on the values which may be returned as weights 
weight
ExpandWeight::get_maxweight() const
{
    // FIXME - check the maths behind this.
    return(log(4.0 * (rsize + 0.5) * (dbsize - rsize + 0.5)) * rsize);
}
