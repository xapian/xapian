/* tradweight.h
 *
 * ----START-LICENCE----
 * -----END-LICENCE-----
 */

#ifndef _tradweight_h_
#define _tradweight_h_

#include "irweight.h"
#include "omtypes.h"
#include "omassert.h"

// Traditional weighting scheme
class TradWeight : public virtual IRWeight {
    private:
	mutable weight termweight;
	mutable doclength lenpart;

	void calc_termweight() const;
    public:
	~TradWeight() { }
	weight get_weight(doccount wdf, doclength len) const;
	weight get_maxweight() const;
};

#endif /* _tradweight_h_ */
