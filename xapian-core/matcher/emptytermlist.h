/* emptytermlist.h: empty term list (for degenerated trees)
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

#ifndef OM_HGUARD_EMPTYTERMLIST_H
#define OM_HGUARD_EMPTYTERMLIST_H

#include "termlist.h"

class EmptyTermList : public virtual TermList {
    public:
	termcount get_approx_size() const;
	weight get_weight() const;
	termname get_termname() const;
	termcount get_wdf() const;
	doccount get_termfreq() const;

	TermList *next();
	bool      at_end() const;
};

inline termcount
EmptyTermList::get_approx_size() const
{
    return 0;
}

inline weight
EmptyTermList::get_weight() const
{
    Assert(0); // no terms
    return 0;
}

inline termname
EmptyTermList::get_termname() const
{
    Assert(0); // no terms
    return "";
}

inline termcount
EmptyTermList::get_wdf() const
{
    Assert(0); // no terms
    return 0;
}

inline doccount
EmptyTermList::get_termfreq() const
{
    Assert(0); // no terms
    return 0;
}

inline TermList *
EmptyTermList::next()
{
    return NULL;
}

inline bool
EmptyTermList::at_end() const
{
    return true;
}

#endif /* OM_HGUARD_EMPTYTERMLIST_H */
