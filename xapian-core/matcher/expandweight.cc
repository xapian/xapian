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
    else {
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
	    "mult=" << bits.multiplier << ")" << endl;
#endif /* MUS_DEBUG_VERBOSE */

    double rtermfreq = bits.rtermfreq;
    
    weight wt;
    wt = (rtermfreq + 0.5) * (dbsize - rsize - termfreq + rtermfreq + 0.5) /
	    ((rsize - rtermfreq + 0.5) * (termfreq - rtermfreq + 0.5));

#ifdef MUS_DEBUG_VERBOSE
    cout << "Term weight = " << wt <<
	    " Expand weight = " << bits.multiplier * wt << endl;
#endif /* MUS_DEBUG_VERBOSE */

    return(bits.multiplier * wt);
}
