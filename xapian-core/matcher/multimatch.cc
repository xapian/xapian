/* multimatch.cc
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

#include "config.h"
#include "multimatch.h"
#include "match.h"
#include "localmatch.h"
#include "rset.h"
#include "omdebug.h"
#include "omenquireinternal.h"
#include "../api/omdatabaseinternal.h"
#include "mergepostlist.h"
#include "msetpostlist.h"

#ifdef MUS_BUILD_BACKEND_REMOTE
#include "networkmatch.h"
#endif /* MUS_BUILD_BACKEND_REMOTE */

#include <algorithm>

////////////////////////////////////
// Initialisation and cleaning up //
////////////////////////////////////

MultiMatch::MultiMatch(const OmDatabase &db,
		       const OmQueryInternal * query,
		       const OmRSet & omrset,
		       const OmSettings & moptions,
		       AutoPtr<StatsGatherer> gatherer_)
	: gatherer(gatherer_),
query_save_for_hack(*query), moptions_save_for_hack(moptions)
{
    std::vector<RefCntPtr<Database> >::iterator i;
    for (i = db.internal->databases.begin();
	 i != db.internal->databases.end(); ++i) {
	RefCntPtr<SingleMatch> smatch(make_match_from_database((*i).get()));
	smatch->link_to_multi(gatherer.get());
	leaves.push_back(smatch);
    }

    set_query(query);
    set_rset(omrset);
    set_options(moptions);

    prepare_matchers();
}

RefCntPtr<SingleMatch>
MultiMatch::make_match_from_database(Database *db)
{
    /* There is currently only one special case, for network
     * databases.
     */
    if (db->is_network()) {
#ifdef MUS_BUILD_BACKEND_REMOTE
	return RefCntPtr<SingleMatch>(new NetworkMatch(db));
#else /* MUS_BUILD_BACKEND_REMOTE */
	throw OmUnimplementedError("Network operation is not available");
#endif /* MUS_BUILD_BACKEND_REMOTE */
    }
    return RefCntPtr<SingleMatch>(new LocalMatch(db));
}

MultiMatch::~MultiMatch()
{
}

void
MultiMatch::set_query(const OmQueryInternal * query)
{
    for(std::vector<RefCntPtr<SingleMatch> >::iterator i = leaves.begin();
	i != leaves.end(); i++) {
	(*i)->set_query(query);
    }
}

void
MultiMatch::set_rset(const OmRSet & omrset)
{
    om_doccount number_of_leaves = leaves.size();
    std::vector<OmRSet> subrsets(number_of_leaves);

    for (std::set<om_docid>::const_iterator reldoc = omrset.items.begin();
	 reldoc != omrset.items.end();
	 reldoc++) {
	om_doccount local_docid = ((*reldoc) - 1) / number_of_leaves + 1;
	om_doccount subdatabase = ((*reldoc) - 1) % number_of_leaves;
	subrsets[subdatabase].add_document(local_docid);
    }

    std::vector<OmRSet>::const_iterator subrset;
    std::vector<RefCntPtr<SingleMatch> >::iterator leaf;
    for (leaf = leaves.begin(), subrset = subrsets.begin();
	 leaf != leaves.end(), subrset != subrsets.end();
	 leaf++, subrset++) {
	(*leaf)->set_rset(*subrset);
    }

    gatherer->set_global_stats(omrset.items.size());
}

void
MultiMatch::set_options(const OmSettings & moptions)
{
    for(std::vector<RefCntPtr<SingleMatch> >::iterator i = leaves.begin();
	i != leaves.end(); i++) {
	(*i)->set_options(moptions);
    }
}

void
MultiMatch::prepare_matchers()
{
    bool prepared;
    bool nowait = true;
    do {
	prepared = true;
	for(std::vector<RefCntPtr<SingleMatch> >::iterator leaf = leaves.begin();
	    leaf != leaves.end(); leaf++) {
	    if (!(*leaf)->prepare_match(nowait)) {
		prepared = false;
	    }
	}
	// Use blocking IO on subsequent passes, so that we don't go into
	// a tight loop.
	nowait = false;
    } while (!prepared);
}

void
MultiMatch::get_mset(om_doccount first, om_doccount maxitems,
		     OmMSet & mset,
		     const OmMatchDecider *mdecider)
{
    Assert(leaves.size() > 0);
    if (leaves.size() == 1) {
	// Only one mset to get - so get it, and block.
	leaves.front()->get_mset(first, maxitems, mset, mdecider, false);
	return;
    }

    std::vector<PostList *> v;
    std::vector<RefCntPtr<SingleMatch> >::iterator i;
    SingleMatch *lm = NULL;
    for (i = leaves.begin(); i != leaves.end(); i++) {
	try {
	    v.push_back((*i)->do_postlist_hack());
	    if (!lm) lm = (*i).get();
	}
	catch (const OmUnimplementedError &e) {
	    // it's a NetworkMatch
	    OmMSet mset_tmp;
	    (*i)->get_mset(0, first + maxitems, mset_tmp, mdecider, false);
	    NetworkMatch *m = dynamic_cast<NetworkMatch *>((*i).get());
	    v.push_back(new MSetPostList(mset_tmp, m->database));
	}
    }
    if (lm) {
	DEBUGLINE(MATCH, "Have a localmatch to abuse");
	lm->do_postlist_hack2(new MergePostList(v));
	lm->get_mset(first, maxitems, mset, mdecider, false);
    } else {
	DEBUGLINE(MATCH, "Don't have a localmatch to abuse");
	NetworkMatch *m = dynamic_cast<NetworkMatch *>(leaves.front().get());
	lm = new LocalMatch(m->database);
	LocalStatsSource s;
	lm->set_query(&query_save_for_hack);
	lm->set_options(moptions_save_for_hack);
	lm->do_postlist_hack2(new MergePostList(v));
	lm->get_mset(first, maxitems, mset, mdecider, false);
	delete lm;
    }
}
