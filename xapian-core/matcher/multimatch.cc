/* multimatch.cc
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

#include "config.h"
#include "multimatch.h"
#include "match.h"
#include "networkmatch.h"
#include "localmatch.h"
#include "rset.h"
#include "multi_database.h"

#include <algorithm>

////////////////////////////////////
// Initialisation and cleaning up //
////////////////////////////////////

MultiMatch::MultiMatch(MultiDatabase *database_,
		       auto_ptr<StatsGatherer> gatherer_)
	: database(database_),
	  gatherer(gatherer_),
	  mcmp(msetcmp_forward)
#ifdef MUS_DEBUG
	, allow_add_singlematch(true)
#endif /* MUS_DEBUG */
{
    vector<IRDatabase *>::iterator db;
    try {
	for (db = database->databases.begin();
	     db != database->databases.end();
	     ++db) {
	    auto_ptr<SingleMatch> smatch(make_match_from_database(*db));
	    SingleMatch *temp = smatch.get();
	    leaves.push_back(temp);

	    // smatch no longer owns the pointer
	    smatch.release();

	    // Link the SingleMatch object to the StatsGatherer
	    temp->link_to_multi(gatherer.get());
	}
    } catch (...) {
	// clean up in case an exception happens above
        for (vector<SingleMatch *>::iterator i = leaves.begin();
	     i != leaves.end();
	     ++i) {
	    delete *i;
	}
    }
}

auto_ptr<SingleMatch>
MultiMatch::make_match_from_database(IRDatabase *db)
{
    /* There is currently only one special case, for network
     * databases.
     */
    if (db->is_network()) {
	// this is a NetworkDatabase.  Make a NetworkMatch.
	return auto_ptr<SingleMatch>(new NetworkMatch(db));
    } else {
	return auto_ptr<SingleMatch>(new LocalMatch(db));
    }
}

MultiMatch::~MultiMatch()
{
    // delete the singlematches in the container.
    for (vector<SingleMatch *>::iterator i = leaves.begin();
	 i != leaves.end();
	 ++i) {
	delete *i;
    }
}

void
MultiMatch::set_query(const OmQueryInternal * query)
{
    Assert((allow_add_singlematch = false) == false);
    for(vector<SingleMatch *>::iterator i = leaves.begin();
	i != leaves.end(); i++) {
	(*i)->set_query(query);
    }
}

void
MultiMatch::set_rset(auto_ptr<RSet> rset_)
{
    Assert((allow_add_singlematch = false) == false);

    rset = rset_;
    for(vector<SingleMatch *>::iterator i = leaves.begin();
	i != leaves.end(); i++) {
	(*i)->set_rset(rset.get());
    }

    gatherer->set_global_stats(rset->get_rsize());
}

void
MultiMatch::set_weighting(IRWeight::weight_type wt_type_)
{
    Assert((allow_add_singlematch = false) == false);
    for(vector<SingleMatch *>::iterator i = leaves.begin();
	i != leaves.end(); i++) {
	(*i)->set_weighting(wt_type_);
    }
}


void
MultiMatch::set_options(const OmMatchOptions & moptions_)
{
#ifdef MUS_DEBUG
    allow_add_singlematch = false;
#endif
    for(vector<SingleMatch *>::iterator i = leaves.begin();
	i != leaves.end(); i++) {
	(*i)->set_options(moptions_);
    }

    mcmp = moptions_.get_sort_comparator();
}


om_weight
MultiMatch::get_max_weight()
{
    Assert((allow_add_singlematch = false) == false);
    Assert(leaves.size() > 0);

    // FIXME: this always asks the first database; make it pick one in some
    // way so that the load is fairly spread?
    leaves.front()->prepare_match(false);
    om_weight result = leaves.front()->get_max_weight();

#ifdef MUS_DEBUG
    for(vector<SingleMatch *>::iterator i = leaves.begin();
	i != leaves.end(); i++) {
	(*i)->prepare_match(false);
	Assert((*i)->get_max_weight() == result);
    }
#endif /* MUS_DEBUG */

    return result;
}


void
MultiMatch::change_docids_to_global(vector<OmMSetItem> & mset,
				    om_doccount number_of_leaves,
				    om_doccount leaf_number)
{
    vector<OmMSetItem>::iterator mset_item;
    for (mset_item = mset.begin();
	 mset_item != mset.end();
	 mset_item++) {
	mset_item->did = (mset_item->did - 1) * number_of_leaves + leaf_number;
    }
}

void
MultiMatch::merge_msets(vector<OmMSetItem> &mset,
			vector<OmMSetItem> &more_mset,
			om_doccount lastitem) {
    // FIXME - this method is likely to be very inefficient
    DebugMsg("Merging mset of size " << more_mset.size() <<
	     " to existing set of size " << mset.size() <<
	     endl);

    vector<OmMSetItem> old_mset;
    old_mset.swap(mset);

    vector<OmMSetItem>::const_iterator i = old_mset.begin();
    vector<OmMSetItem>::const_iterator j = more_mset.begin();
    while(mset.size() < lastitem &&
	  i != old_mset.end() && j != more_mset.end()) {
	if(mcmp(*i, *j)) {
	    mset.push_back(*i++);
	} else {
	    mset.push_back(*j++);
	}
    }
    while(mset.size() < lastitem &&
	  i != old_mset.end()) {
	mset.push_back(*i++);
    }
    while(mset.size() < lastitem &&
	  j != more_mset.end()) {
	mset.push_back(*j++);
    }
}

bool
MultiMatch::add_next_sub_mset(vector<SingleMatch *>::iterator leaf,
			      om_doccount number_of_leaves,
			      om_doccount leaf_number,
			      om_doccount lastitem,
			      const OmMatchDecider *mdecider,
			      vector<bool> & mset_received,
			      vector<SingleMatch *>::size_type *msets_received,
			      OmMSet & mset,
			      bool nowait) {
    OmMSet sub_mset;

    // Get next mset
    if ((*leaf)->get_mset(0, lastitem, sub_mset.items, &(sub_mset.mbound),
			  &(sub_mset.max_attained), mdecider, nowait)) {
	(*msets_received)++;
	mset_received[leaf_number - 1] = true;

	// Merge stats
	mset.mbound += sub_mset.mbound;
	if(sub_mset.max_attained > mset.max_attained)
	    mset.max_attained = sub_mset.max_attained;

	// Merge items
	change_docids_to_global(sub_mset.items, number_of_leaves, leaf_number);
	merge_msets(mset.items, sub_mset.items, lastitem);

	return true;
    }
    return false;
}

void
MultiMatch::prepare_matchers() {
    bool prepared;
    bool nowait = true;
    do {
	prepared = true;
	for(vector<SingleMatch *>::iterator leaf = leaves.begin();
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
MultiMatch::match(om_doccount first,
		  om_doccount maxitems,
		  OmMSet & mset,
		  const OmMatchDecider *mdecider)
{
    Assert((allow_add_singlematch = false) == false);
    Assert(leaves.size() > 0);

    if(leaves.size() == 1) {
	// Only one mset to get - so get it, and block.
	leaves.front()->prepare_match(false);
	leaves.front()->get_mset(first, maxitems, mset.items,
				 &(mset.mbound), &(mset.max_attained),
				 mdecider, false);
	mset.firstitem = first;
	mset.max_possible = get_max_weight();
    } else if(leaves.size() > 1) {
	// Need to merge msets.
	mset.mbound = 0;
	mset.max_attained = 0;
	om_doccount lastitem = first + maxitems;

	prepare_matchers();

	vector<bool> mset_received(leaves.size(), false);
	vector<SingleMatch *>::size_type msets_received = 0;

	om_doccount leaf_number;
	vector<SingleMatch *>::iterator leaf;

	// Get subsequent msets, and merge each one with the current mset
	// FIXME: this approach may be very inefficient - needs attention.
	bool nowait = true;
	while (msets_received != leaves.size()) {
	    for(leaf = leaves.begin(),
		leaf_number = 1;
		leaf != leaves.end();
		leaf++, leaf_number++) {

		if (mset_received[leaf_number-1]) {
		    continue;
		}

		add_next_sub_mset(leaf,
				  leaves.size(),
				  leaf_number,
				  lastitem,
				  mdecider,
				  mset_received,
				  &msets_received,
				  mset,
				  nowait);
	    }

	    // Use blocking IO on subsequent passes, so that we don't go into
	    // a tight loop.
	    nowait = false;
	}


	// Clear unwanted leading elements.
	if(first != 0) {
	    if(mset.items.size() < first) {
		mset.items.clear();
	    } else if (first > 0) {
		mset.items.erase(mset.items.begin(), mset.items.begin() + first);
	    }
	}
	mset.firstitem = first;

	mset.max_possible = get_max_weight();
    }
}
