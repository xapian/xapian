/* rset.h
 */

#ifndef _rset_h_
#define _rset_h_

#include <vector>
#include <map>
#include "omassert.h"

class IRDatabase;

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

#endif /* _rset_h_ */
