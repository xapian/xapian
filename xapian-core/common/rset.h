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
class StatsSource;

class RSetItem {
    public:
	RSetItem(om_docid did_new) : did(did_new) { return; }
	om_docid did;
};

/** A relevance set.  This is used internally, and performs the calculation
 *  and caching of relevant term frequencies. */
class RSet {
    private:
	// disallow copy
	RSet(const RSet &);
	void operator=(const RSet &);

	IRDatabase *root;

	map<om_termname, om_doccount> reltermfreqs;
	bool calculated_reltermfreqs;
    public:
	vector<RSetItem> documents; // FIXME - should be encapsulated

	// FIXME: should take a OmRefCntPtr to an IRDatabase
	RSet(IRDatabase *root_new);
	RSet(IRDatabase *root_new, const OmRSet & omrset);

	void add_document(om_docid did);
	void will_want_reltermfreq(om_termname tname);

	void calculate_stats();
	void give_stats_to_statssource(StatsSource &statssource);

	om_doccount get_rsize() const;
	om_doccount get_reltermfreq(om_termname tname) const;
};

///////////////////////////////
// Inline method definitions //
///////////////////////////////

// Empty initialisation
inline
RSet::RSet(IRDatabase *root_new)
	: root(root_new), calculated_reltermfreqs(false)
{}

// Initialise with an OMRset
inline
RSet::RSet(IRDatabase *root_new, const OmRSet & omrset)
	: root(root_new), calculated_reltermfreqs(false)
{
    set<om_docid>::const_iterator i;
    for(i = omrset.items.begin(); i != omrset.items.end(); i++) {
	add_document(*i);
    }
}

inline void
RSet::add_document(om_docid did)
{
    // FIXME - check that document isn't already in rset
    Assert(!calculated_reltermfreqs);
    documents.push_back(RSetItem(did));
}

inline void
RSet::will_want_reltermfreq(om_termname tname)
{
    reltermfreqs[tname] = 0;
}

inline om_doccount
RSet::get_rsize() const
{
    return documents.size();
}

inline om_doccount
RSet::get_reltermfreq(om_termname tname) const
{
    Assert(calculated_reltermfreqs);

    map<om_termname, om_doccount>::const_iterator rfreq;
    rfreq = reltermfreqs.find(tname);
    Assert(rfreq != reltermfreqs.end());

    return rfreq->second;
}

#endif /* OM_HGUARD_RSET_H */
