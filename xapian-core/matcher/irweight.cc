/* irweight.cc: C++ class for weight calculation routines */

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>

#include "database.h"

// Calculate weights using statistics retrieved from databases
weight
IRWeight::calc_termweight() const
{
    Assert(initialised);
    if(!weight_calculated) {
	doccount dbsize = database->get_doccount();
	doclength avlength = database->get_avlength();

	printf("Statistics: N=%d L=%f n_t=%d ", 
	       dbsize, avlength, termfreq);

	weight tw;
	tw = (dbsize - termfreq + 0.5) / (termfreq + 0.5); 
	if (tw < 2) {
	    // if size and/or termfreq is estimated we can get tw <= 0
	    // so handle this gracefully
	    if (tw <= 1e-6) tw = 1e-6;
	    tw = tw / 2 + 1;
	}
	tw = log(tw);   

	printf("\t=> termweight = %f\n", tw);
	termweight = tw;
	weight_calculated = true;
    }
    
    return termweight;
}                                   
