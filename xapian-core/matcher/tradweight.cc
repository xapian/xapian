/* tradweight.cc: C++ class for weight calculation routines */

#include <math.h>

#include "database.h"
#include "tradweight.h"
#include "rset.h"
#include "omassert.h"

#include "config.h"

const double k = 1;

// Calculate weights using statistics retrieved from databases
void
TradWeight::calc_termweight() const
{
    Assert(initialised);

    doccount dbsize = root->get_doccount();
    lenpart = k / root->get_avlength();

#ifdef MUS_DEBUG_VERBOSE
    cout << "Statistics: N=" << dbsize << " n_t=" << termfreq;
#endif /* MUS_DEBUG_VERBOSE */

    weight tw = 0;
    doccount rsize;
    if(rset != NULL && (rsize = rset->get_rsize()) != 0) {
	doccount rtermfreq = rset->get_reltermfreq(tname);

#ifdef MUS_DEBUG_VERBOSE
	cout << " R=" << rsize << " r_t=" << rtermfreq;
#endif /* MUS_DEBUG_VERBOSE */

	tw = (rtermfreq + 0.5) * (dbsize - rsize - termfreq + rtermfreq + 0.5) /
	     ((rsize - rtermfreq + 0.5) * (termfreq - rtermfreq + 0.5));
    } else {
	tw = (dbsize - termfreq + 0.5) / (termfreq + 0.5);
    }

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
    cout << " => termweight = " << tw << endl;
#endif /* MUS_DEBUG_VERBOSE */
    termweight = tw;
    weight_calculated = true;
}

weight
TradWeight::get_weight(doccount wdf, doclength len) const
{
    if(!weight_calculated) calc_termweight();

    weight wt = (double) wdf / (len * lenpart + wdf);

    wt *= termweight;

    return wt;
}

weight
TradWeight::get_maxweight() const
{   
    if(!weight_calculated) calc_termweight();

    return termweight;
}
