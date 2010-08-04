/** @file multi_alltermslist.cc
 * @brief Class for merging AllTermsList objects from subdatabases.
 */
/* Copyright (C) 2007,2008,2009 Olly Betts
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

#include "multialltermslist.h"

#include <xapian/error.h>

#include "omassert.h"

#include <algorithm>

using namespace std;

/// Comparison functor which orders TermList* by ascending term name.
struct CompareTermListsByTerm {
    /// Order by ascending term name.
    bool operator()(const TermList *a, const TermList *b) {
	return a->get_termname() > b->get_termname();
    }
};

template<class CLASS> struct delete_ptr {
    void operator()(CLASS *p) { delete p; }
};

MultiAllTermsList::MultiAllTermsList(const vector<Xapian::Internal::RefCntPtr<Xapian::Database::Internal> > & dbs,
				     const string & prefix)
{
    // The 0 and 1 cases should be handled by our caller.
    AssertRel(dbs.size(), >=, 2);
    termlists.reserve(dbs.size());
    try {
	vector<Xapian::Internal::RefCntPtr<Xapian::Database::Internal> >::const_iterator i;
	for (i = dbs.begin(); i != dbs.end(); ++i) {
	    termlists.push_back((*i)->open_allterms(prefix));
	}
    } catch (...) {
	for_each(termlists.begin(), termlists.end(), delete_ptr<TermList>());
	throw;
    }
}

MultiAllTermsList::~MultiAllTermsList()
{
    for_each(termlists.begin(), termlists.end(), delete_ptr<TermList>());
}

string
MultiAllTermsList::get_termname() const
{
    return current_term;
}

Xapian::doccount
MultiAllTermsList::get_termfreq() const
{
    if (termlists.empty()) return 0;
    vector<TermList *>::const_iterator i = termlists.begin();
    Xapian::doccount total_tf = (*i)->get_termfreq();
    while (++i != termlists.end()) {
	if ((*i)->get_termname() == current_term)
	    total_tf += (*i)->get_termfreq();
    }
    return total_tf;
}

Xapian::termcount
MultiAllTermsList::get_collection_freq() const
{
    if (termlists.empty()) return 0;
    vector<TermList *>::const_iterator i = termlists.begin();
    Xapian::termcount total_cf = (*i)->get_collection_freq();
    while (++i != termlists.end()) {
	if ((*i)->get_termname() == current_term)
	    total_cf += (*i)->get_collection_freq();
    }
    return total_cf;
}

TermList *
MultiAllTermsList::next()
{
    if (current_term.empty()) {
	// Make termlists into a heap so that the one (or one of the ones) with
	// earliest sorting term is at the top of the heap.
	vector<TermList*>::iterator i = termlists.begin();
	while (i != termlists.end()) {
	    (*i)->next();
	    if ((*i)->at_end()) {
		delete *i;
		i = termlists.erase(i);
	    } else {
		++i;
	    }
	}
	make_heap(termlists.begin(), termlists.end(),
		  CompareTermListsByTerm());
    } else {
	// Advance to the next termname.
	do {
	    TermList * tl = termlists.front();
	    pop_heap(termlists.begin(), termlists.end(),
		     CompareTermListsByTerm());
	    tl->next();
	    if (tl->at_end()) {
		delete tl;
		termlists.pop_back();
	    } else {
		termlists.back() = tl;
		push_heap(termlists.begin(), termlists.end(),
			  CompareTermListsByTerm());
	    }
	} while (!termlists.empty() &&
		 termlists.front()->get_termname() == current_term);
    }

    if (termlists.size() <= 1) {
	if (termlists.empty()) return NULL;
	TermList * tl = termlists[0];
	termlists.clear();
	return tl;
    }

    current_term = termlists.front()->get_termname();
    return NULL;
}

TermList *
MultiAllTermsList::skip_to(const std::string &term)
{
    // Assume the skip is likely to be a long distance, and rebuild the heap
    // from scratch.  FIXME: It would be useful to profile this against an
    // approach more like that next() uses if this ever gets heavy use.
    vector<TermList*>::iterator i = termlists.begin();
    while (i != termlists.end()) {
	(*i)->skip_to(term);
	if ((*i)->at_end()) {
	    delete *i;
	    i = termlists.erase(i);
	} else {
	    ++i;
	}
    }

    if (termlists.size() <= 1) {
	if (termlists.empty()) return NULL;
	TermList * tl = termlists[0];
	termlists.clear();
	return tl;
    }

    make_heap(termlists.begin(), termlists.end(), CompareTermListsByTerm());
    
    current_term = termlists.front()->get_termname();
    return NULL;
}

bool
MultiAllTermsList::at_end() const
{
    return termlists.empty();
}
