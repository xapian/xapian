/* maptermlist.h
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2010 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef OM_HGUARD_MAPTERMLIST_H
#define OM_HGUARD_MAPTERMLIST_H

#include "termlist.h"

#include "inmemory_positionlist.h"
#include "document.h"

#include "omassert.h"

using namespace std;

class MapTermList : public TermList {
    private:
	Xapian::Document::Internal::document_terms::const_iterator it;
	Xapian::Document::Internal::document_terms::const_iterator it_end;
	bool started;

    public:
	MapTermList(const Xapian::Document::Internal::document_terms::const_iterator &it_,
		    const Xapian::Document::Internal::document_terms::const_iterator &it_end_)
		: it(it_), it_end(it_end_), started(false)
	{ }

	// Gets size of termlist
	Xapian::termcount get_approx_size() const {
	    // This method shouldn't get called on a MapTermList.
	    Assert(false);
	    return 0;
	}

	// Gets current termname
	string get_termname() const {
	    Assert(started);
	    Assert(!at_end());
	    return it->first;
	}

	// Get wdf of current term
	Xapian::termcount get_wdf() const {
	    Assert(started);
	    Assert(!at_end());
	    return it->second.wdf;
	}

	// Get num of docs indexed by term
	Xapian::doccount get_termfreq() const {
	    throw Xapian::InvalidOperationError("Can't get term frequency from a document termlist which is not associated with a database.");
	}

	Xapian::PositionIterator positionlist_begin() const {
	    return Xapian::PositionIterator(new InMemoryPositionList(it->second.positions));
	}

	Xapian::termcount positionlist_count() const {
	    return it->second.positions.size();
	}

	TermList * next() {
	    if (!started) {
		started = true;
	    } else {
		Assert(!at_end());
		++it;
	    }
	    return NULL;
	}

	TermList * skip_to(const std::string & term) {
	    while (it != it_end && it->first < term) {
		++it;
	    }
	    started = true;
	    return NULL;
	}

	// True if we're off the end of the list
	bool at_end() const {
	    Assert(started);
	    return it == it_end;
	}
};

#endif /* OM_HGUARD_MAPTERMLIST_H */
