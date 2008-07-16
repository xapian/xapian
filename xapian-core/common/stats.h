/* stats.h: Handling of statistics needed for the search.
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2005,2007 Olly Betts
 * Copyright 2007 Lemur Consulting Ltd
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

#ifndef OM_HGUARD_STATS_H
#define OM_HGUARD_STATS_H

#include "xapian/types.h"
#include "omassert.h"
#include <string>
#include <map>

#include "autoptr.h" // FIXME:1.1: remove this
#include "weightinternal.h" // FIXME:1.1: remove this
#include <list> // FIXME:1.1: remove this

using namespace std;

/** Class to hold statistics for a given collection. */
class Stats {
    public:
	/** Number of documents in the collection. */
	Xapian::doccount collection_size;

	/** Number of relevant documents in the collection. */
	Xapian::doccount rset_size;

	/** Average length of documents in the collection. */
	Xapian::doclength average_length;

	/** Map of term frequencies for the collection. */
	std::map<string, Xapian::doccount> termfreq;

	/** Map of relevant term frequencies for the collection. */
	std::map<string, Xapian::doccount> reltermfreq;


	Stats() : collection_size(0),
		  rset_size(0),
		  average_length(1.0)
	{}

	/** Add in the supplied statistics from a sub-database.
	 */
	Stats & operator +=(const Stats & inc);

	/** Get the term-frequency of the given term.
	 *
	 *  This is "n_t", the number of documents in the collection indexed by
	 *  the given term.
	 */
	Xapian::doccount get_termfreq(const string & tname) const;

	/** Set the term-frequency for the given term.
	 */
	void set_termfreq(const string & tname, Xapian::doccount tfreq);

	/** Get the relevant term-frequency for the given term.
	 *
	 *  This is "r_t", the number of relevant documents in the collection
	 *  indexed by the given term.
	 */
	Xapian::doccount get_reltermfreq(const string & tname) const;

	/** Set the relevant term-frequency for the given term.
	 */
	void set_reltermfreq(const string & tname, Xapian::doccount rtfreq);

	/// Return a string describing this object.
	string get_description() const;


	/** A list of all the Xapian::Weight::Internals allocated.
	 *
	 *  These will be deleted by the destructor.
	 *
	 *  FIXME:1.1: this should be removed for the 1.1 release.  In 1.0, the
	 *  constructor of Xapian::Weight doesn't initialise its "internal"
	 *  member to NULL.  This means that we can't delete the object pointed
	 *  to by "internal" in the destructor, because it may not be
	 *  initialised.  Therefore, we have to register the objects somewhere
	 *  else, and delete them another way.  We use the Stats object for
	 *  this purpose, since they're related to it.
	 */
	mutable list<Xapian::Weight::Internal *> weight_internals;

	/** Destructor - delete the internals registered.
	 *
	 *  FIXME:1.1: remove this - just use the default.
	 */
	~Stats() {
	    list<Xapian::Weight::Internal *>::const_iterator i;
	    for (i = weight_internals.begin(); i != weight_internals.end(); ++i)
	    {
		delete *i;
	    }
	}

	/** Create and return a Weight::Internal object with global statistics.
	 *
	 *  FIXME:1.1: remove this method - just create it directly.
	 *
	 *  All term-specific statistics will be set to 0.
	 *
	 *  The caller must NOT delete the returned object after use - it is
	 *  owned by the Stats object.
	 *
	 *  @param stats Object containing the statistics to use.
	 */
	Xapian::Weight::Internal * create_weight_internal() const
	{
	    AutoPtr<Xapian::Weight::Internal> wti(new Xapian::Weight::Internal(*this));
	    weight_internals.push_back(wti.get());
	    return wti.release();
	}

	/** Create and return a Weight::Internal object with global and term
	 *  statistics.
	 *
	 *  FIXME:1.1: remove this method - just create it directly.
	 *
	 *  The caller must NOT delete the returned object after use - it is
	 *  owned by the Stats object.
	 *
	 *  @param stats Object containing the statistics to use.
	 *  @param tname The term to read the term-specific statistics for.
	 */
	Xapian::Weight::Internal * create_weight_internal(const string & tname) const
	{
	    AutoPtr<Xapian::Weight::Internal> wti(new Xapian::Weight::Internal(*this, tname));
	    weight_internals.push_back(wti.get());
	    return wti.release();
	}
};

/////////////////////////////////////////
// Inline method definitions for Stats //
/////////////////////////////////////////

inline Stats &
Stats::operator +=(const Stats & inc)
{
    // Set the new collection size and average length.
    Xapian::doccount new_collection_size = collection_size + inc.collection_size;
    if (new_collection_size != 0) {
	// Cope with adding in a collection of zero size at the beginning:
	// perhaps we have multiple databases, but some are not yet populated
	average_length = (average_length * collection_size +
			  inc.average_length * inc.collection_size) /
			 new_collection_size;
    }
    collection_size = new_collection_size;

    // Add the rset size.
    rset_size += inc.rset_size;

    // Add termfreqs and reltermfreqs
    std::map<string, Xapian::doccount>::const_iterator i;
    for (i = inc.termfreq.begin(); i != inc.termfreq.end(); ++i) {
	termfreq[i->first] += i->second;
    }
    for (i = inc.reltermfreq.begin(); i != inc.reltermfreq.end(); ++i) {
	reltermfreq[i->first] += i->second;
    }
    return *this;
}

inline Xapian::doccount
Stats::get_termfreq(const string & tname) const
{
    // We pass an empty string for tname when calculating the extra weight.
    if (tname.empty()) return 0;

    std::map<string, Xapian::doccount>::const_iterator tfreq;
    tfreq = termfreq.find(tname);
    Assert(tfreq != termfreq.end());
    return tfreq->second;
}

inline void
Stats::set_termfreq(const string & tname, Xapian::doccount tfreq)
{
    // Can be called a second time, if a term occurs multiple times in the
    // query; if this happens, the termfreq should be the same each time.
    Assert(termfreq.find(tname) == termfreq.end() ||
	   termfreq.find(tname)->second == tfreq);
    termfreq[tname] = tfreq;
}

inline Xapian::doccount
Stats::get_reltermfreq(const string & tname) const
{
    // We pass an empty string for tname when calculating the extra weight.
    if (tname.empty()) return 0;

    std::map<string, Xapian::doccount>::const_iterator rtfreq;
    rtfreq = reltermfreq.find(tname);
    Assert(rtfreq != reltermfreq.end());
    return rtfreq->second;
}

inline void
Stats::set_reltermfreq(const string & tname, Xapian::doccount rtfreq)
{
    // Can be called a second time, if a term occurs multiple times in the
    // query; if this happens, the termfreq should be the same each time.
    Assert(reltermfreq.find(tname) == reltermfreq.end() ||
	   reltermfreq.find(tname)->second == rtfreq);
    reltermfreq[tname] = rtfreq;
}

#endif /* OM_HGUARD_STATS_H */
