/* leafpostlist.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#ifndef OM_HGUARD_LEAFPOSTLIST_H
#define OM_HGUARD_LEAFPOSTLIST_H

#include "postlist.h"
#include "irweight.h"

/** A postlist which generates termweights (rather than merely modifying
 *  them and passing them on)
 */
class LeafPostList : public PostList
{
    protected:
	const IRWeight * ir_wt;
    public:
	LeafPostList() : ir_wt(NULL) { return; }

	~LeafPostList() { delete ir_wt; return; }

	// Sets term weighting formula, and needed information
	virtual void set_termweight(const IRWeight * wt);

	om_weight get_maxweight() const;    // Gets max weight
        om_weight recalc_maxweight();       // recalculate weights
};

inline void
LeafPostList::set_termweight(const IRWeight * wt)
{
    ir_wt = wt;
}

// return an upper bound on the termweight
inline om_weight
LeafPostList::get_maxweight() const
{
    Assert(ir_wt != NULL);
    return ir_wt->get_maxpart();
}

inline om_weight
LeafPostList::recalc_maxweight()
{
    return LeafPostList::get_maxweight();
}

#endif /* OM_HGUARD_LEAFPOSTLIST_H */
