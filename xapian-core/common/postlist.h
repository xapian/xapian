/* postlist.h
 */

#ifndef _postlist_h_
#define _postlist_h_

#include "omassert.h"

#include "omtypes.h"
#include "omerror.h"
#include "irweight.h"

class PostList {
    private:
    public:
	virtual doccount get_termfreq() const = 0;// Gets number of docs indexed by this term

	virtual docid  get_docid() const = 0;     // Gets current docid
	virtual weight get_weight() const = 0;    // Gets current weight
        virtual weight get_maxweight() const = 0;    // Gets max weight

        virtual weight recalc_maxweight() = 0; // recalculate weights (used when tree has been autopruned)

	// w_min in the next two functions is simply a hint -
	// documents with a weight less than w_min will be ignored.
	// However, it may be best to return them anyway, if the weight
	// calculation is expensive, since many documents will be thrown
	// away anyway without calculating the weight.
	virtual PostList *next(weight w_min) = 0; // Moves to next docid
	virtual PostList *skip_to(docid, weight w_min);  // Moves to next docid >= specified docid
	virtual bool   at_end() const = 0;        // True if we're off the end of the list

        virtual ~PostList() { return; }
};

// naive implementation for database that can't do any better
inline PostList *
PostList::skip_to(docid id, weight w_min)
{
    Assert(!at_end());
    while (!at_end() && get_docid() < id) {
	PostList *ret = next(w_min);
	if (ret) return ret;
    }
    return NULL;
}


// Postlist which is actually directly related to entries in a database
// (in some sense)  Nice description, eh -- Richard
class DBPostList : public virtual PostList {
    protected:
	IRWeight own_wt;
	const IRWeight * ir_wt;
    public:
	DBPostList() : ir_wt(&own_wt) { return; }
	void set_termweight(const IRWeight *); // Sets term weight
        weight get_maxweight() const;    // Gets max weight
        weight recalc_maxweight();       // recalculate weights
};

inline void
DBPostList::set_termweight(const IRWeight * wt)
{
    ir_wt = wt;
}

// return an upper bound on the termweight
inline weight
DBPostList::get_maxweight() const
{
    // FIXME - too much indirection?
    return ir_wt->get_maxweight();
}

inline weight
DBPostList::recalc_maxweight()
{
    // FIXME - always this?
    // FIXME - const?
    return DBPostList::get_maxweight();
}

#endif /* _postlist_h_ */
