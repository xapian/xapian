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
#include "leafmatch.h"

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
}

void
MultiMatch::add_leafmatch(LeafMatch * leaf)
{
    Assert(allow_add_leafmatch);

    leaves.push_back(leaf);
}

void
MultiMatch::set_query(const OmQueryInternal * query)
{
    Assert((allow_add_leafmatch = false) == false);
    for(vector<LeafMatch *>::iterator i = leaves.begin();
	i != leaves.end(); i++) {
	(*i)->set_query(query);
    }
}

void
MultiMatch::set_rset(RSet * rset)
{
    Assert((allow_add_leafmatch = false) == false);
    for(vector<LeafMatch *>::iterator i = leaves.begin();
	i != leaves.end(); i++) {
	(*i)->set_rset(rset);
    }
}

void
MultiMatch::set_min_weight_percent(int pcent)
{
    Assert((allow_add_leafmatch = false) == false);
    for(vector<LeafMatch *>::iterator i = leaves.begin();
	i != leaves.end(); i++) {
	(*i)->set_min_weight_percent(pcent);
    }
}

void
MultiMatch::set_collapse_key(om_keyno key)
{
    Assert((allow_add_leafmatch = false) == false);
    for(vector<LeafMatch *>::iterator i = leaves.begin();
	i != leaves.end(); i++) {
	(*i)->set_collapse_key(key);
    }
}

void
MultiMatch::set_no_collapse()
{
    Assert((allow_add_leafmatch = false) == false);
    for(vector<LeafMatch *>::iterator i = leaves.begin();
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
    for(vector<LeafMatch *>::iterator i = leaves.begin();
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

    om_doccount tot_mbound = 0;
    om_weight   tot_greatest_wt = 0;
    vector<OmMSetItem> tot_mset;

    (*(leaves.begin()))->match(0, first + maxitems, tot_mset,
			       cmp, &tot_mbound, &tot_greatest_wt, mdecider);

    if(leaves.size() > 1) {
	for(vector<LeafMatch *>::iterator i = leaves.begin() + 1;
	    i != leaves.end(); i++) {

	    om_doccount sub_mbound;
	    om_weight   sub_greatest_wt;
	    vector<OmMSetItem> sub_mset;

	    (*i)->match(0, first + maxitems, sub_mset,
			cmp, &sub_mbound, &sub_greatest_wt, mdecider);

	    tot_mbound += sub_mbound;
	    if(sub_greatest_wt > tot_greatest_wt) tot_greatest_wt = sub_greatest_wt;

	    DebugMsg("Merging mset of size " << sub_mset.size() <<
		     " to existing set of size " << tot_mset.size() << endl);

	    tot_mset = sub_mset;
	}
    }

    *mbound      = tot_mbound;
    *greatest_wt = tot_greatest_wt;

    if(first != 0) {
	if(tot_mset.size() < first) {
	    tot_mset.clear();
	} else if (first > 0) {
	    tot_mset.erase(tot_mset.begin(), tot_mset.begin() + first);
	}
    }

    mset = tot_mset;
}


// FIXME: this is a bad copy of match - factor out.
void
MultiMatch::boolmatch(om_doccount first,
		      om_doccount maxitems,
		      vector<OmMSetItem> & mset)
{
    Assert((allow_add_leafmatch = false) == false);
    Assert(leaves.size() > 0);

    vector<OmMSetItem> tot_mset;

    for(vector<LeafMatch *>::iterator i = leaves.begin();
	i != leaves.end(); i++) {

	vector<OmMSetItem> sub_mset;

	(*i)->boolmatch(0, first + maxitems, sub_mset);

	tot_mset = sub_mset;
    }

    if(first != 0) {
	if(tot_mset.size() < first) {
	    tot_mset.clear();
	} else if (first > 0) {
	    tot_mset.erase(tot_mset.begin(), tot_mset.begin() + first);
	}
    }

    mset = tot_mset;
}
