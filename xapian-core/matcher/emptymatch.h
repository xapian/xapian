/* match.h: base class for matchers
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

#ifndef OM_HGUARD_EMPTYSUBMATCH_H
#define OM_HGUARD_EMPTYSUBMATCH_H

#include "match.h"
#include "emptypostlist.h"
#include "om/omerror.h"
#include "boolweight.h"

class EmptySubMatch : public SubMatch {
    public:
	~EmptySubMatch() { }

	/// Prepare the match operation - we're always prepared.
	bool prepare_match(bool nowait) {
	    DEBUGCALL(MATCH, bool, "EmptySubMatch::prepare_match", nowait);
	    RETURN(true);
	};

	/// get a postlist - returns an empty postlist
	PostList * get_postlist(om_doccount maxitems, MultiMatch *matcher) {
	    AutoPtr<LeafPostList> lpl(new EmptyPostList);
	    // give it a weighting object
	    // FIXME: make it an EmptyWeight instead of BoolWeight
	    OmSettings unused;
	    lpl->set_termweight(new BoolWeight(unused));
	    return lpl.release();
	}

	Document * open_document(om_docid did) const {
	    throw OmInternalError("Attempt to open document from EmptySubMatch should not happen.");
	}

	const std::map<om_termname, OmMSet::Internal::Data::TermFreqAndWeight> get_term_info() const {
	    throw OmInternalError("EmptySubMatch can't give terminfo.");
	}
};

#endif /* OM_HGUARD_EMPTYSUBMATCH_H */
