/* branchtermlist.h: virtual base class for branched types of termlist
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

#ifndef OM_HGUARD_BRANCHTERMLIST_H
#define OM_HGUARD_BRANCHTERMLIST_H

#include "termlist.h"

class BranchTermList : public TermList {
    protected:
        void handle_prune(TermList *&kid, TermList *ret);
        TermList *l, *r;
    public:
        virtual ~BranchTermList();
};

inline
BranchTermList::~BranchTermList()
{
    if (l) delete l;
    if (r) delete r;
}

inline void
BranchTermList::handle_prune(TermList *&kid, TermList *ret)
{
    if (ret) {
	delete kid;
	kid = ret;
    }
}

#endif /* OM_HGUARD_BRANCHTERMLIST_H */
