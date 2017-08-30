/** @file multi_alltermslist.cc
 * @brief Class for merging AllTermsList objects from subdatabases.
 */
/* Copyright (C) 2007,2008,2009,2011,2017 Olly Betts
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

#include "multi_alltermslist.h"

#include <xapian/database.h>

#include "backends/database.h"
#include "omassert.h"

#include <algorithm>

using namespace std;

/// Comparison functor which orders TermList* by ascending term name.
struct CompareTermListsByTerm {
    /// Order by ascending term name.
    bool operator()(const TermList *a, const TermList *b) const {
	return a->get_termname() > b->get_termname();
    }
};

MultiAllTermsList::MultiAllTermsList(const Xapian::Database& db,
				     const string& prefix)
    : count(0), termlists(new TermList*[db.internal.size()])
{
    // The 0 and 1 cases should be handled by our caller.
    AssertRel(db.internal.size(), >=, 2);
    try {
	for (auto&& sub_db : db.internal) {
	    termlists[count] = sub_db->open_allterms(prefix);
	    ++count;
	}
    } catch (...) {
	while (count)
	    delete termlists[--count];
	delete [] termlists;
	throw;
    }
}

MultiAllTermsList::~MultiAllTermsList()
{
    while (count)
	delete termlists[--count];
    delete [] termlists;
}

string
MultiAllTermsList::get_termname() const
{
    return current_term;
}

Xapian::doccount
MultiAllTermsList::get_termfreq() const
{
    if (current_termfreq == 0 && count != 0) {
	while (true) {
	    TermList * tl = termlists[0];
	    if (tl->get_termname() != current_term)
		break;
	    current_termfreq += tl->get_termfreq();
	    pop_heap(termlists, termlists + count,
		     CompareTermListsByTerm());
	    tl->next();
	    if (tl->at_end()) {
		delete tl;
		if (--count == 0)
		    break;
	    } else {
		push_heap(termlists, termlists + count,
			  CompareTermListsByTerm());
	    }
	}
    }
    return current_termfreq;
}

TermList *
MultiAllTermsList::next()
{
    if (current_term.empty()) {
	// Make termlists into a heap so that the one (or one of the ones) with
	// earliest sorting term is at the top of the heap.
	size_t j = 0;
	for (size_t i = 0; i != count; ++i) {
	    TermList* tl = termlists[i];
	    tl->next();
	    if (!tl->at_end()) {
		if (i != j)
		    swap(termlists[i], termlists[j]);
		++j;
	    }
	}
	while (count > j)
	    delete termlists[--count];
	make_heap(termlists, termlists + count,
		  CompareTermListsByTerm());
    } else {
	// Skip over current_term if we haven't already.
	if (current_termfreq == 0)
	    (void)get_termfreq();
    }

    current_termfreq = 0;

    if (count <= 1) {
	if (count == 0) {
	    current_term = std::string();
	    return NULL;
	}
	count = 0;
	return termlists[0];
    }

    current_term = termlists[0]->get_termname();
    return NULL;
}

TermList *
MultiAllTermsList::skip_to(const std::string &term)
{
    // Assume the skip is likely to be a long distance, and rebuild the heap
    // from scratch.  FIXME: It would be useful to profile this against an
    // approach more like that next() uses if this ever gets heavy use.
    size_t j = 0;
    for (size_t i = 0; i != count; ++i) {
	TermList* tl = termlists[i];
	tl->skip_to(term);
	if (!tl->at_end()) {
	    if (i != j)
		swap(termlists[i], termlists[j]);
	    ++j;
	}
    }
    while (count > j)
	delete termlists[--count];

    current_termfreq = 0;

    if (count <= 1) {
	if (count == 0) {
	    current_term = std::string();
	    return NULL;
	}
	count = 0;
	return termlists[0];
    }

    make_heap(termlists, termlists + count, CompareTermListsByTerm());

    current_term = termlists[0]->get_termname();
    return NULL;
}

bool
MultiAllTermsList::at_end() const
{
    return count == 0 && current_term.empty();
}
