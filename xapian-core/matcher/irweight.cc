/* irweight.cc: How to create irweight objects */

#include "irweight.h"

// Include headers for all built-in weight types
#include "tradweight.h"
#include "bm25weight.h"
#include "boolweight.h"

map<string, IRWeight *> IRWeight::user_weights;

IRWeight *
IRWeight::create(const string &wt_type)
{
    DEBUGLINE(UNKNOWN, "IRWeight::create(" << wt_type << ")");
    IRWeight * weight = NULL;

    if (wt_type.at(0) != 'x') {
	// Create weight of correct type
	if (wt_type == "bm25") {
	    weight = new BM25Weight();
	} else if (wt_type == "trad") {
	    weight = new TradWeight();	
	} else if (wt_type == "bool") {
	    weight = new BoolWeight();
	} else {
	    throw OmInvalidArgumentError("Unknown weighting scheme");
	}	
    } else {
	// handle user weights
	map<string, IRWeight *>::iterator i;
	i = user_weights.find(wt_type);
	if (i == user_weights.end()) {
	    throw OmOpeningError("Unknown user weighting scheme");
	}
	weight = i->second->clone();
    }

    // Check that we have a weighting object
    Assert(weight != NULL);

    return weight;
}
