/* multimatch.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003 Olly Betts
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

#include <config.h>
#include "multimatch.h"
#include "match.h"
#include "localmatch.h"
#include "emptymatch.h"
#include "rset.h"
#include "omdebug.h"
#include "omenquireinternal.h"
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
#include "biaspostlist.h"

#ifdef MUS_BUILD_BACKEND_REMOTE
#include "networkmatch.h"
#endif /* MUS_BUILD_BACKEND_REMOTE */

#include "document.h"
#include "rset.h"
#include "omqueryinternal.h"

#include "../api/omdatabaseinternal.h"

#include "match.h"
#include "stats.h"

#include <algorithm>
#include "autoptr.h"
#include <queue>
#include <vector>
#include <map>
#include <set>

using namespace std;

class OmErrorHandler;

// Comparison which sorts equally weighted MSetItems in docid order
bool
msetcmp_forward(const OmMSetItem &a, const OmMSetItem &b)
{
    if (a.wt > b.wt) return true;
    if (a.wt < b.wt) return false;
    // two special cases to make min_item compares work when did == 0
    if (a.did == 0) return false;
    if (b.did == 0) return true;
    return (a.did < b.did);
}

// Comparison which sorts equally weighted MSetItems in reverse docid order
bool
msetcmp_reverse(const OmMSetItem &a, const OmMSetItem &b)
{
    if (a.wt > b.wt) return true;
    if (a.wt < b.wt) return false;
    return (a.did > b.did);
}

class MSetSortCmp {
    private:
	OmDatabase db;
	double factor;
	om_valueno sort_key;
	bool forward;
	int bands;
    public:
	MSetSortCmp(const OmDatabase &db_, int bands_, double percent_scale,
		    om_valueno sort_key_, bool forward_)
	    : db(db_), factor(percent_scale * bands_ / 100.0),
	      sort_key(sort_key_), forward(forward_), bands(bands_) {
	}
	bool operator()(const OmMSetItem &a, const OmMSetItem &b) const {
	    int band_a = int(ceil(a.wt * factor));
	    int band_b = int(ceil(b.wt * factor));
	    if (band_a > bands) band_a = bands;
	    if (band_b > bands) band_b = bands;
	    
	    if (band_a != band_b) return band_a > band_b;
	    if (sort_key != om_valueno(-1)) {
		if (a.sort_key.empty()) {
		    OmDocument doc = db.get_document(a.did);
		    a.sort_key = doc.get_value(sort_key);
		}
		if (b.sort_key.empty()) {
		    OmDocument doc = db.get_document(b.did);
		    b.sort_key = doc.get_value(sort_key);
		}
		// "bigger is better"
		if (a.sort_key > b.sort_key) return true;
		if (a.sort_key < b.sort_key) return false;
	    }
	    if (forward) return a.did < b.did;
	    return a.did > b.did;
	}
};

////////////////////////////////////
// Initialisation and cleaning up //
////////////////////////////////////
MultiMatch::MultiMatch(const OmDatabase &db_, const OmQuery::Internal * query_,
		       const OmRSet & omrset, om_valueno collapse_key_,
		       int percent_cutoff_, om_weight weight_cutoff_,
		       bool sort_forward_, om_valueno sort_key_,
		       int sort_bands_, time_t bias_halflife_,
		       om_weight bias_weight_, OmErrorHandler * errorhandler_,
		       AutoPtr<StatsGatherer> gatherer_,
		       const OmWeight * weight_)
	: gatherer(gatherer_), db(db_), query(query_),
	  collapse_key(collapse_key_), percent_cutoff(percent_cutoff_),
	  weight_cutoff(weight_cutoff_), sort_forward(sort_forward_),
	  sort_key(sort_key_), sort_bands(sort_bands_),
	  bias_halflife(bias_halflife_), bias_weight(bias_weight_),
	  mcmp(sort_forward ? msetcmp_forward : msetcmp_reverse),
	  errorhandler(errorhandler_), weight(weight_)
{
    DEBUGCALL(MATCH, void, "MultiMatch", db_ << ", " << query_ << ", " <<
	      omrset << ", " << collapse_key_ << ", " << percent_cutoff_ <<
	      ", " << weight_cutoff_ << ", " << sort_forward << ", " <<
	      sort_key_ << ", " << sort_bands_ << ", " << bias_halflife_ <<
	      ", " << bias_weight_ << ", " << errorhandler_ <<
	      ", [gatherer_], [weight_]");
    query->validate_query();

    om_doccount number_of_leaves = db.internal->databases.size();
    vector<OmRSet> subrsets(number_of_leaves);

    set<om_docid>::const_iterator reldoc; 
    for (reldoc = omrset.internal->items.begin();
	 reldoc != omrset.internal->items.end(); ++reldoc) {
	om_doccount local_docid = ((*reldoc) - 1) / number_of_leaves + 1;
	om_doccount subdatabase = ((*reldoc) - 1) % number_of_leaves;
	subrsets[subdatabase].add_document(local_docid);
    }
    
    vector<OmRSet>::const_iterator subrset = subrsets.begin();

    vector<RefCntPtr<Database> >::iterator i;
    for (i = db.internal->databases.begin();
	 i != db.internal->databases.end(); ++i) {
	Assert(subrset != subrsets.end());
	Database *db = (*i).get();
	Assert(db);
	RefCntPtr<SubMatch> smatch;
	try {
	    // There is currently only one special case, for network databases.
#ifdef MUS_BUILD_BACKEND_REMOTE
	    const NetworkDatabase *netdb = db->as_networkdatabase();
	    if (netdb) {
		if (sort_key != om_valueno(-1) || sort_bands) {
		    throw OmUnimplementedError("sort_key and sort_bands not supported with remote backend");
		}
		if (bias_halflife) {
		    throw OmUnimplementedError("bias_halflife and bias_weight not supported with remote backend");
		}
		smatch = RefCntPtr<SubMatch>(
			new RemoteSubMatch(netdb, query, *subrset, collapse_key,
			    sort_forward, percent_cutoff, weight_cutoff,
			    gatherer.get(), weight));
	    } else {
#endif /* MUS_BUILD_BACKEND_REMOTE */
		smatch = RefCntPtr<SubMatch>(new LocalSubMatch(db, query, *subrset, gatherer.get(), weight));
#ifdef MUS_BUILD_BACKEND_REMOTE
	    }
#endif /* MUS_BUILD_BACKEND_REMOTE */
	} catch (OmError & e) {
	    if (errorhandler) {
		DEBUGLINE(EXCEPTION, "Calling error handler for creation of a SubMatch from a database and query.");
		(*errorhandler)(e);
		// Continue match without this sub-postlist.
		smatch = RefCntPtr<SubMatch>(new EmptySubMatch());
	    } else {
		throw;
	    }
	}
	leaves.push_back(smatch);
	++subrset;
    }
    Assert(subrset == subrsets.end());

    gatherer->set_global_stats(omrset.size());
    prepare_matchers();
}

MultiMatch::~MultiMatch()
{
    DEBUGCALL(MATCH, void, "~MultiMatch", "");
}

void
MultiMatch::prepare_matchers()
{
    DEBUGCALL(MATCH, void, "MultiMatch::prepare_matchers", "");
    bool prepared;
    bool nowait = true;
    do {
	prepared = true;
	vector<RefCntPtr<SubMatch> >::iterator leaf;
	for (leaf = leaves.begin(); leaf != leaves.end(); ++leaf) {
	    try {
		if (!(*leaf)->prepare_match(nowait)) prepared = false;
	    } catch (OmError & e) {
		if (errorhandler) {
		    DEBUGLINE(EXCEPTION, "Calling error handler for prepare_match() on a SubMatch.");
		    (*errorhandler)(e);
		    // Continue match without this sub-match.
		    *leaf = RefCntPtr<SubMatch>(new EmptySubMatch());
		    prepared = false;
		} else {
		    throw;
		}
	    }
	}
	// Use blocking IO on subsequent passes, so that we don't go into
	// a tight loop.
	nowait = false;
    } while (!prepared);
}

inline string
MultiMatch::get_collapse_key(PostList *pl, const OmDatabase &db, om_docid did,
			     om_valueno keyno, RefCntPtr<Document> &doc)
{		      
    DEBUGCALL(MATCH, string, "MultiMatch::get_collapse_key", pl << ", " << db << ", " << did << ", " << keyno << ", [doc]");
    const string *key = pl->get_collapse_key();
    if (key) RETURN(*key);
    if (doc.get() == 0) {
	RefCntPtr<Document> temp(db.internal->open_document(did));
	doc = temp;
    }
    RETURN(doc->get_value(keyno));
}

om_weight
MultiMatch::getorrecalc_maxweight(PostList *pl)
{
    DEBUGCALL(MATCH, om_weight, "MultiMatch::getorrecalc_maxweight", pl);
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
    RETURN(wt);
}

void
MultiMatch::get_mset(om_doccount first, om_doccount maxitems,
		     OmMSet & mset, const OmMatchDecider *mdecider)
{
    DEBUGCALL(MATCH, void, "MultiMatch::get_mset", first << ", " << maxitems
	      << ", ...");

    map<om_termname, OmMSet::Internal::Data::TermFreqAndWeight> termfreqandwts;

    Assert(!leaves.empty());

    // Start matchers
    {
	vector<RefCntPtr<SubMatch> >::iterator leaf;
	for (leaf = leaves.begin(); leaf != leaves.end(); ++leaf) {
	    try {
		(*leaf)->start_match(first + maxitems);
	    } catch (OmError & e) {
		if (errorhandler) {
		    DEBUGLINE(EXCEPTION, "Calling error handler for "
			      "start_match() on a SubMatch.");
		    (*errorhandler)(e);
		    // Continue match without this sub-match.
		    *leaf = RefCntPtr<SubMatch>(new EmptySubMatch());
		} else {
		    throw;
		}
	    }
	}
    }

    // Get postlists
    vector<PostList *> postlists;
    vector<RefCntPtr<SubMatch> >::iterator i;
    for (i = leaves.begin(); i != leaves.end(); ++i) {
	// FIXME: errorhandler here? (perhaps not needed if this simply makes a pending postlist)
	postlists.push_back((*i)->get_postlist(first + maxitems, this));
    }

    // Get term info
    termfreqandwts.clear();
    {
	vector<RefCntPtr<SubMatch> >::iterator leaf;
	vector<PostList * >::iterator pl_iter;
	Assert(leaves.size() == postlists.size());
	for (leaf = leaves.begin(), pl_iter = postlists.begin();
	     leaf != leaves.end();
	     ++leaf, ++pl_iter) {
	    try {
		termfreqandwts = (*leaf)->get_term_info();
		break;
	    } catch (OmError & e) {
		if (e.get_type() == "OmInternalError" &&
		    e.get_msg().substr(0, 13) == "EmptySubMatch") {
		    DEBUGLINE(MATCH, "leaf is an EmptySubMatch, trying next");
		} else {
		    if (errorhandler) {
			DEBUGLINE(EXCEPTION, "Calling error handler for "
				  "get_term_info() on a SubMatch.");
			(*errorhandler)(e);
			// Continue match without this sub-match.
			*leaf = RefCntPtr<SubMatch>(new EmptySubMatch());

			AutoPtr<LeafPostList> lpl(new EmptyPostList);
			// give it a weighting object
			// FIXME: make it an EmptyWeight instead of BoolWeight
			lpl->set_termweight(new BoolWeight());

			delete *pl_iter;
			*pl_iter = lpl.release();
		    } else {
			throw;
		    }
		}
	    }
	}
    }
    
    // Get a single combined postlist
    PostList *pl;
    Assert(postlists.size() != 0);
    if (postlists.size() == 1) {
	pl = postlists.front();
    } else {
	pl = new MergePostList(postlists, this, errorhandler);
    }

    // FIXME: a temporary bodge to allow this to be used - I'll write
    // a proper API later, promise - Olly
    if (bias_halflife) {
	pl = new BiasPostList(pl, db,
	       	new OmBiasFunctor(db, bias_weight, bias_halflife), this);
    }

    DEBUGLINE(MATCH, "pl = (" << pl->get_description() << ")");

    // Empty result set
    om_doccount docs_matched = 0;
    om_weight greatest_wt = 0;
    vector<OmMSetItem> items;

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
	mset = OmMSet(new OmMSet::Internal(new OmMSet::Internal::Data(
					   first,
					   matches_upper_bound,
					   matches_lower_bound,
					   matches_estimated,
					   max_weight, greatest_wt, items,
					   termfreqandwts,
					   0)));
	return;
    }

    // Set max number of results that we want - this is used to decide
    // when to throw away unwanted items.
    om_doccount max_msize = first + maxitems;
    items.reserve(max_msize + 1);

    // Set the minimum item, used to compare against to see if an item
    // should be considered for the mset.
    OmMSetItem min_item(weight_cutoff, 0);

    om_weight percent_factor = percent_cutoff / 100.0;
 
    // Table of keys which have been seen already, for collapsing.
    map<string, OmMSetItem> collapse_tab;

    // Perform query

    // We form the mset in two stages.  In the first we fill up our working
    // mset.  Adding a new document does not remove another.
    //
    // In the second, we consider documents which rank higher than the current
    // lowest ranking document in the mset.  Each document added expels the
    // current lowest ranking document.
    //
    // If a percentage cutoff is in effect, it can cause the matcher to return
    // from the second stage from the first.

    // Is the mset a valid heap?
    bool is_heap = false; 

    while (true) {
	if (recalculate_w_max) {
	    if (min_item.wt > 0.0 && getorrecalc_maxweight(pl) < min_item.wt) {
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

	om_docid did = pl->get_docid();
	om_weight wt = 0.0;
	if (min_item.wt > 0.0) wt = pl->get_weight();

	DEBUGLINE(MATCH, "Candidate document id " << did << " wt " << wt);
	OmMSetItem new_item(wt, did);

	// test if item has high enough weight to get into proto-mset
	if (min_item.wt > 0.0 && !mcmp(new_item, min_item)) continue;

	RefCntPtr<Document> doc;

	// Use the decision functor if any.
	// FIXME: if results are from MSetPostList then we can omit this step
	if (mdecider != NULL) {
	    if (doc.get() == 0) {
		RefCntPtr<Document> temp(db.internal->open_document(did));
		doc = temp;
	    }
	    OmDocument mydoc(new OmDocument::Internal(doc, db, did));
	    if (!mdecider->operator()(mydoc)) continue;
	}

	if (min_item.wt <= 0.0) {
	    // we didn't calculate the weight above, but now we will need it
	    wt = pl->get_weight();
	    new_item.wt = wt;
	}

	bool pushback = true;

	// Perform collapsing on key if requested.
	if (collapse_key != om_valueno(-1)) {
	    new_item.collapse_key = get_collapse_key(pl, db, did,
						     collapse_key, doc);

	    // Don't collapse on null key
	    if (!new_item.collapse_key.empty()) {
		map<string, OmMSetItem>::iterator oldkey;
		oldkey = collapse_tab.find(new_item.collapse_key);
		if (oldkey == collapse_tab.end()) {
		    DEBUGLINE(MATCH, "collapsem: new key: " <<
			      new_item.collapse_key);
		    // Key not been seen before
		    collapse_tab.insert(make_pair(new_item.collapse_key,
						  new_item));
		} else {
		    const OmMSetItem old_item = oldkey->second;
		    if (mcmp(old_item, new_item)) {
			DEBUGLINE(MATCH, "collapsem: better exists: " <<
				  new_item.collapse_key);
			// There's already a better match with this key
			continue;
		    }
		    // This is best match with this key so far:
		    // remove the old one from the MSet
		    if (min_item.wt <= 0.0 || mcmp(old_item, min_item)) {
			// Old one hasn't fallen out of MSet yet
			// Scan through (unsorted) MSet looking for entry
			// FIXME: more efficient way than just scanning?
			om_docid olddid = old_item.did;
			DEBUGLINE(MATCH, "collapsem: removing " << olddid <<
				  ": " << new_item.collapse_key);
			vector<OmMSetItem>::iterator i;
			for (i = items.begin(); i->did != olddid; ++i) {
			    Assert(i != items.end());
			}
			*i = new_item;
			pushback = false;
			// We can replace an arbitrary element in O(log N)
			// but have to do it by hand (in this case the new
			// elt is bigger, so we just swap down the tree)
			// FIXME: implement this, and clean up is_heap
			// handling
			is_heap = false;
		    }
		    oldkey->second = new_item;
		}
	    }
	}
	    
	// OK, actually add the item to the mset.
	if (pushback) {
	    ++docs_matched;
	    if (items.size() >= max_msize) {
		if (sort_bands) {
		    if (!is_heap) {
			is_heap = true;
			make_heap<vector<OmMSetItem>::iterator,
				  OmMSetCmp>(items.begin(), items.end(), mcmp);
		    }
		    om_weight tmp = min_item.wt;
		    min_item = items.front();
		    min_item.wt = tmp;
		    om_weight cmp = min_item.wt - new_item.wt;
		    if (cmp != 0.0) {
			if (min_item.sort_key.empty()) {
			    OmDocument doc = db.get_document(min_item.did);
			    min_item.sort_key = doc.get_value(sort_key);
			}
			if (new_item.sort_key.empty()) {
			    OmDocument doc = db.get_document(new_item.did);
			    new_item.sort_key = doc.get_value(sort_key);
			}
			if (cmp > 0) {
			    if (min_item.sort_key >= new_item.sort_key)
				pushback = false;
			} else {
			    if (min_item.sort_key <= new_item.sort_key) {
				pop_heap<vector<OmMSetItem>::iterator,
					 OmMSetCmp>(items.begin(), items.end(),
						    mcmp);
				items.pop_back();
			    }
			}
		    }
		    if (pushback) {
			items.push_back(new_item);
			push_heap<vector<OmMSetItem>::iterator,
				  OmMSetCmp>(items.begin(), items.end(), mcmp);
		    }
		} else {
		    items.push_back(new_item);
		    if (!is_heap) {
			is_heap = true;
			make_heap<vector<OmMSetItem>::iterator,
				  OmMSetCmp>(items.begin(), items.end(), mcmp);
		    } else {
			push_heap<vector<OmMSetItem>::iterator,
				  OmMSetCmp>(items.begin(), items.end(), mcmp);
		    }
		    pop_heap<vector<OmMSetItem>::iterator,
			     OmMSetCmp>(items.begin(), items.end(), mcmp);
		    items.pop_back(); 
		    min_item = items.front();
		    if (getorrecalc_maxweight(pl) < min_item.wt) {
			DEBUGLINE(MATCH, "*** TERMINATING EARLY (3)");
			break;
		    }
		}
	    } else {
		items.push_back(new_item);
		is_heap = false;
		if (!sort_bands && items.size() == max_msize - 1) {
		    // We're done if this is a forward boolean match
		    // (bodgetastic, FIXME better if we can)
		    if (max_weight == 0 && sort_forward) break;
		}
	    }
	}

	// Keep a track of the greatest weight we've seen.
	if (wt > greatest_wt) {
	    greatest_wt = wt;
	    if (percent_cutoff) {
	        om_weight w = wt * percent_factor;
	        if (w > min_item.wt) {
	            min_item.wt = w;
	            min_item.did = 0;
		    if (!is_heap) {
			is_heap = true;
			make_heap<vector<OmMSetItem>::iterator,
				  OmMSetCmp>(items.begin(), items.end(),
						  mcmp);
		    }
		    while (!items.empty() && items.front().wt < min_item.wt) {
			pop_heap<vector<OmMSetItem>::iterator,
				 OmMSetCmp>(items.begin(), items.end(), mcmp);
			Assert(items.back().wt < min_item.wt);
			items.pop_back();
		    }
#ifdef MUS_DEBUG_PARANOID
		    vector<OmMSetItem>::const_iterator i;
		    for (i = items.begin(); i != items.end(); ++i) {
			Assert(i->wt >= min_item.wt);
		    }
#endif
	        }
	    }
	    if (sort_bands > 1) {
		if (greatest_wt >= getorrecalc_maxweight(pl)) {
		    if (!is_heap) {
			is_heap = true;
			make_heap<vector<OmMSetItem>::iterator,
				  OmMSetCmp>(items.begin(), items.end(), mcmp);
		    }
		    // greatest_wt cannot now rise any further, so we now know
		    // exactly where the relevance bands are.
		    om_weight w = greatest_wt / sort_bands *
			    floor(items.front().wt * sort_bands / greatest_wt);
		    if (w > min_item.wt) min_item.wt = w;
		}
	    }
	}
    }

    // done with posting list tree
    delete pl;

    double percent_scale = 0;
    if (!items.empty() && greatest_wt > 0) {
	// Find the document with the highest weight, then total up the
	// weights for the terms it contains
	vector<OmMSetItem>::const_iterator best;
	best = min_element(items.begin(), items.end(), mcmp);

	om_termcount matching_terms = 0;
	map<om_termname,
	    OmMSet::Internal::Data::TermFreqAndWeight>::const_iterator i;

	OmTermIterator docterms = db.termlist_begin(best->did);
        OmTermIterator docterms_end = db.termlist_end(best->did);
	while (docterms != docterms_end) {
	    i = termfreqandwts.find(*docterms);
	    if (i != termfreqandwts.end()) {
		percent_scale += i->second.termweight;
		++matching_terms;
		if (matching_terms == termfreqandwts.size()) break;
	    }
	    ++docterms;
	}
	if (matching_terms < termfreqandwts.size()) {
	    // OK, work out weight corresponding to 100%
	    double denom = 0;
	    for (i = termfreqandwts.begin(); i != termfreqandwts.end(); ++i)
		denom += i->second.termweight;
	    denom *= greatest_wt;
	    Assert(denom > 0);
	    Assert(denom <= percent_scale);
	    percent_scale /= denom;
	} else {
	    // If all the terms match, the 2 sums of weights cancel
	    percent_scale = 1.0 / greatest_wt;
	}
	Assert(percent_scale > 0);
	if (percent_cutoff) {
	    // FIXME: better to sort and binary chop maybe?  we
	    // could use the sort above to find "best" too.
	    // Or we could just use a linear scan here instead.

	    // trim the mset to the correct answer...
	    om_weight min_wt = percent_factor / percent_scale;
	    if (!is_heap) {
		is_heap = true;
		make_heap<vector<OmMSetItem>::iterator,
			  OmMSetCmp>(items.begin(), items.end(), mcmp);
	    }
	    while (!items.empty() && items.front().wt < min_wt) {
		pop_heap<vector<OmMSetItem>::iterator,
			 OmMSetCmp>(items.begin(), items.end(), mcmp);
		Assert(items.back().wt < min_wt);
		items.pop_back();
	    }
#ifdef MUS_DEBUG_PARANOID
	    vector<OmMSetItem>::const_iterator i;
	    for (i = items.begin(); i != items.end(); ++i) {
		Assert(i->wt >= min_wt);
	    }
#endif
	}
	percent_scale *= 100.0;
    }
    
    if (sort_bands) {
	sort(items.begin(), items.end(),
	     MSetSortCmp(db, sort_bands, percent_scale,
			 sort_key, sort_forward));
	if (items.size() > max_msize) {
	    items.erase(items.begin() + max_msize, items.end());
	}
    }

    if (items.size() < max_msize) {
	DEBUGLINE(MATCH, "items.size() = " << items.size() <<
		  ", max_msize = " << max_msize << ", setting bounds equal");
	Assert(percent_cutoff || docs_matched == items.size());
	matches_lower_bound = matches_upper_bound = matches_estimated
	    = items.size();
    } else if (percent_cutoff) {
	// another approach: om_doccount new_est = items.size() * (1 - percent_factor) / (1 - min_item.wt / greatest_wt);
	om_doccount new_est = (om_doccount)((1 - percent_factor) *
					    matches_estimated);
	matches_estimated = max((size_t)new_est, items.size());
	// and another: items.size() + (1 - greatest_wt * percent_factor / min_item.wt) * (matches_estimated - items.size());
	
	// Very likely an underestimate, but we can't really do better without
	// checking further matches...  Only possibility would be to track how
	// many docs made the min weight test but didn't make the candidate set
	// since the last greatest_wt change, which we could use if the top
	// documents matched all the prob terms.
	matches_lower_bound = items.size();
	// matches_upper_bound could be reduced by the number of documents
	// which fail the min weight test
    }

    DEBUGLINE(MATCH, items.size() << " items in potential mset");

    if (first > 0) {
	// Remove unwanted leading entries
	if (items.size() <= first) {
	    items.clear();
	} else {
	    DEBUGLINE(MATCH, "finding " << first << "th");
	    if (!sort_bands)
		nth_element(items.begin(), items.begin() + first, items.end(),
			    mcmp);
	    // erase the leading ``first'' elements
	    items.erase(items.begin(), items.begin() + first);
	}
    }

    DEBUGLINE(MATCH,
	      "msize = " << items.size() << ", " <<
	      "docs_matched = " << docs_matched << ", " <<
	      "matches_lower_bound = " << matches_lower_bound << ", " <<
	      "matches_estimated = " << matches_estimated << ", " <<
	      "matches_upper_bound = " << matches_upper_bound);

    if (!sort_bands && !items.empty()) {
	DEBUGLINE(MATCH, "sorting");

	// Need a stable sort, but this is provided by comparison operator
	sort(items.begin(), items.end(), mcmp);

	DEBUGLINE(MATCH, "max weight in mset = " << items[0].wt <<
		  ", min weight in mset = " << items.back().wt);
    }

    Assert(matches_estimated >= matches_lower_bound);
    Assert(matches_estimated <= matches_upper_bound);

    if (!percent_cutoff) {
	Assert(docs_matched <= matches_upper_bound);
	if (docs_matched > matches_lower_bound)
	    matches_lower_bound = docs_matched;
	if (docs_matched > matches_estimated)
	    matches_estimated = docs_matched;
    }

    mset = OmMSet(new OmMSet::Internal(new OmMSet::Internal::Data(
				       first,
				       matches_upper_bound,
				       matches_lower_bound,
				       matches_estimated,
				       max_weight, greatest_wt, items,
				       termfreqandwts,
				       percent_scale)));
}

// This method is called by branch postlists when they rebalance
// in order to recalculate the weights in the tree
void
MultiMatch::recalc_maxweight()
{
    DEBUGCALL(MATCH, void, "MultiMatch::recalc_maxweight", "");
    recalculate_w_max = true;
}
