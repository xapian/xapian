/* irweight.cc: C++ class for weight calculation routines */

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>

#include "database.h"

// Calculate weights using statistics retrieved from databases
void
IRWeight::calc_termweight() const
{
    Assert(initialised);

    doccount dbsize = database->get_doccount();

    //printf("Statistics: N=%d n_t=%d ", dbsize, termfreq);

    weight tw;
    tw = (dbsize - termfreq + 0.5) / (termfreq + 0.5); 
    if (tw < 2) {
	// if size and/or termfreq is estimated we can get tw <= 0
	// so handle this gracefully
	if (tw <= 1e-6) tw = 1e-6;
	tw = tw / 2 + 1;
    }
    tw = log(tw);   

    //printf("\t=> termweight = %f\n", tw);
    termweight = tw;
    weight_calculated = true;
}

const double k = 1;

weight
IRWeight::get_weight(doccount wdf) const
{
    if(!weight_calculated) calc_termweight();

    weight wt;

    doclength avlength = database->get_avlength();

    //printf("(wdf, termweight, avlength)  = (%4d, %4.2f, %4f)\n",
    //	   wdf, termweight, avlength);

    // FIXME - precalculate this freq score for several values of wdf - may
    // remove much computation.
    wt = (double) wdf / (k * avlength + wdf);

    //printf("(freq score %4.2f)", wt);

    wt *= termweight;

    //printf("\t=> weight = %f\n", wt);

    return wt;
}

weight
IRWeight::get_maxweight() const
{   
    if(!weight_calculated) calc_termweight();

    return termweight;
}

