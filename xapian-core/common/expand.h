/* expand.h: class for finding expand terms
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

#ifndef OM_HGUARD_EXPAND_H
#define OM_HGUARD_EXPAND_H

#include "database.h"
#include "termlist.h"
#include "om/omenquire.h"

#include <queue>
#include <stack>
#include <vector>

/** Expand decision functor which always decides to use the term. */
class OmExpandDeciderAlways : public OmExpandDecider {
    public:
	int operator()(const om_termname & tname) const { return true; }
};

/** Class for performing the expand operation. */
class OmExpand {
    private:
	// disallow copy
	OmExpand(const OmExpand &);
	void operator=(const OmExpand &);

        IRDatabase *database;

        bool recalculate_maxweight;
	TermList * build_tree(const RSet *rset, const OmExpandWeight *ewt);
    public:
        OmExpand(IRDatabase * database_);

	void expand(om_termcount max_esize,
		    OmESet & eset,
		    const RSet * rset,
		    const OmExpandDecider * decider,
		    bool use_exact_termfreq);
};

inline OmExpand::OmExpand(IRDatabase * database_)
	: database(database_)
{ return; }

#endif /* OM_HGUARD_EXPAND_H */
