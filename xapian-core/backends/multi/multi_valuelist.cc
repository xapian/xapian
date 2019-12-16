/** @file multi_valuelist.cc
 * @brief Class for merging ValueList objects from subdatabases.
 */
/* Copyright (C) 2007,2008,2009,2011,2017,2018 Olly Betts
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

#include "multi_valuelist.h"

#include <xapian/error.h>

#include "heap.h"
#include "omassert.h"

#include <algorithm>

using namespace std;
using Xapian::Internal::intrusive_ptr;

/// Comparison functor which orders SubValueList* by ascending docid.
struct CompareSubValueListsByDocId {
    /// Order by ascending docid.
    bool operator()(const SubValueList *a, const SubValueList *b) const {
	Xapian::docid did_a = a->get_docid();
	Xapian::docid did_b = b->get_docid();
	if (did_a > did_b) return true;
	if (did_a < did_b) return false;
	return a->shard > b->shard;
    }
};

MultiValueList::~MultiValueList()
{
    while (count)
	delete valuelists[--count];
    delete [] valuelists;
}

Xapian::docid
MultiValueList::get_docid() const
{
    Assert(!at_end());
    return current_docid;
}

std::string
MultiValueList::get_value() const
{
    Assert(!at_end());
    return valuelists[0]->get_value();
}

Xapian::valueno
MultiValueList::get_valueno() const
{
    return slot;
}

bool
MultiValueList::at_end() const
{
    return count == 0;
}

void
MultiValueList::next()
{
    if (current_docid == 0) {
	// Make valuelists into a heap so that the one with the earliest
	// sorting docid is at the top of the heap.
	Xapian::doccount j = 0;
	for (Xapian::doccount i = 0; i != count; ++i) {
	    valuelists[i]->next();
	    if (valuelists[i]->at_end()) {
		delete valuelists[i];
		valuelists[i] = 0;
	    } else {
		if (i != j)
		    swap(valuelists[i], valuelists[j]);
		++j;
	    }
	}
	count = j;
	if (rare(count == 0))
	    return;

	Heap::make(valuelists, valuelists + count,
		   CompareSubValueListsByDocId());
    } else {
	// Advance to the next docid.
	SubValueList * vl = valuelists[0];
	vl->next();
	if (vl->at_end()) {
	    Heap::pop(valuelists, valuelists + count,
		      CompareSubValueListsByDocId());
	    delete vl;
	    if (--count == 0)
		return;
	} else {
	    Heap::replace(valuelists, valuelists + count,
			  CompareSubValueListsByDocId());
	}
    }

    current_docid = valuelists[0]->get_merged_docid(n_shards);
}

void
MultiValueList::skip_to(Xapian::docid did)
{
    // Assume the skip is likely to be a long distance, and rebuild the heap
    // from scratch.  FIXME: It would be useful to profile this against an
    // approach more like that next() uses if this ever gets heavy use.
    Xapian::doccount j = 0;
    for (Xapian::doccount i = 0; i != count; ++i) {
	valuelists[i]->skip_to(did, n_shards);
	if (valuelists[i]->at_end()) {
	    delete valuelists[i];
	    valuelists[i] = 0;
	} else {
	    if (i != j)
		swap(valuelists[i], valuelists[j]);
	    ++j;
	}
    }
    count = j;
    if (rare(count == 0))
	return;

    Heap::make(valuelists, valuelists + count, CompareSubValueListsByDocId());

    current_docid = valuelists[0]->get_merged_docid(n_shards);
}

bool
MultiValueList::check(Xapian::docid did)
{
    // FIXME: just run check on the subvaluelist which would hold that docid.
    skip_to(did);
    return true;
}

string
MultiValueList::get_description() const
{
    return "MultiValueList()"; // FIXME: improve description...
}
