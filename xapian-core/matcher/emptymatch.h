/* emptymatch.h: class representing an "empty" matcher
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004 Olly Betts
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

#ifndef OM_HGUARD_EMPTYSUBMATCH_H
#define OM_HGUARD_EMPTYSUBMATCH_H

#include "match.h"
#include "emptypostlist.h"
#include <xapian/enquire.h>

class EmptySubMatch : public SubMatch {
    public:
	~EmptySubMatch() { }

	/// Prepare the match operation - we're always prepared.
	bool prepare_match(bool nowait) {
	    DEBUGCALL(MATCH, bool, "EmptySubMatch::prepare_match", nowait);
	    (void)nowait;
	    RETURN(true);
	}

	/// get a postlist - returns an empty postlist
	PostList * get_postlist(Xapian::doccount /*maxitems*/, MultiMatch * /*matcher*/) {
	    AutoPtr<LeafPostList> lpl(new EmptyPostList);
	    // give it a weighting object
	    // FIXME: make it an EmptyWeight instead of BoolWeight
	    lpl->set_termweight(new Xapian::BoolWeight());
	    return lpl.release();
	}

	const std::map<string, Xapian::MSet::Internal::TermFreqAndWeight> get_term_info() const {
	    throw Xapian::InternalError("EmptySubMatch can't give terminfo.");
	}
};

#endif /* OM_HGUARD_EMPTYSUBMATCH_H */
