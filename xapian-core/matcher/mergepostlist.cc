/* mergepostlist.cc: MERGE of two posting lists
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

// NB don't prune - even with one sublist we still translate docids...

#include "mergepostlist.h"
#include "omdebug.h"
#include "om/omerrorhandler.h"
#include "boolweight.h"

MergePostList::MergePostList(std::vector<PostList *> plists_,
			     MultiMatch *matcher_,
			     OmErrorHandler * errorhandler_)
	: plists(plists_), current(-1), matcher(matcher_),
	  errorhandler(errorhandler_)
{
    DEBUGCALL(MATCH, void, "MergePostList::MergePostList", "std::vector<PostList *>");
#ifndef USE_MSETPOSTLIST
    for (int i = 0; i < plists.size(); i++) done.push_back(false);
#endif
}

MergePostList::~MergePostList()
{
    DEBUGCALL(MATCH, void, "MergePostList::~MergePostList", "");
    std::vector<PostList *>::const_iterator i;
    for (i = plists.begin(); i != plists.end(); i++) {
	delete *i;
    }
}

PostList *
MergePostList::next(om_weight w_min)
{
    DEBUGCALL(MATCH, PostList *, "MergePostList::next", w_min);
    DEBUGLINE(MATCH, "current = " << current);
#ifdef USE_MSETPOSTLIST
    if (current == -1) current = 0;
#else
    current = (current + 1) % plists.size();
    int old_current = current;
#endif
    do {
	// FIXME: should skip over Remote matchers which aren't ready yet
	// and come back to them later...
	try {
#ifdef USE_MSETPOSTLIST
	    next_handling_prune(plists[current], w_min, matcher);
	    if (!plists[current]->at_end()) break;
	    current++;
#else
	    if (!done[current]) {
		next_handling_prune(plists[current], w_min, matcher);
		if (!plists[current]->at_end()) break;
		done[current] = true;
	    }
#endif
	} catch (OmError & e) {
	    if (errorhandler) {
		DEBUGLINE(EXCEPTION, "Calling error handler in MergePostList::next().");
		(*errorhandler)(e);
		// Continue match without this sub-postlist.
		delete plists[current];
		AutoPtr<LeafPostList> lpl(new EmptyPostList);
		// give it a weighting object
		// FIXME: make it an EmptyWeight instead of BoolWeight
		OmSettings unused;
		lpl->set_termweight(new BoolWeight(unused));
		plists[current] = lpl.release();
	    } else {
		throw;
	    }
	}
#ifdef USE_MSETPOSTLIST
    } while ((unsigned)current < plists.size());
#else
	current = (current + 1) % plists.size();
    } while (current != old_current);
#endif
    DEBUGLINE(MATCH, "current = " << current);
    RETURN(NULL);
}

PostList *
MergePostList::skip_to(om_docid did, om_weight w_min)
{
    // MergePostList doesn't return documents in docid order, so skip_to
    // isn't a meaningful operation.
    throw OmInvalidOperationError("MergePostList doesn't support skip_to");
}
