/* ortermlist.h: OR of two term lists
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

#ifndef OM_HGUARD_ORTERMLIST_H
#define OM_HGUARD_ORTERMLIST_H

#include "database.h"
#include "branchtermlist.h"

class OrTermList : public BranchTermList {
    private:
        om_termname lhead, rhead;
	bool started;
    public:
	om_termcount get_approx_size() const;

	OmExpandBits get_weighting() const;
	const om_termname get_termname() const;
        om_termcount get_wdf() const;
        om_doccount get_termfreq() const;

	TermList *next();
	bool   at_end() const;

        OrTermList(TermList * left, TermList * right);
};

inline OmExpandBits
OrTermList::get_weighting() const
{
    Assert(started);
    if (lhead < rhead) return l->get_weighting();
    if (lhead > rhead) return r->get_weighting();
    return l->get_weighting() + r->get_weighting();
}

inline om_doccount
OrTermList::get_termfreq() const
{
    Assert(started);
    if (lhead < rhead) return l->get_termfreq();
    return r->get_termfreq();
}

inline const om_termname
OrTermList::get_termname() const
{
    Assert(started);
    if (lhead < rhead) return l->get_termname();
    return r->get_termname();
}

inline om_termcount
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

inline om_termcount
OrTermList::get_approx_size() const
{
    return l->get_approx_size() + r->get_approx_size();
}

#endif /* OM_HGUARD_ORTERMLIST_H */
