/* bm25weight.h
 */

#ifndef _bm25weight_h_
#define _bm25weight_h_

#include "irweight.h"
#include "omtypes.h"
#include "omassert.h"

// BM25 weighting scheme
class BM25Weight : public virtual IRWeight {
    private:
	mutable weight termweight;
	mutable doclength lenpart;

	virtual void calc_termweight() const;
    public:
	~BM25Weight() { }
	weight get_weight(doccount wdf, doclength len) const;
	weight get_maxweight() const;
};

#endif /* _bm25weight_h_ */
