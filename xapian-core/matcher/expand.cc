/* expand.cc
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

#include "expand.h"
#include "rset.h"
#include "database.h"
#include "ortermlist.h"
#include "omdebug.h"

#include <algorithm>

class OmESetCmp {
    public:
        bool operator()(const OmESetItem &a, const OmESetItem &b) {
	    if(a.wt > b.wt) return true;
	    if(a.wt != b.wt) return false;
	    return a.tname > b.tname;
        }
};

class TLPCmpGt {
    public:
	bool operator()(const TermList *a, const TermList *b) {
	    return a->get_approx_size() > b->get_approx_size();
	}
};

TermList *
OmExpand::build_tree(const RSet *rset, const OmExpandWeight *ewt)
{
    // Put items in priority queue, such that items with greatest size
    // are returned first.
    // This is the same idea as for a set of postlists ORed together in
    // the matcher.
    //
    // FIXME: try using a heap instead (C++ sect 18.8)?
    priority_queue<TermList*, vector<TermList*>, TLPCmpGt> pq;
    vector<RSetItem>::const_iterator i;
    for (i = rset->documents.begin();
	 i != rset->documents.end();
	 i++) {
	LeafTermList *tl = database->open_term_list((*i).did);
	tl->set_weighting(ewt);
	pq.push(tl);
    }

    if (pq.empty()) {
	return NULL;
    }

    // Build a tree balanced by the term frequencies
    // (similar to building a huffman encoding tree).
    //
    // This scheme reduces the number of objects terms from large docs
    // get "pulled" through, reducing the amount of work done which
    // speeds things up.
    while (true) {
	TermList *p = pq.top();
	DEBUGMSG(EXPAND, "OmExpand: adding termlist " << p << " to tree" << endl);
	pq.pop();
	if (pq.empty()) {
	    return p;
	}
	// NB right is always <= left - we can use this to optimise
	p = new OrTermList(pq.top(), p);
	pq.pop();
	pq.push(p);
    }
}

void
OmExpand::expand(om_termcount max_esize,
		 OmESet & eset,
		 const RSet * rset,
		 const OmExpandDecider * decider)
{
    eset.items.clear();
    eset.ebound = 0;

    DEBUGMSG(EXPAND, "OmExpand::expand()" << endl);
    if (rset->get_rsize() == 0) return; // No query
    DEBUGMSG(EXPAND, "OmExpand::expand() 2" << endl);

    om_weight w_min = 0;

    // Start weighting scheme
    OmExpandWeight ewt(database, rset->get_rsize());

    TermList *merger = build_tree(rset, &ewt);
    if(merger == NULL) return;

    DEBUGMSG(EXPAND, "ewt.get_maxweight() = " << ewt.get_maxweight() << endl);
    while (1) {
	TermList *ret = merger->next();
        if (ret) {
	    DEBUGMSG(EXPAND, "*** REPLACING ROOT" << endl);
	    delete merger;
	    merger = ret;
	}

	if (merger->at_end()) break;

	om_termname tname = merger->get_termname();
	if((*decider)(tname)) {
	    eset.ebound++;

	    OmExpandBits ebits = merger->get_weighting();
	    om_weight wt = ewt.get_weight(ebits, tname);

	    if (wt > w_min) {
		eset.items.push_back(OmESetItem(wt, tname));

		// FIXME: find balance between larger size for more efficient
		// nth_element and smaller size for better w_min optimisations
		if (eset.items.size() == max_esize * 2) {
		    // find last element we care about
		    DEBUGMSG(EXPAND, "finding nth" << endl);
		    nth_element(eset.items.begin(),
				eset.items.begin() + max_esize,
				eset.items.end(),
				OmESetCmp());
		    // erase elements which don't make the grade
		    eset.items.erase(eset.items.begin() + max_esize,
				     eset.items.end());
		    w_min = eset.items.back().wt;
		    DEBUGMSG(EXPAND, "eset size = " << eset.items.size() << endl);
		}
	    }
	}
    }

    if (eset.items.size() > max_esize) {
	// find last element we care about
	DEBUGMSG(EXPAND, "finding nth" << endl);
	nth_element(eset.items.begin(),
		    eset.items.begin() + max_esize,
		    eset.items.end(), OmESetCmp());
	// erase elements which don't make the grade
	eset.items.erase(eset.items.begin() + max_esize, eset.items.end());
    }
    DEBUGMSG(EXPAND, "sorting" << endl);

    // Need a stable sort, but this is provided by comparison operator
    sort(eset.items.begin(), eset.items.end(), OmESetCmp());

    DEBUGMSG(EXPAND, "esize = " << eset.items.size() << ", ebound = " << eset.ebound << endl);
    if (eset.items.size()) {
	DEBUGMSG(EXPAND, "max weight in eset = " << eset.items.front().wt
		 << ", min weight in eset = " << eset.items.back().wt << endl);
    }
    delete merger;
}
