/* postlist.h
 *
 * ----START-LICENCE----
 * Copyright 1999 Dialog Corporation
 * 
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
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
	virtual PostList *skip_to(docid, weight w_min) = 0; // Moves to next docid >= specified docid
	virtual bool   at_end() const = 0;        // True if we're off the end of the list

        virtual ~PostList() { return; }
};

// Postlist which generates termweights (rather than merely modifying them
// and passing them on)
// FIXME - choose a better name for this class
class DBPostList : public virtual PostList {
    protected:
	const IRWeight * ir_wt;
    public:
	DBPostList() : ir_wt(NULL) { return; }

	~DBPostList() { return; }

	virtual void set_termweight(const IRWeight *); // Sets term weight
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
    Assert(ir_wt != NULL);
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
