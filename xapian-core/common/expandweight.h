/* expandweight.h
 */

#ifndef _expandweight_h_
#define _expandweight_h_

#include "omtypes.h"
#include "omassert.h"
#include "database.h"

class RSet;

// Information which is passed up through tree of termlists to calculate
// term weightings
class ExpandBits {
    friend class ExpandWeight;
    private:
	weight multiplier;   // Multiplier to apply to get expand weight
	termcount rtermfreq; // Number of relevant docs indexed by term
	termcount termfreq;  // Term frequency (may be within a subset of whole database)
	doccount dbsize;     // Size of database to which termfreq applies
    public:
	ExpandBits(weight multiplier_new,
		   termcount termfreq_new,
		   doccount dbsize_new)
		: multiplier(multiplier_new),
		  rtermfreq(1),
		  termfreq(termfreq_new),
		  dbsize(dbsize_new)
		  { return; }

	friend ExpandBits operator+(const ExpandBits &, const ExpandBits &);
};

// Abstract base class for weighting schemes
class ExpandWeight {
    protected:
	const IRDatabase *root; // Root database
	doccount rootsize;  // Size of whole collection
	doccount rsetsize;  // Size of RSet
    public:
	ExpandWeight(const IRDatabase *root, doccount rsetsize_new);

	ExpandBits get_bits(doccount wdf, doclength len,
			    termcount termfreq, doccount dbsize) const;
	weight get_weight(const ExpandBits &) const;
};

///////////////////////////////
// Inline method definitions //
///////////////////////////////

inline
ExpandWeight::ExpandWeight(const IRDatabase *root_new,
			   doccount rsetsize_new)
	: root(root_new),
	  rsetsize(rsetsize_new)
{
    rootsize = root_new->get_doccount();
    return;
}

inline ExpandBits
ExpandWeight::get_bits(doccount wdf,
		       doclength len,
		       termcount termfreq,
		       doccount dbsize) const
{
    // FIXME -- use wdf and len to calculate multiplier
    return ExpandBits(1.0, termfreq, dbsize);
}

#endif /* _expandweight_h_ */
