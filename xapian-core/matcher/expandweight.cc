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
	sum.termfreq = bits2.termfreq;
	sum.dbsize = bits2.dbsize;
    }
    return sum;
}

weight
ExpandWeight::get_weight(const ExpandBits &bits) const
{
    cout << "ExpandWeight::get_weight(" <<
	    bits.multiplier << ", " <<
	    bits.rtermfreq << ", " <<
	    bits.termfreq << ", " <<
	    bits.dbsize << endl;

    return(bits.rtermfreq);
}
