/* ortermlist.h: OR of two term lists
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

#ifndef _ortermlist_h_
#define _ortermlist_h_

#include "database.h"
#include "branchtermlist.h"

class OrTermList : public virtual BranchTermList {
    private:
        termname lhead, rhead;
	bool started;
    public:
	termcount get_approx_size() const;

	ExpandBits get_weighting() const;
	const termname get_termname() const;
        termcount get_wdf() const;
        doccount get_termfreq() const;

	TermList *next();
	bool   at_end() const;

        OrTermList(TermList *, TermList *);
};

inline ExpandBits
OrTermList::get_weighting() const
{
    Assert(started);
    if (lhead < rhead) return l->get_weighting();
    if (lhead > rhead) return r->get_weighting();
    return l->get_weighting() + r->get_weighting();
}

inline doccount
OrTermList::get_termfreq() const
{
    Assert(started);
    if (lhead < rhead) return l->get_termfreq();
    return r->get_termfreq();
}

inline const termname
OrTermList::get_termname() const
{
    Assert(started);
    if (lhead < rhead) return l->get_termname();
    return r->get_termname();
}

inline termcount
OrTermList::get_wdf() const
{
    Assert(started);
    if (lhead < rhead) return l->get_wdf();
    if (lhead > rhead) return r->get_wdf();
    return l->get_wdf() + r->get_wdf();
}

inline bool
OrTermList::at_end() const
{
    AssertParanoid(!(l->at_end()) && !(r->at_end()));
    return false; // Should have thrown a sub-tree, rather than got to end
}

inline termcount
OrTermList::get_approx_size() const
{
    return l->get_approx_size() + r->get_approx_size();
}

#endif /* _ortermlist_h_ */
