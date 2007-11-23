/* stats.h: Handling of statistics needed for the search.
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2005,2007 Olly Betts
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

#include <string>

#include <xapian/enquire.h> // for Xapian::Weight
#include "omassert.h"
#include "omdebug.h"
#include <map>
#include <set>

using namespace std;

/** Class to hold statistics for a given collection. */
class Xapian::Weight::Internal {
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


	Internal() : collection_size(0),
		  rset_size(0),
		  average_length(1.0)
	{}

	/** Add in the supplied statistics from a sub-database.
	 */
	Internal & operator +=(const Internal & inc);

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
};

typedef Xapian::Weight::Internal Stats;

/** Statistics collector: gathers statistics from sub-databases, puts them
 *  together to build statistics for whole collection, and returns the
 *  overall statistics to each sub-database.
 */
class StatsGatherer {
    protected:
	/** Flag to say that statistics have been gathered.
	 *  Some entries in stats are only valid after this.
	 *  Stats should also not be modified before this.
	 */
	mutable bool have_gathered;

	/** The total statistics gathered for the whole collection.
	 */
	mutable Stats total_stats;
    public:
	StatsGatherer();
	virtual ~StatsGatherer() {}

	/** Set the global collection statistics.
	 *  Should be called before the match is performed.
	 */
	virtual void set_global_stats(Xapian::doccount rset_size);

	/** Contribute some statistics to the overall statistics.
	 *  Should only be called once by each sub-database.
	 */
	void contrib_stats(const Stats & extra_stats);

	/** Get the collected statistics for the whole collection.
	 */
	virtual const Stats * get_stats() const = 0;
};

/** The top-level StatsGatherer.
 */
class LocalStatsGatherer : public StatsGatherer {
    public:
	const Stats *get_stats() const;
};

/////////////////////////////////////////
// Inline method definitions for Stats //
/////////////////////////////////////////

inline Xapian::Weight::Internal &
Xapian::Weight::Internal::operator +=(const Xapian::Weight::Internal & inc)
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

    // rset_size is set at the top level, we don't want
    // to pass it back up again.
    Assert(inc.rset_size == 0);

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
Xapian::Weight::Internal::get_termfreq(const string & tname) const
{
    // We pass an empty string for tname when calculating the extra weight.
    if (tname.empty()) return 0;

    std::map<string, Xapian::doccount>::const_iterator tfreq;
    tfreq = termfreq.find(tname);
    Assert(tfreq != termfreq.end());
    return tfreq->second;
}

inline void
Xapian::Weight::Internal::set_termfreq(const string & tname,
				       Xapian::doccount tfreq)
{
    // Can be called a second time, if a term occurs multiple times in the
    // query; if this happens, the termfreq should be the same each time.
    Assert(termfreq.find(tname) == termfreq.end() ||
	   termfreq.find(tname)->second == tfreq);
    termfreq[tname] = tfreq;
}

inline Xapian::doccount
Xapian::Weight::Internal::get_reltermfreq(const string & tname) const
{
    // We pass an empty string for tname when calculating the extra weight.
    if (tname.empty()) return 0;

    std::map<string, Xapian::doccount>::const_iterator rtfreq;
    rtfreq = reltermfreq.find(tname);
    Assert(rtfreq != reltermfreq.end());
    return rtfreq->second;
}

inline void
Xapian::Weight::Internal::set_reltermfreq(const string & tname,
					  Xapian::doccount rtfreq)
{
    // Can be called a second time, if a term occurs multiple times in the
    // query; if this happens, the termfreq should be the same each time.
    Assert(reltermfreq.find(tname) == reltermfreq.end() ||
	   reltermfreq.find(tname)->second == rtfreq);
    reltermfreq[tname] = rtfreq;
}

/////////////////////////////////////////////////
// Inline method definitions for StatsGatherer //
/////////////////////////////////////////////////

inline
StatsGatherer::StatsGatherer()
	: have_gathered(false)
{}

inline void
StatsGatherer::set_global_stats(Xapian::doccount rset_size)
{
    total_stats.rset_size = rset_size;
}

#endif /* OM_HGUARD_STATS_H */
