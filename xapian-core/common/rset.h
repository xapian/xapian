/* rset.h
 */

#ifndef _rset_h_
#define _rset_h_

#include <vector>
#include <map>
#include "termlist.h"
#include "omassert.h"

class RSetItem {
    public:
	RSetItem(docid did_new) : did(did_new) { return; }
	docid did;
};

class RSet {
    private:
	IRDatabase *root;

	mutable map<termname, doccount> reltermfreqs;
	mutable bool initialised_reltermfreqs;
    public:
	vector<RSetItem> documents; // FIXME - should be encapsulated

	RSet(IRDatabase *root_new)
		: root(root_new), initialised_reltermfreqs(false) { return; }
	void add_document(docid did);
	void will_want_termfreq(termname tname) const;
	doccount get_rsize() const;
	doccount get_reltermfreq(termname tname) const;
};

///////////////////////////////
// Inline method definitions //
///////////////////////////////

inline void
RSet::add_document(docid did)
{
    // FIXME - check that document isn't already in rset
    Assert(!initialised_reltermfreqs);
    documents.push_back(RSetItem(did));
}

inline doccount
RSet::get_rsize() const
{
    return documents.size();
}

inline void
RSet::will_want_termfreq(termname tname) const
{
    reltermfreqs[tname] = 0;
}

inline doccount
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

#endif /* _rset_h_ */
