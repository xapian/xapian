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
#include "omdatabaseinterface.h"

#include "andpostlist.h"
#include "orpostlist.h"
#include "xorpostlist.h"
#include "andnotpostlist.h"
#include "andmaybepostlist.h"
#include "filterpostlist.h"
#include "phrasepostlist.h"
#include "emptypostlist.h"
#include "leafpostlist.h"
#include "mergepostlist.h"
#include "msetpostlist.h"

#ifdef MUS_BUILD_BACKEND_REMOTE
#include "networkmatch.h"
#endif /* MUS_BUILD_BACKEND_REMOTE */

#include "document.h"
#include "rset.h"
#include "omqueryinternal.h"
#include "omdocumentparams.h"

#include "../api/omdatabaseinternal.h"

#include "match.h"
#include "stats.h"
#include "irweight.h"

#include <algorithm>
#include "om/autoptr.h"
#include <queue>

class OmErrorHandler;

// Comparison which sorts equally weighted MSetItems in docid order
bool msetcmp_forward(const OmMSetItem &a, const OmMSetItem &b) {
    if(a.wt > b.wt) return true;
    if(a.wt == b.wt) return a.did < b.did;
    return false;
}

// Comparison which sorts equally weighted MSetItems in reverse docid order
bool msetcmp_reverse(const OmMSetItem &a, const OmMSetItem &b) {
    if(a.wt > b.wt) return true;
    if(a.wt == b.wt) return a.did > b.did;
    return false;
}

////////////////////////////////////
// Initialisation and cleaning up //
////////////////////////////////////

MultiMatch::MultiMatch(const OmDatabase &db_,
		       const OmQueryInternal * query,
		       const OmRSet & omrset,
		       const OmSettings & opts_,
		       AutoPtr<StatsGatherer> gatherer_)
	: gatherer(gatherer_), db(db_), opts(opts_), mcmp(msetcmp_forward)
{
    // FIXME: has this check been done already?
    // Check that we have a valid query to run
    if (!query->isdefined) {
	throw OmInvalidArgumentError("Query is not defined.");
    }

    OmDatabase::Internal * internal = OmDatabase::InternalInterface::get(db);
    om_doccount number_of_leaves = internal->databases.size();
    std::vector<OmRSet> subrsets(number_of_leaves);

    for (std::set<om_docid>::const_iterator reldoc = omrset.items.begin();
	 reldoc != omrset.items.end(); reldoc++) {
	om_doccount local_docid = ((*reldoc) - 1) / number_of_leaves + 1;
	om_doccount subdatabase = ((*reldoc) - 1) % number_of_leaves;
	subrsets[subdatabase].add_document(local_docid);
    }
    
    std::vector<OmRSet>::const_iterator subrset = subrsets.begin();

    std::vector<RefCntPtr<Database> >::iterator i;
    for (i = internal->databases.begin(); i != internal->databases.end(); ++i) {
	Assert(subrset != subrsets.end());
	Database *db = (*i).get();
	Assert(db);
	RefCntPtr<SubMatch> smatch;
	// There is currently only one special case, for network databases.
	if (db->is_network()) {
#ifdef MUS_BUILD_BACKEND_REMOTE
	    smatch = RefCntPtr<SubMatch>(new RemoteSubMatch(db, query, *subrset, opts, gatherer.get()));
#else /* MUS_BUILD_BACKEND_REMOTE */
	    throw OmUnimplementedError("Network operation is not available");
#endif /* MUS_BUILD_BACKEND_REMOTE */
	} else {
	    smatch = RefCntPtr<SubMatch>(new LocalSubMatch(db, query, *subrset, opts, gatherer.get()));
	}
	leaves.push_back(smatch);
	subrset++;
    }
    Assert(subrset == subrsets.end());

    gatherer->set_global_stats(omrset.items.size());
    prepare_matchers();

    if (!opts.get_bool("match_sort_forward", true)) {
	mcmp = OmMSetCmp(msetcmp_reverse);
    }
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
	std::vector<RefCntPtr<SubMatch> >::iterator leaf;
	for (leaf = leaves.begin(); leaf != leaves.end(); leaf++) {
	    if (!(*leaf)->prepare_match(nowait)) prepared = false;
	}
	// Use blocking IO on subsequent passes, so that we don't go into
	// a tight loop.
	nowait = false;
    } while (!prepared);
}

PostList *
MultiMatch::get_postlist(om_doccount first, om_doccount maxitems,
			 std::map<om_termname, OmMSet::TermFreqAndWeight> & termfreqandwts,
			 OmErrorHandler * errorhandler)
{
    Assert(!leaves.empty());

    {
	std::vector<RefCntPtr<SubMatch> >::iterator leaf;
	for (leaf = leaves.begin(); leaf != leaves.end(); leaf++) {
	    (*leaf)->start_match(first + maxitems);
	}
    }

    PostList *pl;
    if (leaves.size() == 1) {
	// Only one mset to get - so get it
	pl = leaves.front()->get_postlist(first + maxitems, this);
    } else {
	std::vector<PostList *> v;
	std::vector<RefCntPtr<SubMatch> >::iterator i;
	for (i = leaves.begin(); i != leaves.end(); i++) {
	    v.push_back((*i)->get_postlist(first + maxitems, this));
	}
	pl = new MergePostList(v, errorhandler);
    }

    termfreqandwts = leaves.front()->get_term_info();
    
    return pl;
}

void
MultiMatch::get_mset(om_doccount first, om_doccount maxitems,
		     OmMSet & mset, const OmMatchDecider *mdecider,
		     OmErrorHandler * errorhandler)
{
    Assert(!leaves.empty());

    {
	std::vector<RefCntPtr<SubMatch> >::iterator leaf;
	for (leaf = leaves.begin(); leaf != leaves.end(); leaf++) {
	    (*leaf)->start_match(first + maxitems);
	}
    }
    
    PostList *pl;
    if (leaves.size() == 1) {
	// Only one mset to get - so get it
	pl = leaves.front()->get_postlist(first + maxitems, this);
    } else {
	std::vector<PostList *> v;
	std::vector<RefCntPtr<SubMatch> >::iterator i;
	for (i = leaves.begin(); i != leaves.end(); i++) {
	    v.push_back((*i)->get_postlist(first + maxitems, this));
	}
	pl = new MergePostList(v, errorhandler);
    }

    const std::map<om_termname, OmMSet::TermFreqAndWeight> &termfreqandwts =
	leaves.front()->get_term_info();

    DEBUGLINE(MATCH, "pl = (" << pl->get_description() << ")");

    // Empty result set
    om_doccount mbound = 0;
    om_weight greatest_wt = 0;
    std::vector<OmMSetItem> items;

    // maximum weight a document could possibly have
    const om_weight max_weight = pl->recalc_maxweight();

    recalculate_w_max = false;

    // Check if any results have been asked for (might just be wanting
    // maxweight)
    if (maxitems == 0) {
	delete pl;

	mset = OmMSet(first, mbound, max_weight, greatest_wt, items,
		      termfreqandwts);

	return;
    }

    // Set max number of results that we want - this is used to decide
    // when to throw away unwanted items.
    om_doccount max_msize = first + maxitems;

    // Set the minimum item, used to compare against to see if an item
    // should be considered for the mset.
    //
    // NB set weight to -1 so that all documents pass test until we
    // have an mset full - if initial weight is 0, we reject any
    // documents with weight 0 if we use msetcmp_forward.
    OmMSetItem min_item(-1, 0);

    {
	int val = opts.get_int("match_percent_cutoff", 0);
	if (val > 0) {
	    min_item.wt = val * max_weight / 100;
	}
    }

    // Table of keys which have been seen already, for collapsing.
    std::map<OmKey, OmMSetItem> collapse_tab;

    // Whether to perform collapse operation
    bool do_collapse = false;
    // Key to collapse on, if desired
    om_keyno collapse_key;
    {
	int val = opts.get_int("match_collapse_key", -1);
	if (val >= 0) {
	    do_collapse = true;
	    collapse_key = val;
	}
    }

    // Perform query
    while (1) {
	if (recalculate_w_max) {
	    recalculate_w_max = false;
	    om_weight w_max = pl->recalc_maxweight();
	    DEBUGLINE(MATCH, "max possible doc weight = " << w_max);
	    if (w_max < min_item.wt) {
		DEBUGLINE(MATCH, "*** TERMINATING EARLY (1)");
		break;
	    }
	}

	PostList *ret = pl->next(min_item.wt);
        if (ret) {
	    DEBUGLINE(MATCH, "*** REPLACING ROOT");
	    delete pl;
	    pl = ret;

	    // no need for a full recalc (unless we've got to do one because
	    // of a prune elsewhere) - we're just switching to a subtree
	    om_weight w_max = pl->get_maxweight();
	    DEBUGLINE(MATCH, "max possible doc weight = " << w_max);
            AssertParanoid(recalculate_w_max || fabs(w_max - pl->recalc_maxweight()) < 1e-9);

	    if (w_max < min_item.wt) {
		DEBUGLINE(MATCH, "*** TERMINATING EARLY (2)");
		break;
	    }
	}

	if (pl->at_end()) break;

        mbound++;

	om_docid did = pl->get_docid();
        om_weight wt = pl->get_weight();

	DEBUGLINE(MATCH, "Candidate document id " << did << " wt " << wt);
	OmMSetItem new_item(wt, did);

	// test if item has high enough weight to get into proto-mset
	if (!mcmp(new_item, min_item)) continue;

	RefCntPtr<LeafDocument> irdoc;

	// Use the decision functor if any.
	// FIXME: if results are from MSetPostList then we can omit this
	// step
	if (mdecider != NULL) {
	    if (irdoc.get() == 0) {
		RefCntPtr<LeafDocument> temp(OmDatabase::InternalInterface::get(db)->open_document(did));
		irdoc = temp;
	    }
	    OmDocument mydoc(irdoc);
	    if (!mdecider->operator()(mydoc)) continue;
	}

	// Perform collapsing on key if requested.
	if (do_collapse) {
	    const OmKey *key = pl->get_collapse_key();
	    if (key) {
		new_item.collapse_key = *key;
	    } else {
		if (irdoc.get() == 0) {
		    RefCntPtr<LeafDocument> temp(OmDatabase::InternalInterface::get(db)->open_document(did));
		    irdoc = temp;
		}
		new_item.collapse_key = irdoc.get()->get_key(collapse_key);
	    }
	    if (!perform_collapse(items, collapse_tab, did, new_item, min_item))
		continue;
	}

	// OK, actually add the item to the mset.
	items.push_back(new_item);

	// Keep a track of the greatest weight we've seen.
	if (wt > greatest_wt) greatest_wt = wt;

	// FIXME: find balance between larger size for more efficient
	// nth_element and smaller size for better minimum weight
	// optimisations
	if (items.size() == max_msize * 2) {
	    // find last element we care about
	    DEBUGLINE(MATCH, "finding nth");
	    std::nth_element(items.begin(), items.begin() + max_msize,
			     items.end(), mcmp);
	    // erase elements which don't make the grade
	    items.erase(items.begin() + max_msize, items.end());
	    min_item = items.back();
	    DEBUGLINE(MATCH, "mset size = " << items.size());
	}
    }

    // done with posting list tree
    delete pl;
    
    if (items.size() > max_msize) {
	// find last element we care about
	DEBUGLINE(MATCH, "finding nth");
	std::nth_element(items.begin(), items.begin() + max_msize, items.end(), mcmp);
	// erase elements which don't make the grade
	items.erase(items.begin() + max_msize, items.end());
    }

    if (first > 0) {
	// Remove unwanted leading entries
	if (items.size() <= first) {
	    items.clear();
	} else {
	    DEBUGLINE(MATCH, "finding " << first << "th");
	    std::nth_element(items.begin(), items.begin() + first, items.end(), mcmp);
	    // erase the leading ``first'' elements
	    items.erase(items.begin(), items.begin() + first);
	}
    }

    DEBUGLINE(MATCH, "msize = " << items.size() << ", mbound = " << mbound);
    if (items.size()) {
	DEBUGLINE(MATCH, "sorting");

	// Need a stable sort, but this is provided by comparison operator
	std::sort(items.begin(), items.end(), mcmp);

	DEBUGLINE(MATCH, "max weight in mset = " << items[0].wt <<
		  ", min weight in mset = " << items.back().wt);
    }

    mset = OmMSet(first, mbound, max_weight, greatest_wt, items,
		  termfreqandwts);
}

// This method is called by branch postlists when they rebalance
// in order to recalculate the weights in the tree
void
MultiMatch::recalc_maxweight()
{
    recalculate_w_max = true;
}


// Internal method to perform the collapse operation
inline bool
MultiMatch::perform_collapse(std::vector<OmMSetItem> &mset,
			     std::map<OmKey, OmMSetItem> &collapse_tab,
			     om_docid did,
			     const OmMSetItem &new_item,
			     const OmMSetItem &min_item)
{
    // Don't collapse on null key
    if (new_item.collapse_key.value.size() == 0) return true;

    bool add_item = true;
    std::map<OmKey, OmMSetItem>::iterator oldkey;
    oldkey = collapse_tab.find(new_item.collapse_key);
    if (oldkey == collapse_tab.end()) {
	DEBUGLINE(MATCH, "collapsem: new key: " << new_item.collapse_key.value);
	// Key not been seen before
	collapse_tab.insert(std::make_pair(new_item.collapse_key, new_item));
    } else {
	const OmMSetItem olditem = (*oldkey).second;
	if (mcmp(olditem, new_item)) {
	    DEBUGLINE(MATCH, "collapsem: better exists: " <<
		      new_item.collapse_key.value);
	    // There's already a better match with this key
	    add_item = false;
	} else {
	    // This is best match with this key so far:
	    // remove the old one from the MSet
	    if (mcmp(olditem, min_item)) {
		// Old one hasn't fallen out of MSet yet
		// Scan through (unsorted) MSet looking for entry
		// FIXME: more efficient way than just scanning?
		om_docid olddid = olditem.did;
		DEBUGLINE(MATCH, "collapsem: removing " << olddid <<
			  ": " << new_item.collapse_key.value);
		std::vector<OmMSetItem>::iterator i;
		for (i = mset.begin(); i->did != olddid; i++) {
		    Assert(i != mset.end());
		}
		mset.erase(i);
	    }
	    oldkey->second = new_item;
	}
    }
    return add_item;
}
