/* rset.cc
 */

#include "rset.h"
#include "termlist.h"

doccount
RSet::get_reltermfreq(termname tname) const
{
    if(!initialised_reltermfreqs) {
	vector<RSetItem>::const_iterator doc;
	for(doc = documents.begin(); doc != documents.end(); doc++) {
	    TermList * tl = root->open_term_list((*doc).did);
	    tl->next();
	    while(!(tl->at_end())) {
		// FIXME - can this lookup be done faster?
		// Store termnamess in a hash for each document, rather than
		// a list?
		termname tname_new = tl->get_termname();
		if(reltermfreqs.find(tname_new) != reltermfreqs.end())
		    reltermfreqs[tname_new] ++;
		tl->next();
	    }
	}
	initialised_reltermfreqs = true;
    }

    Assert(reltermfreqs.find(tname) != reltermfreqs.end());

    return reltermfreqs[tname];
}
