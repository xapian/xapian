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
#include "rset.h"

#include <algorithm>

////////////////////////////////////
// Initialisation and cleaning up //
////////////////////////////////////

MultiMatch::MultiMatch()
#ifdef MUS_DEBUG
	: allow_add_leafmatch(true)
#endif /* MUS_DEBUG */
{
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
MultiMatch::add_singlematch(auto_ptr<SingleMatch> smatch)
{
    Assert(allow_add_leafmatch);

    SingleMatch *temp = smatch.get();
    leaves.push_back(temp);
    smatch.release(); // the leaves container now owns the pointer.

    temp->link_to_multi(&gatherer);
}

void
MultiMatch::set_query(const OmQueryInternal * query)
{
    Assert((allow_add_leafmatch = false) == false);
    for(vector<SingleMatch *>::iterator i = leaves.begin();
	i != leaves.end(); i++) {
	(*i)->set_query(query);
    }
}

void
MultiMatch::set_rset(auto_ptr<RSet> rset_)
{
    Assert((allow_add_leafmatch = false) == false);

    rset = rset_;
    for(vector<SingleMatch *>::iterator i = leaves.begin();
	i != leaves.end(); i++) {
	(*i)->set_rset(rset.get());
    }

    gatherer.set_global_stats(rset->get_rsize());
}

void
MultiMatch::set_weighting(IRWeight::weight_type wt_type_)
{
    Assert((allow_add_leafmatch = false) == false);
    for(vector<SingleMatch *>::iterator i = leaves.begin();
	i != leaves.end(); i++) {
	(*i)->set_weighting(wt_type_);
    }
}


void
MultiMatch::set_min_weight_percent(int pcent)
{
    Assert((allow_add_leafmatch = false) == false);
    for(vector<SingleMatch *>::iterator i = leaves.begin();
	i != leaves.end(); i++) {
	(*i)->set_min_weight_percent(pcent);
    }
}

void
MultiMatch::set_collapse_key(om_keyno key)
{
    Assert((allow_add_leafmatch = false) == false);
    for(vector<SingleMatch *>::iterator i = leaves.begin();
	i != leaves.end(); i++) {
	(*i)->set_collapse_key(key);
    }
}

void
MultiMatch::set_no_collapse()
{
    Assert((allow_add_leafmatch = false) == false);
    for(vector<SingleMatch *>::iterator i = leaves.begin();
	i != leaves.end(); i++) {
	(*i)->set_no_collapse();
    }
}


om_weight
MultiMatch::get_max_weight()
{
    Assert((allow_add_leafmatch = false) == false);
    Assert(leaves.size() > 0);

    om_weight result = leaves.front()->get_max_weight();

#ifdef MUS_DEBUG
    for(vector<SingleMatch *>::iterator i = leaves.begin();
	i != leaves.end(); i++) {
	Assert((*i)->get_max_weight() == result);
    }
#endif /* MUS_DEBUG */

    return result;
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
    Assert((allow_add_leafmatch = false) == false);
    Assert(leaves.size() > 0);

    if(leaves.size() == 1) {
	// Only one mset to get - so get it.
	(*(leaves.begin()))->match(first, maxitems, mset, cmp,
				   mbound, greatest_wt, mdecider);
    } else if(leaves.size() > 1) {
	// Need to merge msets.
	MSetCmp mcmp(cmp);

	om_doccount tot_mbound = 0;
	om_weight   tot_greatest_wt = 0;
	om_doccount lastitem = first + maxitems;

	// Get the first mset
	(*(leaves.begin()))->match(0, lastitem, mset, cmp,
				   &tot_mbound, &tot_greatest_wt, mdecider);

	if(leaves.size() > 1) {
	    // Get subsequent msets, and merge each one with the current mset
	    // FIXME: this approach may be very inefficient - needs attention.
	    for(vector<SingleMatch *>::iterator leaf = leaves.begin() + 1;
		leaf != leaves.end(); leaf++) {

		om_doccount sub_mbound;
		om_weight   sub_greatest_wt;
		vector<OmMSetItem> sub_mset;

		// Get next mset
		(*leaf)->match(0, lastitem, sub_mset, cmp,
			       &sub_mbound, &sub_greatest_wt, mdecider);

		DebugMsg("Merging mset of size " << sub_mset.size() <<
			 " to existing set of size " << mset.size() <<
			 endl);

		// Merge stats
		tot_mbound += sub_mbound;
		if(sub_greatest_wt > tot_greatest_wt)
		    tot_greatest_wt = sub_greatest_wt;

		// Merge msets: FIXME - this is likely to be very inefficient
		vector<OmMSetItem> old_mset;
		old_mset.swap(mset);

		vector<OmMSetItem>::const_iterator i = old_mset.begin();
		vector<OmMSetItem>::const_iterator j = sub_mset.begin();
		while(mset.size() < lastitem &&
		      i != old_mset.end() && j != sub_mset.end()) {
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
		      j != sub_mset.end()) {
		    mset.push_back(*j++);
		}
	    }
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
