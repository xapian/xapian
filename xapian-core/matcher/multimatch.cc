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
	  gatherer(gatherer_)
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
MultiMatch::set_min_weight_percent(int pcent)
{
    Assert((allow_add_singlematch = false) == false);
    for(vector<SingleMatch *>::iterator i = leaves.begin();
	i != leaves.end(); i++) {
	(*i)->set_min_weight_percent(pcent);
    }
}

void
MultiMatch::set_collapse_key(om_keyno key)
{
    Assert((allow_add_singlematch = false) == false);
    for(vector<SingleMatch *>::iterator i = leaves.begin();
	i != leaves.end(); i++) {
	(*i)->set_collapse_key(key);
    }
}

void
MultiMatch::set_no_collapse()
{
    Assert((allow_add_singlematch = false) == false);
    for(vector<SingleMatch *>::iterator i = leaves.begin();
	i != leaves.end(); i++) {
	(*i)->set_no_collapse();
    }
}


om_weight
MultiMatch::get_max_weight()
{
    Assert((allow_add_singlematch = false) == false);
    Assert(leaves.size() > 0);

    leaves.front()->prepare_match();
    om_weight result = leaves.front()->get_max_weight();

#ifdef MUS_DEBUG
    for(vector<SingleMatch *>::iterator i = leaves.begin();
	i != leaves.end(); i++) {
	(*i)->prepare_match();
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
			om_doccount lastitem,
			MSetCmp & mcmp) {
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

void
MultiMatch::match(om_doccount first,
		  om_doccount maxitems,
		  vector<OmMSetItem> & mset,
		  mset_cmp cmp,
		  om_doccount * mbound,
		  om_weight * greatest_wt,
		  const OmMatchDecider *mdecider)
{
    Assert((allow_add_singlematch = false) == false);
    Assert(leaves.size() > 0);

    if(leaves.size() == 1) {
	// Only one mset to get - so get it.
	leaves.front()->prepare_match();
	leaves.front()->get_mset(first, maxitems, mset, cmp,
				 mbound, greatest_wt, mdecider);
    } else if(leaves.size() > 1) {
	// Need to merge msets.
	MSetCmp mcmp(cmp);

	om_doccount tot_mbound = 0;
	om_weight   tot_greatest_wt = 0;
	om_doccount lastitem = first + maxitems;

	bool prepared;
	do {
	    prepared = true;
	    // Prepare all the msets
	    for(vector<SingleMatch *>::iterator leaf = leaves.begin();
		leaf != leaves.end(); leaf++) {
		if (!(*leaf)->prepare_match(true)) {
		    prepared = false;
		}
	    }
	} while (!prepared);

	// Get the first mset
	(*(leaves.begin()))->get_mset(0, lastitem, mset, cmp,
				      &tot_mbound, &tot_greatest_wt, mdecider);

	// Modify its docids to be compilant with the whole.
	change_docids_to_global(mset, leaves.size(), 1);

	// Get subsequent msets, and merge each one with the current mset
	// FIXME: this approach may be very inefficient - needs attention.
	om_doccount leaf_number;
	vector<SingleMatch *>::iterator leaf;
	for(leaf = leaves.begin() + 1,
	    leaf_number = 2;
	    leaf != leaves.end();
	    leaf++, leaf_number++) {

	    om_doccount sub_mbound;
	    om_weight   sub_greatest_wt;
	    vector<OmMSetItem> sub_mset;

	    // Get next mset
	    (*leaf)->get_mset(0, lastitem, sub_mset, cmp,
			      &sub_mbound, &sub_greatest_wt, mdecider);
	    change_docids_to_global(sub_mset,
				    leaves.size(),
				    leaf_number);

	    // Merge stats
	    tot_mbound += sub_mbound;
	    if(sub_greatest_wt > tot_greatest_wt)
		tot_greatest_wt = sub_greatest_wt;

	    merge_msets(mset, sub_mset, lastitem, mcmp);
	}

	// Clear unwanted leading elements.
	if(first != 0) {
	    if(mset.size() < first) {
		mset.clear();
	    } else if (first > 0) {
		mset.erase(mset.begin(), mset.begin() + first);
	    }
	}

	// Set the mbound and greatest_wt appropriately.
	*mbound      = tot_mbound;
	*greatest_wt = tot_greatest_wt;
    }
}
