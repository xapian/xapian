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
#include "multi_database.h"
#include "omdebug.h"
#include "omenquireinternal.h"

#ifdef MUS_BUILD_BACKEND_REMOTE
#include "networkmatch.h"
#endif /* MUS_BUILD_BACKEND_REMOTE */

#include <algorithm>

////////////////////////////////////
// Initialisation and cleaning up //
////////////////////////////////////

MultiMatch::MultiMatch(const MultiDatabase * multi_database_,
		       const OmQueryInternal * query,
		       const OmRSet & omrset,
		       const OmSettings & moptions,
		       AutoPtr<StatsGatherer> gatherer_)
	: multi_database(multi_database_),
	  gatherer(gatherer_),
	  mcmp(msetcmp_forward)
{
    std::vector<RefCntPtr<Database> >::const_iterator db;
    for (db = multi_database->databases.begin();
	 db != multi_database->databases.end();
	 ++db) {
	RefCntPtr<SingleMatch> smatch(make_match_from_database(db->get()));
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
    } else {
	return RefCntPtr<SingleMatch>(new LocalMatch(db));
    }
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

    // FIXME: same code as in localmatch.cc...
    if (moptions.get_bool("match_sort_forward", true)) {
	mcmp = OmMSetCmp(msetcmp_forward);
    } else {
	mcmp = OmMSetCmp(msetcmp_reverse);
    }
}

void
MultiMatch::change_docids_to_global(std::vector<OmMSetItem> & mset,
				    om_doccount leaf_number)
{
    om_doccount number_of_leaves = leaves.size();
    std::vector<OmMSetItem>::iterator mset_item;
    for (mset_item = mset.begin();
	 mset_item != mset.end();
	 mset_item++) {
	mset_item->did = (mset_item->did - 1) * number_of_leaves + leaf_number;
    }
}

bool
MultiMatch::have_not_seen_key(std::set<OmKey> & collapse_entries,
			      const OmKey & new_key)
{
    if (new_key.value.size() == 0) return true;
    std::pair<std::set<OmKey>::iterator, bool> p = collapse_entries.insert(new_key);
    return p.second;
}

void
MultiMatch::merge_msets(std::vector<OmMSetItem> &mset,
			std::vector<OmMSetItem> &more_mset,
			om_doccount lastitem)
{
    // FIXME - this method is likely to be very inefficient
    // both because of the fact that we're doing a binary merge, and
    // because we collapse very inefficiently.  (Don't use fact that
    // each key can only occur once in each mset, for a start)
    DEBUGLINE(MATCH, "Merging mset of size " << more_mset.size() <<
	      " to existing set of size " << mset.size());

    std::vector<OmMSetItem> old_mset;
    old_mset.swap(mset);

    std::set<OmKey> collapse_entries;

    std::vector<OmMSetItem>::const_iterator i = old_mset.begin();
    std::vector<OmMSetItem>::const_iterator j = more_mset.begin();
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
    if (leaf->get_mset(0, lastitem, sub_mset,
			  mdecider, nowait)) {

	// Merge stats
	mset.mbound += sub_mset.mbound;
	if(sub_mset.max_attained > mset.max_attained)
	    mset.max_attained = sub_mset.max_attained;
	if(sub_mset.max_possible > mset.max_possible)
	    mset.max_possible = sub_mset.max_possible;

	// Merge items
	change_docids_to_global(sub_mset.items, leaf_number);
	merge_msets(mset.items, sub_mset.items, lastitem);

	// Merge term information
	std::map<om_termname, OmMSet::TermFreqAndWeight> *msettermfreqandwts =
		&OmMSet::InternalInterface::get_termfreqandwts(mset);
	std::map<om_termname, OmMSet::TermFreqAndWeight> *sub_msettermfreqandwts =
		&OmMSet::InternalInterface::get_termfreqandwts(sub_mset);

	if(msettermfreqandwts->size() == 0) {
	    *msettermfreqandwts = *sub_msettermfreqandwts;
	} else {
	    std::map<om_termname, OmMSet::TermFreqAndWeight>::iterator i;
	    std::map<om_termname, OmMSet::TermFreqAndWeight>::const_iterator j;

	    for (i = msettermfreqandwts->begin(),
		 j = sub_msettermfreqandwts->begin();
		 i != msettermfreqandwts->end() &&
		 j != sub_msettermfreqandwts->end();
		 i++, j++) {
		if(i->second.termweight == 0 && i->second.termfreq != 0) {
		    DEBUGLINE(WTCALC, "termweight of `" << i->first <<
			      "' in first mset is 0, setting to " <<
			      j->second.termweight);
		    i->second.termweight = j->second.termweight;
		}
	    }

#ifdef MUS_DEBUG_PARANOID
	    AssertParanoid(msettermfreqandwts->size() ==
			   sub_msettermfreqandwts->size());
	    for (i = msettermfreqandwts->begin(),
		 j = sub_msettermfreqandwts->begin();
		 i != msettermfreqandwts->end() &&
		 j != sub_msettermfreqandwts->end();
		 i++, j++) {
		DebugMsg("Comparing " <<
			 i->first << "," <<
			 i->second.termfreq << "," <<
			 i->second.termweight << " with " <<
			 j->first << "," <<
			 j->second.termfreq << "," <<
			 j->second.termweight);

		AssertParanoid(i->first == j->first);
		AssertParanoid(i->second.termfreq == j->second.termfreq);
		AssertParanoid(i->second.termweight == j->second.termweight ||
			       j->second.termweight == 0);
	    }
#endif /* MUS_DEBUG_PARANOID */
	}

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
MultiMatch::collect_msets(om_doccount lastitem,
			  const OmMatchDecider *mdecider,
			  OmMSet & mset)
{
    // Empty the mset
    mset.items.clear();
    mset.mbound = 0;
    mset.max_attained = 0;
    mset.max_possible = 0;
    mset.firstitem = 0;

    std::vector<bool> mset_received(leaves.size(), false);
    std::vector<RefCntPtr<SingleMatch> >::size_type msets_received = 0;

    om_doccount leaf_number;
    std::vector<RefCntPtr<SingleMatch> >::iterator leaf;

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
MultiMatch::get_mset(om_doccount first, om_doccount maxitems,
		     OmMSet & mset,
		     const OmMatchDecider *mdecider)
{
    Assert(leaves.size() > 0);

    if(leaves.size() == 1) {
	// Only one mset to get - so get it, and block.
	leaves.front()->get_mset(first, maxitems, mset,
				 mdecider, false);
    } else if(leaves.size() > 1) {
	// Need to merge msets.
	collect_msets(first + maxitems, mdecider, mset);
	remove_leading_elements(first, mset);
    }
}

