/** @file
 * @brief Build tree to merge TermList objects
 */
/* Copyright (C) 2008,2010,2011,2013,2016,2017,2018 Olly Betts
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

#ifndef XAPIAN_INCLUDED_TERMLISTMERGER_H
#define XAPIAN_INCLUDED_TERMLISTMERGER_H

#include "api/termlist.h"
#include "heap.h"
#include "omassert.h"
#include "ortermlist.h"

struct CompareTermListSizeAscending {
    bool operator()(const TermList* a, const TermList* b) const {
	return a->get_approx_size() > b->get_approx_size();
    }
};

template<class ORTERMLIST = OrTermList>
inline TermList*
make_termlist_merger(std::vector<TermList*>& termlists)
{
    if (termlists.size() <= 1) {
	return termlists.size() == 1 ? termlists[0] : NULL;
    }

    // Make termlists into a heap so that the longest termlist is at the
    // top of the heap.
    Heap::make(termlists.begin(), termlists.end(),
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
	TermList* r = termlists.front();
	Heap::pop(termlists.begin(), termlists.end(),
		  CompareTermListSizeAscending());
	termlists.pop_back();
	TermList* l = termlists.front();

	TermList* tl = new ORTERMLIST(l, r);

	if (termlists.size() == 1)
	    return tl;

	termlists.front() = tl;
	Heap::replace(termlists.begin(), termlists.end(),
		      CompareTermListSizeAscending());
    }
}

#endif // XAPIAN_INCLUDED_TERMLISTMERGER_H
