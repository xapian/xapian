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
	doccount rtermfreq; // Number of relevant docs indexed by term
	doccount termfreq;  // Term frequency (may be within a subset of whole database)
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
	doccount dbsize;        // Size of whole collection
	doccount rsize;         // Size of RSet
    public:
	ExpandWeight(const IRDatabase *root, doccount rsetsize_new);

	ExpandBits get_bits(termcount wdf, doclength len,
			    doccount termfreq, doccount dbsize) const;
	weight get_weight(const ExpandBits &, const termname &) const;
	weight get_maxweight() const;
};

///////////////////////////////
// Inline method definitions //
///////////////////////////////

inline
ExpandWeight::ExpandWeight(const IRDatabase *root_new,
			   doccount rsetsize_new)
	: root(root_new),
	  rsize(rsetsize_new)
{
    dbsize = root_new->get_doccount();
    return;
}

const double k = 1;

inline ExpandBits
ExpandWeight::get_bits(termcount wdf,
		       doclength len,
		       doccount termfreq,
		       doccount dbsize) const
{
    weight multiplier = 1.0;

    if(wdf > 0) {
	// FIXME -- use alpha, document importance
	// FIXME -- lots of repeated calculation here - have a weight for each
	// termlist, so can cache results?
	multiplier = (k + 1) * wdf / (k * len + wdf);
#if 0
	DebugMsg("Using (wdf, len) = (" << wdf << ", " << len <<
		 ") => multiplier = " << multiplier << endl);
    } else {
	DebugMsg("No wdf information => multiplier = " << multiplier << endl);
#endif
    }
    return ExpandBits(multiplier, termfreq, dbsize);
}

#endif /* _expandweight_h_ */
