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

#ifndef OM_HGUARD_RSET_H
#define OM_HGUARD_RSET_H

#include <vector>
#include <map>
#include "omassert.h"
#include "om/omenquire.h"

class IRDatabase;

class RSetItem {
    public:
	RSetItem(om_docid did_new) : did(did_new) { return; }
	om_docid did;
};

class RSet {
    private:
	// disallow copy
	RSet(const RSet &);
	void operator=(const RSet &);

	IRDatabase *root;

	mutable map<om_termname, om_doccount> reltermfreqs;
	mutable bool initialised_reltermfreqs;
    public:
	vector<RSetItem> documents; // FIXME - should be encapsulated

	RSet(IRDatabase *root_new);
	RSet(IRDatabase *root_new, const OmRSet & _rset);

	void add_document(om_docid did);
	void will_want_termfreq(om_termname tname) const;
	om_doccount get_rsize() const;
	om_doccount get_reltermfreq(om_termname tname) const;
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
RSet::RSet(IRDatabase *root_new, const OmRSet & _rset)
	: root(root_new), initialised_reltermfreqs(false)
{
    set<om_docid>::const_iterator i;
    for(i = _rset.items.begin(); i != _rset.items.end(); i++) {
	add_document(*i);
    }
}

inline void
RSet::add_document(om_docid did)
{
    // FIXME - check that document isn't already in rset
    Assert(!initialised_reltermfreqs);
    documents.push_back(RSetItem(did));
}

inline om_doccount
RSet::get_rsize() const
{
    return documents.size();
}

inline void
RSet::will_want_termfreq(om_termname tname) const
{
    reltermfreqs[tname] = 0;
}

#endif /* OM_HGUARD_RSET_H */
