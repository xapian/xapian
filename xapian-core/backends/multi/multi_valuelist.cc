/** @file
 * @brief Class for merging ValueList objects from subdatabases.
 */
/* Copyright (C) 2007,2008,2009,2011 Olly Betts
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

#include "backends/multivaluelist.h"

#include <xapian/error.h>

#include "omassert.h"

#include <algorithm>

using namespace std;
using Xapian::Internal::intrusive_ptr;

struct SubValueList {
    ValueList * valuelist;
    unsigned db_idx;

    SubValueList(ValueList * vl, unsigned db_idx_)
	: valuelist(vl), db_idx(db_idx_) { }

    ~SubValueList() {
	delete valuelist;
    }

    void skip_to(Xapian::docid did, size_t multiplier) {
	// Translate did from merged docid.
	did = (did - 1) / multiplier + 1 + ((did - 1) % multiplier > db_idx);
	valuelist->skip_to(did);
    }

    Xapian::docid get_docid() const {
	return valuelist->get_docid();
    }

    Xapian::docid get_merged_docid(unsigned multiplier) const {
	return (valuelist->get_docid() - 1) * multiplier + db_idx + 1;
    }

    std::string get_value() const { return valuelist->get_value(); }

    void next() {
	valuelist->next();
    }

    bool at_end() const { return valuelist->at_end(); }
};

/// Comparison functor which orders SubValueList* by ascending docid.
struct CompareSubValueListsByDocId {
    /// Order by ascending docid.
    bool operator()(const SubValueList *a, const SubValueList *b) const {
	Xapian::docid did_a = a->get_docid();
	Xapian::docid did_b = b->get_docid();
	if (did_a > did_b) return true;
	if (did_a < did_b) return false;
	return a->db_idx > b->db_idx;
    }
};

template<class CLASS> struct delete_ptr {
    void operator()(CLASS *p) const { delete p; }
};

MultiValueList::MultiValueList(const vector<intrusive_ptr<Xapian::Database::Internal> > & dbs,
			       Xapian::valueno slot_)
    : current_docid(0), slot(slot_), multiplier(dbs.size())
{
    // The 0 and 1 cases should be handled by our caller.
    AssertRel(multiplier, >=, 2);
    valuelists.reserve(multiplier);
    try {
	unsigned db_idx = 0;
	vector<intrusive_ptr<Xapian::Database::Internal> >::const_iterator i;
	for (i = dbs.begin(); i != dbs.end(); ++i) {
	    ValueList * vl = (*i)->open_value_list(slot);
	    valuelists.push_back(new SubValueList(vl, db_idx));
	    ++db_idx;
	}
    } catch (...) {
	for_each(valuelists.begin(), valuelists.end(), delete_ptr<SubValueList>());
	throw;
    }
}

MultiValueList::~MultiValueList()
{
    for_each(valuelists.begin(), valuelists.end(), delete_ptr<SubValueList>());
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
    return valuelists.front()->get_value();
}

Xapian::valueno
MultiValueList::get_valueno() const
{
    return slot;
}

bool
MultiValueList::at_end() const
{
    return valuelists.empty();
}

void
MultiValueList::next()
{
    if (current_docid == 0) {
	// Make valuelists into a heap so that the one with the earliest
	// sorting docid is at the top of the heap.
	vector<SubValueList *>::iterator i = valuelists.begin();
	while (i != valuelists.end()) {
	    (*i)->next();
	    if ((*i)->at_end()) {
		SubValueList * vl = NULL;
		swap(vl, *i);
		i = valuelists.erase(i);
		delete vl;
	    } else {
		++i;
	    }
	}
	if (rare(valuelists.empty()))
	    return;
	make_heap(valuelists.begin(), valuelists.end(),
		  CompareSubValueListsByDocId());
    } else {
	// Advance to the next docid.
	pop_heap(valuelists.begin(), valuelists.end(),
		 CompareSubValueListsByDocId());
	SubValueList * vl = valuelists.back();
	vl->next();
	if (vl->at_end()) {
	    delete vl;
	    valuelists.pop_back();
	    if (valuelists.empty()) return;
	} else {
	    push_heap(valuelists.begin(), valuelists.end(),
		      CompareSubValueListsByDocId());
	}
    }

    current_docid = valuelists.front()->get_merged_docid(multiplier);
}

void
MultiValueList::skip_to(Xapian::docid did)
{
    // Assume the skip is likely to be a long distance, and rebuild the heap
    // from scratch.  FIXME: It would be useful to profile this against an
    // approach more like that next() uses if this ever gets heavy use.
    vector<SubValueList*>::iterator i = valuelists.begin();
    while (i != valuelists.end()) {
	(*i)->skip_to(did, multiplier);
	if ((*i)->at_end()) {
	    SubValueList * vl = NULL;
	    swap(vl, *i);
	    i = valuelists.erase(i);
	    delete vl;
	} else {
	    ++i;
	}
    }

    if (valuelists.empty()) return;

    make_heap(valuelists.begin(), valuelists.end(), CompareSubValueListsByDocId());

    current_docid = valuelists.front()->get_merged_docid(multiplier);
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
