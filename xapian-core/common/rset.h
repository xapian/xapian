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

	mutable map<termid, doccount> reltermfreqs;
	mutable bool initialised_reltermfreqs;
    public:
	vector<RSetItem> documents; // FIXME - should be encapsulated

	RSet(IRDatabase *root_new)
		: root(root_new), initialised_reltermfreqs(false) { return; }
	void add_document(docid did);
	void will_want_termfreq(termid tid) const;
	doccount get_rsize() const;
	doccount get_reltermfreq(termid tid) const;
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
RSet::will_want_termfreq(termid tid) const
{
    reltermfreqs[tid] = 0;
}

inline doccount
RSet::get_reltermfreq(termid tid) const
{
    return 1;
    if(initialised_reltermfreqs) {
	vector<RSetItem>::const_iterator doc = documents.begin();
	while(doc != documents.end()) {
	    TermList * tl = root->open_term_list((*doc).did);
	    tl->next();
	    while(!(tl->at_end())) {
		// FIXME - can this lookup be done faster?
		// Store termids in a hash for each document, rather than
		// a list?
		termid newtid = tl->get_termid();
		if(reltermfreqs.find(newtid) != reltermfreqs.end())
		    reltermfreqs[tid] ++;
		tl->next();
	    }
	}
	initialised_reltermfreqs = true;
    }

    Assert(reltermfreqs.find(tid) != reltermfreqs.end());

    return reltermfreqs[tid];
}

#endif /* _rset_h_ */
