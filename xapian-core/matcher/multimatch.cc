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
#include "omdocumentinternal.h"

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

#include "../api/omdatabaseinternal.h"

#include "match.h"
#include "stats.h"
#include "irweight.h"

#include <algorithm>
#include "autoptr.h"
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
		       const OmQuery::Internal * query,
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

    for (std::set<om_docid>::const_iterator reldoc = omrset.internal->items.begin();
	 reldoc != omrset.internal->items.end(); reldoc++) {
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

    gatherer->set_global_stats(omrset.size());
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
			 std::map<om_termname, OmMSet::Internal::TermFreqAndWeight> & termfreqandwts,
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
	pl = new MergePostList(v, this, errorhandler);
    }

    termfreqandwts = leaves.front()->get_term_info();
    
    return pl;
}

inline OmKey
MultiMatch::get_collapse_key(PostList *pl, const OmDatabase &db, om_docid did,
			     om_keyno keyno, RefCntPtr<Document> &doc)
{		      
    const OmKey *key = pl->get_collapse_key();
    if (key) return *key;
    if (doc.get() == 0) {
	RefCntPtr<Document> temp(OmDatabase::InternalInterface::get(db)->open_document(did));
	doc = temp;
    }
    return doc->get_key(keyno);
}

om_weight
MultiMatch::getorrecalc_maxweight(PostList *pl)
{
    om_weight wt;
    if (recalculate_w_max) {
	DEBUGLINE(MATCH, "recalculating max weight");
	wt = pl->recalc_maxweight();
	recalculate_w_max = false;
    } else {
	wt = pl->get_maxweight();
	AssertParanoid(fabs(wt - pl->recalc_maxweight()) < 1e-9);
    }
    DEBUGLINE(MATCH, "max possible doc weight = " << wt);
    return wt;
}

void
MultiMatch::get_mset(om_doccount first, om_doccount maxitems,
		     OmMSet & mset, const OmMatchDecider *mdecider,
		     OmErrorHandler * errorhandler,
		     void (*snooper)(const OmMSetItem &))
{
    std::map<om_termname, OmMSet::Internal::TermFreqAndWeight> termfreqandwts;
    PostList *pl = get_postlist(first, maxitems, termfreqandwts, errorhandler);
    get_mset_2(pl, termfreqandwts, first, maxitems, mset, mdecider, snooper);
}

void
MultiMatch::get_mset_2(PostList *pl, 
		       std::map<om_termname, OmMSet::Internal::TermFreqAndWeight> & termfreqandwts,
		       om_doccount first, om_doccount maxitems,
		       OmMSet & mset, const OmMatchDecider *mdecider,
		       void (*snooper)(const OmMSetItem &))
{
    DEBUGLINE(MATCH, "pl = (" << pl->get_description() << ")");

    // Empty result set
    om_doccount docs_considered = 0;
    om_weight greatest_wt = 0;
    std::vector<OmMSetItem> items;

    // maximum weight a document could possibly have
    const om_weight max_weight = pl->recalc_maxweight();

    DEBUGLINE(MATCH, "pl = (" << pl->get_description() << ")");
    recalculate_w_max = false;

    om_doccount matches_upper_bound = pl->get_termfreq_max();
    om_doccount matches_lower_bound = pl->get_termfreq_min();
    om_doccount matches_estimated   = pl->get_termfreq_est();

    // Check if any results have been asked for (might just be wanting
    // maxweight)
    if (maxitems == 0) {
	delete pl;
	mset = OmMSet(new OmMSet::Internal(first,
					   matches_upper_bound,
					   matches_lower_bound,
					   matches_estimated,
					   max_weight, greatest_wt, items,
					   termfreqandwts));
	return;
    }

    // Set max number of results that we want - this is used to decide
    // when to throw away unwanted items.
    om_doccount max_msize = first + maxitems;

    // Set the minimum item, used to compare against to see if an item
    // should be considered for the mset.
    OmMSetItem min_item(0, 0);

    {
	int val = opts.get_int("match_percent_cutoff", 0);
	if (val > 0) {
	    min_item.wt = val * max_weight / 100;
	}
    }

    {
	om_weight val = opts.get_real("match_cutoff", 0);
	if (val > 0) {
	    if (min_item.wt < val) {
		min_item.wt = val;
	    }
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

    // We form the mset in two stages.  In the first we fill up our working
    // mset.  Adding a new document does not remove another.  In the
    // second, we consider documents which may be better than documents
    // already in the mset.  Each document added expels another already in
    // the mset.
    if (min_item.wt <= 0) {
	while (items.size() < max_msize) {
	    next_handling_prune(pl, min_item.wt, this);

	    if (pl->at_end()) break;
	    
	    docs_considered++;
	    
	    om_docid did = pl->get_docid();
	    
	    RefCntPtr<Document> doc;
	    
	    // Use the decision functor if any.
	    // FIXME: if results are from MSetPostList then we can omit this
	    // step
	    if (mdecider != NULL) {
		if (doc.get() == 0) {
		    RefCntPtr<Document> temp(OmDatabase::InternalInterface::get(db)->open_document(did));
		    doc = temp;
		}
		OmDocument mydoc(new OmDocument::Internal(doc, db, did));
		if (!mdecider->operator()(mydoc)) continue;
	    }
	    
	    om_weight wt = pl->get_weight();
	    OmMSetItem new_item(wt, did);
	    DEBUGLINE(MATCH, "Candidate document id " << did << " wt " << wt);

	    bool pushback = true;
	    
	    // Perform collapsing on key if requested.
	    if (do_collapse) {
		new_item.collapse_key =
		    get_collapse_key(pl, db, did, collapse_key, doc);

		// Don't collapse on null key
		if (!new_item.collapse_key.value.empty()) {
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
			    continue;
			}
			// This is best match with this key so far:
			// remove the old one from the MSet
			om_docid olddid = olditem.did;
			DEBUGLINE(MATCH, "collapsem: removing " << olddid <<
				  ": " << new_item.collapse_key.value);
			std::vector<OmMSetItem>::iterator i;
			for (i = items.begin(); i->did != olddid; i++) {
			    Assert(i != items.end());
			}
			*i = new_item;
			pushback = false;
			oldkey->second = new_item;
		    }
		}
	    }
	    
	    // OK, actually add the item to the mset.
	    if (pushback) items.push_back(new_item);
	    
	    // Keep a track of the greatest weight we've seen.
	    if (wt > greatest_wt) greatest_wt = wt;

	    if (snooper) snooper(new_item);
	}
    }
    
    // We're done if this is a forward boolean match
    // (bodgetastic, FIXME better if we can)
    if (max_weight == 0) {
	if (opts.get_bool("match_sort_forward", true)) goto match_complete;
    }

    if (min_item.wt > 0 || items.size() >= max_msize) {
	// ie. we didn't go into the previous loop and exit because of at_end()
	while (1) {
	    if (recalculate_w_max) {
		if (getorrecalc_maxweight(pl) < min_item.wt) {
		    DEBUGLINE(MATCH, "*** TERMINATING EARLY (1)");
		    break;
		}
	    }

	    if (next_handling_prune(pl, min_item.wt, this)) {
		DEBUGLINE(MATCH, "*** REPLACING ROOT");
		
		// no need for a full recalc (unless we've got to do one because
		// of a prune elsewhere) - we're just switching to a subtree
		if (getorrecalc_maxweight(pl) < min_item.wt) {
		    DEBUGLINE(MATCH, "*** TERMINATING EARLY (2)");
		    break;
		}
	    }
	    
	    if (pl->at_end()) {
		DEBUGLINE(MATCH, "Reached end of potential matches");
		break;
	    }
	    
	    docs_considered++;
	    
	    om_docid did = pl->get_docid();
	    om_weight wt = pl->get_weight();
	    
	    DEBUGLINE(MATCH, "Candidate document id " << did << " wt " << wt);
	    OmMSetItem new_item(wt, did);
	    
	    // test if item has high enough weight to get into proto-mset
	    if (!mcmp(new_item, min_item)) continue;
	    
	    RefCntPtr<Document> doc;
	    
	    // Use the decision functor if any.
	    // FIXME: if results are from MSetPostList then we can omit this
	    // step
	    if (mdecider != NULL) {
		if (doc.get() == 0) {
		    RefCntPtr<Document> temp(OmDatabase::InternalInterface::get(db)->open_document(did));
		    doc = temp;
		}
		OmDocument mydoc(new OmDocument::Internal(doc, db, did));
		if (!mdecider->operator()(mydoc)) continue;
	    }
	    
	    bool pushback = true;

	    // Perform collapsing on key if requested.
	    if (do_collapse) {
		new_item.collapse_key =
		    get_collapse_key(pl, db, did, collapse_key, doc);

		// Don't collapse on null key
		if (!new_item.collapse_key.value.empty()) {
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
			    continue;
			}
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
			    for (i = items.begin(); i->did != olddid; i++) {
				Assert(i != items.end());
			    }
			    *i = new_item;
			    pushback = false;
			}
			oldkey->second = new_item;
		    }
		}
	    }
	    
	    // OK, actually add the item to the mset.
	    if (pushback) items.push_back(new_item);

	    // Keep a track of the greatest weight we've seen.
	    if (wt > greatest_wt) greatest_wt = wt;

	    if (snooper) snooper(new_item);
	    
	    // FIXME: find balance between larger size for more efficient
	    // nth_element and smaller size for better minimum weight
	    // optimisations
	    if (items.size() == max_msize * 2) {
		// find last element we care about
		DEBUGLINE(MATCH, "finding nth");
		std::nth_element(items.begin(), items.begin() + max_msize - 1,
				 items.end(), mcmp);
		// erase elements which don't make the grade
		items.erase(items.begin() + max_msize, items.end());
		min_item = items.back(); // Note that this item is the only one sorted
		DEBUGLINE(MATCH, "mset size = " << items.size());
		DEBUGLINE(MATCH, "min_item.wt = " << min_item.wt);
		if (getorrecalc_maxweight(pl) < min_item.wt) {
		    DEBUGLINE(MATCH, "*** TERMINATING EARLY (3)");
		    break;
		}
	    }
	}
    }

    match_complete:
    
    // done with posting list tree
    delete pl;
    
    DEBUGLINE(MATCH, items.size() << " items in potential mset");

    if (items.size() > max_msize) {
	// find last element we care about
	DEBUGLINE(MATCH, "finding nth");
	std::nth_element(items.begin(), items.begin() + max_msize - 1, items.end(), mcmp);
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

    DEBUGLINE(MATCH,
	      "msize = " << items.size() << ", " <<
	      "docs_considered = " << docs_considered << ", " <<
	      "matches_lower_bound = " << matches_lower_bound << ", " <<
	      "matches_estimated = " << matches_estimated << ", " <<
	      "matches_upper_bound = " << matches_upper_bound);

    if (items.size()) {
	DEBUGLINE(MATCH, "sorting");

	// Need a stable sort, but this is provided by comparison operator
	std::sort(items.begin(), items.end(), mcmp);

	DEBUGLINE(MATCH, "max weight in mset = " << items[0].wt <<
		  ", min weight in mset = " << items.back().wt);
    }

    Assert(matches_estimated >= matches_lower_bound);
    Assert(matches_estimated <= matches_upper_bound);

    Assert(docs_considered <= matches_upper_bound);
    if (docs_considered > matches_lower_bound)
	matches_lower_bound = docs_considered;
    if (docs_considered > matches_estimated)
	matches_estimated = docs_considered;

    mset = OmMSet(new OmMSet::Internal(first,
				       matches_upper_bound,
				       matches_lower_bound,
				       matches_estimated,
				       max_weight, greatest_wt, items,
				       termfreqandwts));
}

// This method is called by branch postlists when they rebalance
// in order to recalculate the weights in the tree
void
MultiMatch::recalc_maxweight()
{
    recalculate_w_max = true;
}
