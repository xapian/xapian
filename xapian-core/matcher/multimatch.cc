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

MultiMatch::MultiMatch(const OmDatabase &db_,
		       const OmQueryInternal * query,
		       const OmRSet & omrset,
		       const OmSettings & moptions_,
		       AutoPtr<StatsGatherer> gatherer_)
	: gatherer(gatherer_), db(db_), moptions(moptions_)
{
    om_doccount number_of_leaves = db.internal->databases.size();
    std::vector<OmRSet> subrsets(number_of_leaves);

    for (std::set<om_docid>::const_iterator reldoc = omrset.items.begin();
	 reldoc != omrset.items.end(); reldoc++) {
	om_doccount local_docid = ((*reldoc) - 1) / number_of_leaves + 1;
	om_doccount subdatabase = ((*reldoc) - 1) % number_of_leaves;
	subrsets[subdatabase].add_document(local_docid);
    }
    
    std::vector<OmRSet>::const_iterator subrset = subrsets.begin();

    std::vector<RefCntPtr<Database> >::iterator i;
    for (i = db.internal->databases.begin();
	 i != db.internal->databases.end(); ++i) {
	Assert(subrset != subrsets.end());
	Database *db = (*i).get();
	Assert(db);
	RefCntPtr<SubMatch> smatch;
	/* There is currently only one special case, for network
	 * databases.
	 */
	if (db->is_network()) {
#if 0 //FIXME def MUS_BUILD_BACKEND_REMOTE
	    smatch = RefCntPtr<SubMatch>(new NetworkMatch(db, query, *subrset, moptions, gatherer.get()));
#else /* MUS_BUILD_BACKEND_REMOTE */
	    throw OmUnimplementedError("Network operation is not available");
#endif /* MUS_BUILD_BACKEND_REMOTE */
	} else {
	    smatch = RefCntPtr<SubMatch>(new LocalSubMatch(db, query, *subrset, moptions, gatherer.get()));
	}
	leaves.push_back(smatch);
	subrset++;
    }
    Assert(subrset == subrsets.end());

    gatherer->set_global_stats(omrset.items.size());
    prepare_matchers();
}

MultiMatch::~MultiMatch()
{
}

void
MultiMatch::prepare_matchers()
{
    bool prepared;
    bool nowait = true;
    do {
	prepared = true;
	for (std::vector<RefCntPtr<SubMatch> >::iterator leaf = leaves.begin();
	    leaf != leaves.end(); leaf++) {
	    if (!(*leaf)->prepare_match(nowait)) prepared = false;
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

    PostList *pl;

    if (leaves.size() == 1) {
	// Only one mset to get - so get it
	pl = leaves.front()->get_postlist();
    } else {
	std::vector<PostList *> v;
	std::vector<RefCntPtr<SubMatch> >::iterator i;
	std::map<om_termname, OmMSet::TermFreqAndWeight> term_info;
	for (i = leaves.begin(); i != leaves.end(); i++) {
	    v.push_back((*i)->get_postlist());
	}
	pl = new MergePostList(v);
//	try {
//	}
//	catch (const OmUnimplementedError &e) {
#if 0
	    // it's a NetworkMatch
	    OmMSet mset_tmp;
	    (*i)->get_mset(0, first + maxitems, mset_tmp, mdecider, false);
	    NetworkMatch *m = dynamic_cast<NetworkMatch *>((*i).get());
	    v.push_back(new MSetPostList(mset_tmp, m->database));
	    static const std::map<om_termname, OmMSet::TermFreqAndWeight> &M =
		OmMSet::InternalInterface::get_termfreqandwts(mset_tmp);
	    std::map<om_termname, OmMSet::TermFreqAndWeight>::const_iterator i;
	    for (i = M.begin(); i != M.end(); i++) {
		std::map<om_termname, OmMSet::TermFreqAndWeight>::iterator j;
		j = term_info.find(i->first);
		if (j == term_info.end()) term_info.insert(*i);
	    }
#endif
//	}
    }

    // Extra weight object - used to calculate part of doc weight which
    // doesn't come from the sum.
    IRWeight * extra_weight = leaves.front()->mk_weight();
    LocalMatch lm(db);
    lm.set_options(moptions);
    lm.get_mset(pl, first, maxitems, mset, mdecider, extra_weight,
		leaves.front()->get_term_info(), false);
    delete extra_weight;
}
