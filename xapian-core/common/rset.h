/* rset.h
 */

#ifndef _rset_h_
#define _rset_h_

#include <vector>

class RSetItem {
    public:
	RSetItem(docid did_new) : did(did_new) { return; }
	docid did;
};

class RSet {
    private:
	vector<RSetItem> documents;
	IRDatabase *root;
    public:
	RSet(IRDatabase *root_new) : root(root_new) { return; }
	void add_document(docid did);
	doccount get_rsize() const;
	doccount get_reltermfreq(termid tid) const;
};

///////////////////////////////
// Inline method definitions //
///////////////////////////////

inline void
RSet::add_document(docid did)
{
    documents.push_back(RSetItem(did));
}

inline doccount
RSet::get_rsize() const
{
    return documents.size();
}

// FIXME - dummy implementation for now
inline doccount
RSet::get_reltermfreq(termid tid) const
{
    return 1;
}

#endif /* _rset_h_ */
