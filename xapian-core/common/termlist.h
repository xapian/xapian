/* termlist.h
 */

#ifndef _termlist_h_
#define _termlist_h_

#include "omassert.h"

#include "omtypes.h"
#include "omerror.h"
#include "expandweight.h"

class TermList {
    private:
    public:
	virtual termcount get_approx_size() const = 0; // Gets size of termlist
	virtual ExpandBits get_weighting() const = 0; // Gets weighting info for current term
	virtual termname get_termname() const = 0; // Gets current termname
	virtual termcount get_wdf() const = 0; // Get wdf of current term

	virtual doccount get_termfreq() const = 0; // Get num of docs indexed by term
	virtual TermList * next() = 0; // Moves to next term
	virtual bool   at_end() const = 0; // True if we're off the end of the list
};

class DBTermList : public virtual TermList {
    protected:
	const ExpandWeight * wt;
    public:
	DBTermList() : wt(NULL) { return; }
	void set_weighting(const ExpandWeight *); // Sets term weight
};

inline void
DBTermList::set_weighting(const ExpandWeight * wt_new)
{   
    wt = wt_new;
}

#endif /* _termlist_h_ */
