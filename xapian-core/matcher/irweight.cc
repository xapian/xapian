/* irweight.cc: C++ class for weight calculation routines */

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>

#include "database.h"

// Calculate weights using statistics retrieved from db
// If db is NULL weights are not calculated
IRWeight::IRWeight(IRDatabase *db, PostList *pl) {
    doccount dbsize = db->get_doccount();
    doclength avlength = db->get_avlength();
    doccount termfreq = pl->get_termfreq();

    printf("Statistics: N=%d L=%f n_t=%d ", 
	   dbsize, avlength, termfreq);

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
}                                   
