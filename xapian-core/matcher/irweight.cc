/* irweight.cc: How to create irweight objects */

#include "irweight.h"

// Include headers for all the weight types
#include "tradweight.h"
#include "bm25weight.h"
#include "boolweight.h"

IRWeight * 
IRWeight::create(IRWeight::weight_type wt_type)
{
    IRWeight * weight = NULL;

    // Create weight of correct type
    switch(wt_type) {
	case WTTYPE_BOOL:
	    weight = new BoolWeight();
	    break;
	case WTTYPE_TRAD:
	    weight = new TradWeight();
	    break;
	case WTTYPE_BM25:
	    weight = new BM25Weight();
	    break;
	default:
	    throw OmInvalidArgumentError("Unknown weighting scheme");
    }

    // Check that we have a weighting object
    if(weight == NULL) {
	throw OmOpeningError("Couldn't create weighting object");
    }

    return weight;
}
