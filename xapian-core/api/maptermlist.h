/* maptermlist.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2005 Olly Betts
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

#ifndef OM_HGUARD_MAPTERMLIST_H
#define OM_HGUARD_MAPTERMLIST_H

#include "termlist.h"

#include "inmemory_positionlist.h"
#include "document.h"

using namespace std;

class MapTermList : public TermList {
    private:
	Xapian::Document::Internal::document_terms::const_iterator it;
	Xapian::Document::Internal::document_terms::const_iterator it_end;
	Xapian::termcount size;
	bool started;

    public:
	MapTermList(const Xapian::Document::Internal::document_terms::const_iterator &it_,
		    const Xapian::Document::Internal::document_terms::const_iterator &it_end_,
		    Xapian::termcount size_)
		: it(it_), it_end(it_end_), size(size_), started(false)
	{ }

	// Gets size of termlist
	Xapian::termcount get_approx_size() const {
	    return size;
	}

	// Gets weighting info for current term
	OmExpandBits get_weighting() const {
	    Assert(false); // should never get called
	    abort();
#if defined __SUNPRO_CC || defined __sgi
	    // For compilers which worry abort() might return...
	    return OmExpandBits(0, 0, 0);
#endif
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
	    Assert(false); // should never get called
	    return 0;
	}

	Xapian::PositionIterator positionlist_begin() const {
	    return Xapian::PositionIterator(new InMemoryPositionList(it->second.positions));
	}

	TermList * next() {
	    if (!started) {
		started = true;
	    } else {
		Assert(!at_end());
		it++;
	    }
	    return NULL;
	}

	// True if we're off the end of the list
	bool at_end() const {
	    Assert(started);
	    return it == it_end;
	}
};

#endif /* OM_HGUARD_MAPTERMLIST_H */
