/* rset.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#ifndef _rset_h_
#define _rset_h_

#include <vector>
#include <map>
#include "omassert.h"
#include "omenquire.h"

class IRDatabase;

class RSetItem {
    public:
	RSetItem(docid did_new) : did(did_new) { return; }
	docid did;
};

class RSet {
    private:
	IRDatabase *root;

	mutable map<termname, doccount> reltermfreqs;
	mutable bool initialised_reltermfreqs;
    public:
	vector<RSetItem> documents; // FIXME - should be encapsulated

	RSet(IRDatabase *root_new);
	RSet(IRDatabase *root_new, const OMRSet & _rset);

	void add_document(docid did);
	void will_want_termfreq(termname tname) const;
	doccount get_rsize() const;
	doccount get_reltermfreq(termname tname) const;
};

///////////////////////////////
// Inline method definitions //
///////////////////////////////

// Empty initialisation
inline
RSet::RSet(IRDatabase *root_new)
	: root(root_new), initialised_reltermfreqs(false)
{}

// Initialise with an OMRset
inline
RSet::RSet(IRDatabase *root_new, const OMRSet & _rset)
	: root(root_new), initialised_reltermfreqs(false)
{
    set<docid>::const_iterator i;
    for(i = _rset.items.begin(); i != _rset.items.end(); i++) {
	add_document(*i);
    }
}

inline void
RSet::add_document(docid did)
{
    // FIXME - check that document isn't already in rset
    Assert(!initialised_reltermfreqs);
    documents.push_back(RSetItem(did));
}

inline doccount
RSet::get_rsize() const
{
    return documents.size();
}

inline void
RSet::will_want_termfreq(termname tname) const
{
    reltermfreqs[tname] = 0;
}

#endif /* _rset_h_ */
