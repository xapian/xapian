/* rset.h
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003,2005,2006 Olly Betts
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

#ifndef OM_HGUARD_RSET_H
#define OM_HGUARD_RSET_H

#include <set>
#include <map>
#include "omdebug.h"
#include <xapian/enquire.h>
#include "omenquireinternal.h"

/** A relevance set.
 *
 * This is used internally, and performs the calculation and caching of
 * relevant term frequencies.
 */
class RSetI {
    private:
	// disallow copy
	RSetI(const RSetI &);
	void operator=(const RSetI &);

	// FIXME: should use one or the other (probably Xapian::Database)
	const Xapian::Database root;
	const Xapian::Database::Internal *dbroot;

	std::map<string, Xapian::doccount> reltermfreqs;
	bool calculated_reltermfreqs;
    public:
	std::set<Xapian::docid> documents;

	RSetI(const Xapian::Database &root_, const Xapian::RSet & rset);
	RSetI(const Xapian::Database::Internal *dbroot_, const Xapian::RSet & rset);

	void will_want_reltermfreq(string tname);

	void calculate_stats();
	void give_stats_to_statssource(Xapian::Weight::Internal *statssource);

	Xapian::doccount get_rsize() const;
	Xapian::doccount get_reltermfreq(string tname) const;
};

///////////////////////////////
// Inline method definitions //
///////////////////////////////

/// Initialise with a Xapian::Database and a Xapian::RSet
inline
RSetI::RSetI(const Xapian::Database &root_, const Xapian::RSet & rset)
	: root(root_), dbroot(NULL), calculated_reltermfreqs(false),
	  documents(rset.internal->get_items())
{
}

/// Initialise with a Xapian::Database::Internal and a Xapian::RSet
inline
RSetI::RSetI(const Xapian::Database::Internal *dbroot_, const Xapian::RSet & rset)
	: dbroot(dbroot_), calculated_reltermfreqs(false),
	  documents(rset.internal->get_items())
{
}

#if 0
inline void
RSetI::add_document(Xapian::docid did)
{
    Assert(!calculated_reltermfreqs);
    Assert(!documents[did]);
    documents.insert(did);
}
#endif

inline void
RSetI::will_want_reltermfreq(string tname)
{
    reltermfreqs[tname] = 0;
}

inline Xapian::doccount
RSetI::get_rsize() const
{
    return documents.size();
}

inline Xapian::doccount
RSetI::get_reltermfreq(string tname) const
{
    Assert(calculated_reltermfreqs);

    std::map<string, Xapian::doccount>::const_iterator rfreq;
    rfreq = reltermfreqs.find(tname);
    Assert(rfreq != reltermfreqs.end());

    return rfreq->second;
}

#endif /* OM_HGUARD_RSET_H */
