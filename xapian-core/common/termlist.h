/* termlist.h
 */

#ifndef _termlist_h_
#define _termlist_h_

#include "omassert.h"

#include "omtypes.h"
#include "omerror.h"
#include "irweight.h"

class TermList {
    private:
    public:
	virtual termcount get_approx_size() const = 0; // Gets size of termlist
	virtual weight get_weight() const = 0; // Gets weight of current termid
	virtual termid get_termid() const = 0; // Gets current termid
	virtual termcount get_wdf() const = 0; // Get wdf of current term
	virtual doccount get_termfreq() const = 0; // Get num of docs indexed by term
	virtual TermList * next() = 0; // Moves to next termid
	virtual bool   at_end() const = 0; // True if we're off the end of the list

        virtual ~TermList() { return; }
};

class DBTermList : public virtual TermList {
    protected:
	BM25Weight own_wt;
	const IRWeight * ir_wt;
    public:
	DBTermList() : ir_wt(&own_wt) { return; }
	void set_termweight(const IRWeight *); // Sets term weight
	weight get_maxweight() const;    // Gets max weight
	weight recalc_maxweight();       // recalculate weights
};

inline void
DBTermList::set_termweight(const IRWeight * wt)
{   
        ir_wt = wt;
}

// return an upper bound on the termweight
inline weight
DBTermList::get_maxweight() const
{
    // FIXME - too much indirection?
    return ir_wt->get_maxweight();
}

inline weight
DBTermList::recalc_maxweight()
{
    // FIXME - always this?
    // FIXME - const?
    return DBTermList::get_maxweight();
}

#endif /* _termlist_h_ */
