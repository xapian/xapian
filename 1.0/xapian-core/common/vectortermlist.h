/* vectortermlist.h
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2005,2006,2007 Olly Betts
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

#ifndef OM_HGUARD_VECTORTERMLIST_H
#define OM_HGUARD_VECTORTERMLIST_H

#include <xapian/error.h>
#include "omassert.h"
#include "termlist.h"

#include <list>
#include <vector>

using namespace std;

class VectorTermList : public TermList {
    private:
	vector<string> terms;
	vector<string>::size_type offset;
	bool before_start;

    public:
	VectorTermList(vector<string>::const_iterator begin,
		       vector<string>::const_iterator end)
	    : terms(begin, end), offset(0), before_start(true)
	{
	}

	VectorTermList(list<string>::const_iterator begin,
		       list<string>::const_iterator end)
	    : terms(begin, end), offset(0), before_start(true)
	{
	}

	// Gets size of termlist
	Xapian::termcount get_approx_size() const {
	    return terms.size();
	}

	// Gets current termname
	string get_termname() const {
	    Assert(!before_start && offset < terms.size());
	    return terms[offset];
	}

	// Get wdf of current term
	Xapian::termcount get_wdf() const {
	    Assert(!before_start && offset < terms.size());
	    return 1; // FIXME: or is Xapian::InvalidOperationError better?
	}

	// Get num of docs indexed by term
	Xapian::doccount get_termfreq() const {
            throw Xapian::InvalidOperationError("VectorTermList::get_termfreq() not supported");
	}

	/** next() causes the TermList to move to the next term in the list.
	 *  It must be called before any other methods.
	 *  If next() returns a non-zero pointer P, then the original
	 *  termlist should be deleted, and the original pointer replaced
	 *  with P.
	 *  In a leaf TermList, next() will always return 0.
	 */
	TermList * next() {
	    Assert(!at_end());
	    if (before_start)
		before_start = false;
	    else
		offset++;
	    return NULL;
	}

	TermList *skip_to(const string &/*tname*/) {
	    Assert(!at_end());
	    // termlist not ordered
	    Assert(false);
            throw Xapian::InvalidOperationError("VectorTermList::skip_to() not supported");
	}

	// True if we're off the end of the list
	bool at_end() const {
	    return !before_start && offset == terms.size();
	}

	Xapian::PositionIterator positionlist_begin() const {
	    throw Xapian::InvalidOperationError("VectorTermList::positionlist_begin() isn't meaningful");
	}

	Xapian::termcount positionlist_count() const {
	    throw Xapian::InvalidOperationError("VectorTermList::positionlist_count() isn't meaningful");
	}
};

#endif // OM_HGUARD_VECTORTERMLIST_H
