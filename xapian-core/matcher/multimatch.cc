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

MultiMatch::MultiMatch(const MultiDatabase * multi_database_,
		       const OmQueryInternal * query,
		       const OmRSet & omrset,
		       IRWeight::weight_type wt_type,
		       const OmMatchOptions & moptions,
		       auto_ptr<StatsGatherer> gatherer_)
	: multi_database(multi_database_),
	  gatherer(gatherer_),
	  mcmp(msetcmp_forward)
{
    vector<OmRefCntPtr<IRDatabase> >::const_iterator db;
    for (db = multi_database->databases.begin();
	 db != multi_database->databases.end();
	 ++db) {
	OmRefCntPtr<SingleMatch> smatch(make_match_from_database(db->get()));
	smatch->link_to_multi(gatherer.get());
	leaves.push_back(smatch);
    }

    set_query(query);
    set_rset(omrset);
    set_weighting(wt_type);
    set_options(moptions);

    prepare_matchers();
}

OmRefCntPtr<SingleMatch>
MultiMatch::make_match_from_database(IRDatabase *db)
{
    /* There is currently only one special case, for network
     * databases.
     */
    if (db->is_network()) {
	return OmRefCntPtr<SingleMatch>(new NetworkMatch(db));
    } else {
	return OmRefCntPtr<SingleMatch>(new LocalMatch(db));
    }
}

MultiMatch::~MultiMatch()
{
}

void
MultiMatch::set_query(const OmQueryInternal * query)
{
    for(vector<OmRefCntPtr<SingleMatch> >::iterator i = leaves.begin();
	i != leaves.end(); i++) {
	(*i)->set_query(query);
    }
}

void
MultiMatch::set_rset(const OmRSet & omrset)
{
    om_doccount number_of_leaves = leaves.size();
    vector<OmRSet> subrsets(number_of_leaves);

    for (set<om_docid>::const_iterator reldoc = omrset.items.begin();
	 reldoc != omrset.items.end();
	 reldoc++) {
	om_doccount local_docid = ((*reldoc) - 1) / number_of_leaves + 1;
	om_doccount subdatabase = ((*reldoc) - 1) % number_of_leaves;
	subrsets[subdatabase].add_document(local_docid);
    }

    vector<OmRSet>::const_iterator subrset;
    vector<OmRefCntPtr<SingleMatch> >::iterator leaf;
    for (leaf = leaves.begin(), subrset = subrsets.begin();
	 leaf != leaves.end(), subrset != subrsets.end();
	 leaf++, subrset++) {
	(*leaf)->set_rset(*subrset);
    }

    gatherer->set_global_stats(omrset.items.size());
}

void
MultiMatch::set_weighting(IRWeight::weight_type wt_type_)
{
    for(vector<OmRefCntPtr<SingleMatch> >::iterator i = leaves.begin();
	i != leaves.end(); i++) {
	(*i)->set_weighting(wt_type_);
    }
}


void
MultiMatch::set_options(const OmMatchOptions & moptions)
{
    for(vector<OmRefCntPtr<SingleMatch> >::iterator i = leaves.begin();
	i != leaves.end(); i++) {
	(*i)->set_options(moptions);
    }

    mcmp = moptions.get_sort_comparator();
}


om_weight
MultiMatch::get_max_weight()
{
    // FIXME: DESTROY this method (put it as part of get_mset(), which we need
    // to create)
    Assert(leaves.size() > 0);

    // FIXME: this always asks the first database; make it pick one in some
    // way so that the load is fairly spread?
    om_weight result = 0;

    for (vector<OmRefCntPtr<SingleMatch> >::iterator i = leaves.begin();
	 i != leaves.end();
	 i++) {
	om_weight this_max = (*i)->get_max_weight();
	if(result < this_max) result = this_max;
    }

    return result;
}


void
MultiMatch::change_docids_to_global(vector<OmMSetItem> & mset,
				    om_doccount leaf_number)
{
    om_doccount number_of_leaves = leaves.size();
    vector<OmMSetItem>::iterator mset_item;
    for (mset_item = mset.begin();
	 mset_item != mset.end();
	 mset_item++) {
	mset_item->did = (mset_item->did - 1) * number_of_leaves + leaf_number;
    }
}

bool
MultiMatch::have_not_seen_key(set<OmKey> & collapse_entries,
			      const OmKey & new_key)
{
    if (new_key.value.size() == 0) return true;
    pair<set<OmKey>::iterator, bool> p = collapse_entries.insert(new_key);
    return p.second;
}

void
MultiMatch::merge_msets(vector<OmMSetItem> &mset,
			vector<OmMSetItem> &more_mset,
			om_doccount lastitem)
{
    // FIXME - this method is likely to be very inefficient
    // both because of the fact that we're doing a binary merge, and
    // because we collapse very inefficiently.  (Don't use fact that
    // each key can only occur once in each mset, for a start)
    DebugMsg("Merging mset of size " << more_mset.size() <<
	     " to existing set of size " << mset.size() <<
	     endl);

    vector<OmMSetItem> old_mset;
    old_mset.swap(mset);

    set<OmKey> collapse_entries;

    vector<OmMSetItem>::const_iterator i = old_mset.begin();
    vector<OmMSetItem>::const_iterator j = more_mset.begin();
    while(mset.size() < lastitem &&
	  i != old_mset.end() && j != more_mset.end()) {
	if(mcmp(*i, *j)) {
	    if (have_not_seen_key(collapse_entries, i->collapse_key))
		mset.push_back(*i);
	    i++;
	} else {
	    if (have_not_seen_key(collapse_entries, j->collapse_key))
		mset.push_back(*j);
	    j++;
	}
    }
    while(mset.size() < lastitem &&
	  i != old_mset.end()) {
	if (have_not_seen_key(collapse_entries, i->collapse_key))
	    mset.push_back(*i);
	i++;
    }
    while(mset.size() < lastitem &&
	  j != more_mset.end()) {
	if (have_not_seen_key(collapse_entries, j->collapse_key))
	    mset.push_back(*j);
	j++;
    }
}

bool
MultiMatch::add_next_sub_mset(SingleMatch * leaf,
			      om_doccount leaf_number,
			      om_doccount lastitem,
			      const OmMatchDecider *mdecider,
			      OmMSet & mset,
			      bool nowait)
{
    OmMSet sub_mset;

    // Get next mset
    if (leaf->get_mset(0, lastitem, sub_mset.items, &(sub_mset.mbound),
			  &(sub_mset.max_attained), mdecider, nowait)) {
	// Merge stats
	mset.mbound += sub_mset.mbound;
	if(sub_mset.max_attained > mset.max_attained)
	    mset.max_attained = sub_mset.max_attained;

	// Merge items
	change_docids_to_global(sub_mset.items, leaf_number);
	merge_msets(mset.items, sub_mset.items, lastitem);

	return true;
    }
    return false;
}

void
MultiMatch::prepare_matchers()
{
    bool prepared;
    bool nowait = true;
    do {
	prepared = true;
	for(vector<OmRefCntPtr<SingleMatch> >::iterator leaf = leaves.begin();
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
MultiMatch::collect_msets(om_doccount lastitem,
			  const OmMatchDecider *mdecider,
			  OmMSet & mset)
{
    // Empty the mset
    mset.items.clear();
    mset.mbound = 0;
    mset.max_attained = 0;
    mset.firstitem = 0;

    vector<bool> mset_received(leaves.size(), false);
    vector<OmRefCntPtr<SingleMatch> >::size_type msets_received = 0;

    om_doccount leaf_number;
    vector<OmRefCntPtr<SingleMatch> >::iterator leaf;

    // Get msets one by one, and merge each one with the current mset.
    // FIXME: this approach may be very inefficient - needs attention.
    bool nowait = true;
    while (msets_received != leaves.size()) {
	for(leaf = leaves.begin(),
	    leaf_number = 1;
	    leaf != leaves.end();
	    leaf++, leaf_number++) {

	    if (mset_received[leaf_number - 1]) {
		continue;
	    }

	    if (add_next_sub_mset((*leaf).get(),
				  leaf_number,
				  lastitem,
				  mdecider,
				  mset,
				  nowait)) {
		msets_received++;
		mset_received[leaf_number - 1] = true;
	    }
	}

	// Use blocking IO on subsequent passes, so that we don't go into
	// a tight loop.
	nowait = false;
    }
}

void
MultiMatch::remove_leading_elements(om_doccount number_to_remove,
				    OmMSet & mset)
{
    // Clear unwanted leading elements.
    if(number_to_remove != 0) {
	if(mset.items.size() < number_to_remove) {
	    mset.items.clear();
	} else {
	    mset.items.erase(mset.items.begin(),
			     mset.items.begin() + number_to_remove);
	}
	mset.firstitem += number_to_remove;
    }
}

void
MultiMatch::match(om_doccount first,
		  om_doccount maxitems,
		  OmMSet & mset,
		  const OmMatchDecider *mdecider)
{
    Assert(leaves.size() > 0);

    if(leaves.size() == 1) {
	// Only one mset to get - so get it, and block.
	leaves.front()->get_mset(first, maxitems, mset.items,
				 &(mset.mbound), &(mset.max_attained),
				 mdecider, false);
	mset.firstitem = first;
	mset.max_possible = get_max_weight();
    } else if(leaves.size() > 1) {
	// Need to merge msets.
	collect_msets(first + maxitems, mdecider, mset);
	remove_leading_elements(first, mset);

	// FIXME: get from sub msets (need to make them set it correctly first)
	mset.max_possible = get_max_weight();
    }
}
