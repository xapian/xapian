/* irweight.cc: C++ class for weight calculation routines */

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>

#include "database.h"

// Calculate weights using statistics retrieved from databases
weight
IRWeight::get_weight() const
{
    doccount dbsize = database->get_doccount();
    doclength avlength = database->get_avlength();

    printf("Statistics: N=%d L=%f n_t=%d ", 
	   dbsize, avlength, termfreq);

    weight termweight;
    termweight = (dbsize - termfreq + 0.5) / (termfreq + 0.5); 
    if (termweight < 2) {
	// if size and/or termfreq is estimated we can get termweight <= 0
	// so handle this gracefully
	if (termweight <= 1e-6) termweight = 1e-6;
	termweight = termweight / 2 + 1;
    }
    termweight = log(termweight);   

    printf("\t=> termweight = %f\n",
	   termweight);

    return termweight;
}                                   
