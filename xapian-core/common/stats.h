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

	/** Get the term of the given term.
	 *
	 *  This is "n_t", the number of documents in the collection indexed by
	 *  the given term.
	 */
	Xapian::doccount get_termfreq(const string & tname) const;

	/** Get the relevant term-frequency for the given term.
	 *
	 *  This is "r_t", the number of relevant documents in the collection
	 *  indexed by the given term.
	 */
	Xapian::doccount get_reltermfreq(const string & tname) const;
};

typedef Xapian::Weight::Internal Stats;

// Forward declaration.
class StatsSource;

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

	/** A collection of StatsSource children.
	 *  The Gatherer uses this to make sure that each child
	 *  has contributed before it will give out its statistics.
	 */
	mutable std::set<StatsSource *> sources;

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

	/** Add a StatsSource object to this gatherer.
	 *  The gatherer will include the source's statistics
	 *  into its own summary statistics.
	 */
	void add_child(StatsSource *source);

	/** Remove a StatsSource object from this gatherer.
	 *  This is needed in the case of some parts of the database dying
	 *  during the match.
	 */
	void remove_child(StatsSource *source);

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

/** Statistics source: gathers notifications of statistics which will be
 *  needed, and passes them on in bulk to a StatsGatherer.
 */
class StatsSource {
    private:
        // Prevent copying
        StatsSource(const StatsSource &);
        StatsSource & operator=(const StatsSource &);

	/** The gatherer which we report our information to, and ask
	 *  for the global information from.
	 */
	StatsGatherer * gatherer;

	/** The stats to give to the StatsGatherer.
	 */
	Stats my_stats;

	/** The collection statistics, held by the StatsGatherer.
	 *  0 before these have been retrieved.
	 */
	const Stats * total_stats;

    public:
	/// Constructor
	StatsSource(StatsGatherer *gatherer_) : gatherer(gatherer_), total_stats(0)
	{
	    gatherer->add_child(this);
	}

	/// Destructor
	~StatsSource() {
	    gatherer->remove_child(this);
	}

	/** Get all the statistics that don't depend on global stats.
	 */
	const Stats & get_my_stats() {
	    return my_stats;
	}

	///////////////////////////////////////////////////////////////////
	// Give statistics about yourself.  These are used to generate,
	// or check, the global information.

	/** Set stats about this sub-database: the number of documents and
	 *  average length of a document.
	 */
	void take_my_stats(Xapian::doccount csize, Xapian::doclength avlen);

	/** Set the term frequency in the sub-database which this stats
	 *  object represents.  This is the number of documents in
	 *  the sub-database indexed by the given term.
	 */
	void my_termfreq_is(const string & tname, Xapian::doccount tfreq);

	/** Set the relevant term-frequency in the sub-database which this
	 *  stats object represents.  This is the number of relevant
	 *  documents in the sub-database indexed by the given term.
	 */
	void my_reltermfreq_is(const string & tname, Xapian::doccount rtfreq);

	/** Set all the statistics about this sub-database.
	 *
	 *  The supplied statistics overwrite any statistics set previously.
	 */
	void set_my_stats(const Stats & stats);

	// /////////////////////////////////////////////////////////////////
	// Get the statistics back.  The result of each of the following
	// methods may be an approximation.

	/** Set the total_stats stored.
	 */
	void set_total_stats(const Stats * stats);

	/** Get the statistics for the whole collection.
	 */
	const Stats & get_total_stats() const {
	    Assert(total_stats != 0);
	    return *total_stats;
	}
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

///////////////////////////////////////////////
// Inline method definitions for StatsSource //
///////////////////////////////////////////////
 
inline void
StatsSource::take_my_stats(Xapian::doccount csize, Xapian::doclength avlen)
{
    Assert(total_stats == 0);
    my_stats.collection_size = csize;
    my_stats.average_length = avlen;
}

inline void
StatsSource::my_termfreq_is(const string & tname, Xapian::doccount tfreq)
{
    Assert(total_stats == 0);
    // Can be called a second time, if a term occurs multiple times in the
    // query.
    Assert(my_stats.termfreq.find(tname) == my_stats.termfreq.end() ||
	   my_stats.termfreq.find(tname)->second == tfreq);
    my_stats.termfreq[tname] = tfreq;
}

inline void
StatsSource::my_reltermfreq_is(const string & tname, Xapian::doccount rtfreq)
{
    Assert(total_stats == 0);
    // Can be called a second time, if a term occurs multiple times in the
    // query.
    Assert(my_stats.reltermfreq.find(tname) == my_stats.reltermfreq.end() ||
	   my_stats.reltermfreq.find(tname)->second == rtfreq);
    my_stats.reltermfreq[tname] = rtfreq;
}

#endif /* OM_HGUARD_STATS_H */
