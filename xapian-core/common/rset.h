/* rset.h
 */

#ifndef _rset_h_
#define _rset_h_

#include <vector>

class RSetItem {
    public:
	docid did;
};

class RSet {
    public:
	vector<RSetItem> documents;
};

#endif /* _rset_h_ */
