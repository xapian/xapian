/* irweight.h
 */

#ifndef _irweight_h_
#define _irweight_h_

#include "omtypes.h"
#include "omassert.h"

class IRDatabase;
class RSet;

// FIXME - make IRWeight abstract, and have different weighting schemes
// as subclasses
class IRWeight {
    protected:
	const IRDatabase *root;
	doccount termfreq;
	termname tname;
	const RSet * rset;

	bool initialised;

	mutable bool weight_calculated;
	mutable weight termweight;
	mutable doclength lenpart;
    public:
	IRWeight() : initialised(false), weight_calculated(false) { return; }
	virtual ~IRWeight() { }
	void set_stats(const IRDatabase *, doccount, termname, const RSet *);
	virtual void calc_termweight() const;
	virtual weight get_weight(doccount wdf, doclength len) const;
	weight get_maxweight() const;
};

class BM25Weight : public IRWeight {
    public:
	virtual ~BM25Weight() { }
	void calc_termweight() const;
	weight get_weight(doccount wdf, doclength len) const;
};


inline void
IRWeight::set_stats(const IRDatabase *root_new,
		    doccount termfreq_new,
		    termname tname_new,
		    const RSet *rset_new = NULL) {
    // Can set stats several times, but can't set them after we've used them
    Assert(!weight_calculated);

    root = root_new;
    termfreq = termfreq_new;
    tname = tname_new;
    rset = rset_new;
    initialised = true;
}

#endif /* _irweight_h_ */
