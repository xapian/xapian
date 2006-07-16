/* branchtermlist.h: virtual base class for branched types of termlist
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2004,2006 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef OM_HGUARD_BRANCHTERMLIST_H
#define OM_HGUARD_BRANCHTERMLIST_H

#include "termlist.h"

class BranchTermList : public TermList {
    protected:
	TermList *l, *r;

	void handle_prune(TermList *&kid, TermList *ret) {
	    if (ret) {
		delete kid;
		kid = ret;
	    }
	}

    public:
	virtual ~BranchTermList() {
	    if (l) delete l;
	    if (r) delete r;
	}

	Xapian::termcount positionlist_count() const {
	    throw Xapian::InvalidOperationError("BranchTermList::positionlist_count() isn't meaningful");
	}

	Xapian::PositionIterator positionlist_begin() const {
	    throw Xapian::InvalidOperationError("BranchTermList::positionlist_begin() isn't meaningful");
	}
};

#endif /* OM_HGUARD_BRANCHTERMLIST_H */
