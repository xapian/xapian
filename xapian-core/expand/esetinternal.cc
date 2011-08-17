/** @file esetinternal.cc
 * @brief Xapian::ESet::Internal class
 */
/* Copyright (C) 2008,2010 Olly Betts
 * Copyright (C) 2011 Action Without Borders
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "esetinternal.h"

#include "xapian/enquire.h"
#include "xapian/expanddecider.h"
#include "database.h"
#include "debuglog.h"
#include "omenquireinternal.h"
#include "expandweight.h"
#include "omassert.h"
#include "ortermlist.h"
#include "str.h"
#include "termlist.h"

#include "autoptr.h"
#include <set>
#include <string>
#include <vector>

using namespace std;

namespace Xapian {

string
Internal::ExpandTerm::get_description() const
{
    string desc("ExpandTerm(");
    desc += str(wt);
    desc += ", ";
    desc += term;
    desc += ')';
    return desc;
}

template<class CLASS> struct delete_ptr {
    void operator()(CLASS *p) { delete p; }
};

struct CompareTermListSizeAscending {
    bool operator()(const TermList *a, const TermList *b) {
	return a->get_approx_size() > b->get_approx_size();
    }
};

/** Build a tree of binary TermList objects like QueryOptimiser does for
 *  OrPostList objects.
 */
static TermList *
build_termlist_tree(const Xapian::Database &db, const RSet & rset)
{
    Assert(!rset.empty());

    const set<Xapian::docid> & docids = rset.internal->get_items();

    vector<TermList*> termlists;
    termlists.reserve(docids.size());

    try {
	const size_t multiplier = db.internal.size();
	set<Xapian::docid>::const_iterator i;
	for (i = docids.begin(); i != docids.end(); ++i) {
	    Xapian::docid realdid = (*i - 1) / multiplier + 1;
	    Xapian::doccount dbnumber = (*i - 1) % multiplier;

	    // Push NULL first to avoid leaking the new TermList if push_back()
	    // throws.
	    termlists.push_back(0);
	    termlists.back() = db.internal[dbnumber]->open_term_list(realdid);
	}

	Assert(!termlists.empty());
	if (termlists.size() == 1) return termlists[0];

	// Make termlists into a heap so that the longest termlist is at the
	// top of the heap.
	make_heap(termlists.begin(), termlists.end(),
		  CompareTermListSizeAscending());

	// Now build a tree of binary TermList objects.  The algorithm used to
	// build the tree is like that used to build an optimal Huffman coding
	// tree.  If we called next() repeatedly, this arrangement would
	// minimise the number of method calls.  Generally we don't actually do
	// that, but this arrangement is still likely to be a good one, and it
	// does minimise the work in the worst case.
	while (true) {
	    AssertRel(termlists.size(), >=, 2);
	    // We build the tree such that at each branch:
	    //
	    //   l.get_approx_size() >= r.get_approx_size()
	    //
	    // We do this so that the OrTermList class can be optimised
	    // assuming that this is the case.
	    TermList * r = termlists.front();
	    pop_heap(termlists.begin(), termlists.end(),
		     CompareTermListSizeAscending());
	    termlists.pop_back();
	    TermList * l = termlists.front();

	    TermList * pl = new OrTermList(l, r);

	    if (termlists.size() == 1) return pl;

	    pop_heap(termlists.begin(), termlists.end(),
		     CompareTermListSizeAscending());
	    termlists.back() = pl;
	    push_heap(termlists.begin(), termlists.end(),
		      CompareTermListSizeAscending());
	}
    } catch (...) {
	for_each(termlists.begin(), termlists.end(), delete_ptr<TermList>());
	throw;
    }
}

void
ESet::Internal::expand(Xapian::termcount max_esize,
		       const Xapian::Database & db,
		       const RSet & rset,
		       const Xapian::ExpandDecider * edecider,
		       const Xapian::Internal::ExpandWeight & eweight,
		       Xapian::weight min_wt)
{
    LOGCALL_VOID(EXPAND, "ESet::Internal::expand", max_esize | db | rset | edecider | eweight);
    // These two cases are handled by our caller.
    Assert(max_esize);
    Assert(!rset.empty());
    // This method should only be called once for a given ESet::Internal, so
    // check we're empty.
    Assert(ebound == 0);
    Assert(items.empty());

    AutoPtr<TermList> tree(build_termlist_tree(db, rset));
    Assert(tree.get());

    bool is_heap = false;
    while (true) {
	// See if the root needs replacing.
	TermList * new_root = tree->next();
	if (new_root) {
	    LOGLINE(EXPAND, "Replacing the root of the termlist tree");
	    tree.reset(new_root);
	}

	if (tree->at_end()) break;

	string term = tree->get_termname();

	// If there's an ExpandDecider, see if it accepts the term.
	if (edecider && !(*edecider)(term)) continue;

	++ebound;

	Xapian::weight wt = eweight.get_weight(tree.get(), term);
	// If the weights are equal, we prefer the lexically smaller term and
	// so we use "<=" not "<" here.
	if (wt <= min_wt) continue;

	items.push_back(Xapian::Internal::ExpandTerm(wt, term));

	// The candidate ESet is overflowing, so remove the worst element in it
	// using a min-heap.
	if (items.size() > max_esize) {
	    if (rare(!is_heap)) {
		is_heap = true;
		make_heap(items.begin(), items.end());
	    } else {
		push_heap(items.begin(), items.end());
	    }
	    pop_heap(items.begin(), items.end());
	    items.pop_back();
	    min_wt = items.front().wt;
	}
    }

    // Now sort the contents of the new ESet.
    if (is_heap) {
	sort_heap(items.begin(), items.end());
    } else {
	sort(items.begin(), items.end());
    }
}

string
ESet::Internal::get_description() const
{
    string desc("ESet::Internal(ebound=");
    desc += str(ebound);

    vector<Xapian::Internal::ExpandTerm>::const_iterator i;
    for (i = items.begin(); i != items.end(); ++i) {
	desc += ", ";
	desc += i->get_description();
    }
    desc += ')';

    return desc;
}

}
