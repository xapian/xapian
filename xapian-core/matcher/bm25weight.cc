/* bm25weight.cc: C++ class for weight calculation routines
 *
 * ----START-LICENCE----
 * -----END-LICENCE-----
 */

#include <math.h>

#include "database.h"
#include "bm25weight.h"
#include "rset.h"
#include "omassert.h"

#include "config.h"

///////////////////////////////////////////////////////////////////////////

// const double A = 1; // used with wqf (which we don't do yet)
const double B = 1;
const double D = .5;
const double C = 1 / (B + 1);

// Calculate weights using statistics retrieved from databases
void
BM25Weight::calc_termweight() const
{
    Assert(initialised);

    doccount dbsize = root->get_doccount();
    lenpart = B * D / root->get_avlength();

    DebugMsg("Statistics: N=" << dbsize << " n_t=" << termfreq);

    weight tw = 0;
    doccount rsize;
    if(rset != NULL && (rsize = rset->get_rsize()) != 0) {
	doccount rtermfreq = rset->get_reltermfreq(tname);

	DebugMsg(" R=" << rsize << " r_t=" << rtermfreq);

	tw = (rtermfreq + 0.5) * (dbsize - rsize - termfreq + rtermfreq + 0.5) /
	     ((rsize - rtermfreq + 0.5) * (termfreq - rtermfreq + 0.5));
    } else {
	tw = (dbsize - termfreq + 0.5) / (termfreq + 0.5);
    }
    if (tw < 2) {
	// if size and/or termfreq is estimated we can get tw <= 0
	// so handle this gracefully
	if (tw <= 1e-6) tw = 1e-6;
	tw = tw / 2 + 1;
    }
    tw = log(tw);

    DebugMsg(" => termweight = " << tw << endl);
    termweight = tw;
    weight_calculated = true;
}

weight
BM25Weight::get_weight(doccount wdf, doclength len) const
{
    if(!weight_calculated) calc_termweight();

    weight wt = (double) wdf / (len * lenpart + B * (1 - D) + wdf);

    wt *= termweight;

    return wt;
}

weight
BM25Weight::get_maxweight() const
{   
    if(!weight_calculated) calc_termweight();

    return termweight;
}
