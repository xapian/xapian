/* dbpostlist.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#ifndef _dbpostlist_h_
#define _dbpostlist_h_

#include "postlist.h"
#include "irweight.h"

// Postlist which generates termweights (rather than merely modifying them
// and passing them on)
// FIXME - choose a better name for this class
class DBPostList : public virtual PostList
{
    protected:
	const IRWeight * ir_wt;
    public:
	DBPostList() : ir_wt(NULL) { return; }

	~DBPostList() { return; }

	// Sets term weighting formula, and needed information
	virtual void set_termweight(const IRWeight * wt);

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
    return DBPostList::get_maxweight();
}

#endif /* _dbpostlist_h_ */
