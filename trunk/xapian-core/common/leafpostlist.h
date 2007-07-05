/* leafpostlist.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003 Olly Betts
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

#include <xapian/enquire.h>
#include "omassert.h"
#include "postlist.h"

/** A postlist which generates termweights (rather than merely modifying
 *  them and passing them on)
 */
class LeafPostList : public PostList {
    protected:
	const Xapian::Weight * ir_wt;
	bool want_doclength;
    public:
	LeafPostList() : ir_wt(NULL), want_doclength(false) { }

	~LeafPostList() { delete ir_wt; }

	virtual Xapian::doccount get_termfreq() const = 0;
	Xapian::doccount get_termfreq_max() const { return get_termfreq(); }
	Xapian::doccount get_termfreq_min() const { return get_termfreq(); }
	Xapian::doccount get_termfreq_est() const { return get_termfreq(); }

	// Sets term weighting formula, and needed information
	virtual void set_termweight(const Xapian::Weight * wt);

	virtual Xapian::weight get_weight() const;

	virtual Xapian::weight get_maxweight() const;    // Gets max weight
        virtual Xapian::weight recalc_maxweight();       // recalculate weights
};

inline void
LeafPostList::set_termweight(const Xapian::Weight * wt)
{
    ir_wt = wt;
    want_doclength = wt->get_sumpart_needs_doclength();
}

inline Xapian::weight
LeafPostList::get_weight() const
{
    Assert(ir_wt != NULL);
    return ir_wt->get_sumpart(get_wdf(), want_doclength ? get_doclength() : 0);
}

// return an upper bound on the termweight
inline Xapian::weight
LeafPostList::get_maxweight() const
{
    Assert(ir_wt != NULL);
    return ir_wt->get_maxpart();
}

inline Xapian::weight
LeafPostList::recalc_maxweight()
{
    return LeafPostList::get_maxweight();
}

#endif /* OM_HGUARD_LEAFPOSTLIST_H */
