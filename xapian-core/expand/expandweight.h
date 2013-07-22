/** @file expandweight.h
 * @brief Collate statistics and calculate the term weights for the ESet.
 */
/* Copyright (C) 2007,2008,2009,2011 Olly Betts
 * Copyright (C) 2013 Aarsh Shah
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_EXPANDWEIGHT_H
#define XAPIAN_INCLUDED_EXPANDWEIGHT_H

#include <xapian/database.h>

#include "api/termlist.h"
#include "internaltypes.h"

#include <string>
#include <vector>

namespace Xapian {
namespace Internal {

/// Collates statistics while calculating term weight in an ESet.
class ExpandStats {
    // Which databases in a multidb are included in termfreq.
    std::vector<bool> dbs_seen;

    // Average document length in the whole database.
    Xapian::doclength avlen;

  public:
    /// Size of the subset of a multidb to which the value in termfreq applies.
    Xapian::doccount dbsize;

    /// Term frequency (for a multidb, may be for a subset of the databases).
    Xapian::doccount termfreq;

    /// The number of times the term occurs in the rset.
    Xapian::termcount rcollection_freq;

    /// The sum of the lengths of the documents in the Rset.
    totlen_t rtotlen;

    /// The number of documents from the RSet indexed by the current term (r).
    Xapian::doccount rtermfreq;

    /// Keeps track of the index of the sub-database we're accumulating for.
    size_t db_index;

    ExpandStats(Xapian::doclength avlen_)
	: avlen(avlen_), dbsize(0), termfreq(0), rcollection_freq(0),
	  rtotlen(0), rtermfreq(0), db_index(0) {
    }

    void accumulate(Xapian::termcount wdf, Xapian::termcount doclen,
		    Xapian::doccount subtf, Xapian::doccount subdbsize) {
	// Boolean terms may have wdf == 0, but treat that as 1 so such terms
	// get a non-zero weight.
	if (wdf == 0) wdf = 1;
	++rtermfreq;

	// If we've not seen this sub-database before, then update dbsize and
	// termfreq and note that we have seen it.
	if (db_index >= dbs_seen.size() || !dbs_seen[db_index]) {
	    if (db_index >= dbs_seen.size()) dbs_seen.resize(db_index + 1);
	    dbs_seen[db_index] = true;
	    dbsize += subdbsize;
	    termfreq += subtf;
	    rcollection_freq += wdf;
	    rtotlen += doclen;
	}
    }
};

/// Class for calculating probabilistic ESet term weights.
class ExpandWeight {
    /// The combined database.
    const Xapian::Database db;

    /// The number of documents in the whole database.
    Xapian::doccount dbsize;

    /// Average document length in the whole database.
    Xapian::doclength avlen;

    /// The number of documents in the RSet.
    Xapian::doccount rsize;

    /// The collection frequency of the term.
    Xapian::termcount collection_freq;

    /// The total length of the databse.
    totlen_t collection_len;

    /** Should we calculate the exact term frequency when generating an ESet?
     *
     *  This only has any effect if we're using a combined database.
     *
     *  If this member is true, the exact term frequency will be obtained from
     *  the Database object.  If this member is false, then an approximation is
     *  used to estimate the term frequency based on the term frequencies in
     *  the sub-databases which we see while collating term statistics, and the
     *  relative sizes of the sub-databases.
     */
    bool use_exact_termfreq;

public:

    /// An ExpandStats object to accumulate statistics.
    ExpandStats stats;

    /** Constructor.
     *
     *  @param db_ The database.
     *  @param rsize_ The number of documents in the RSet.
     *  @param use_exact_termfreq_ When expanding over a combined database,
     *				   should we use the exact termfreq (if false
     *				   a cheaper approximation is used).
     */
    ExpandWeight(const Xapian::Database &db_,
	        Xapian::doccount rsize_,
	        bool use_exact_termfreq_)
	: db(db_), dbsize(db.get_doccount()), avlen(db.get_avlength()),
	  rsize(rsize_),  collection_freq(0),  collection_len(avlen * dbsize),
	  use_exact_termfreq(use_exact_termfreq_), stats(avlen) {}

    /** Get the term statistics.
     *  @param merger The tree of TermList objects.
     *  @param term The current term name.
     */
    void collect_stats(TermList * merger, const std::string & term);

    /// Allow the expansion scheme to perform initializations.
    virtual void init()  = 0;

    /// Calculate the weight.
    virtual double get_weight() const = 0;

protected:
    /// Return the average length of the databse.
    double get_avelen() const { return avlen; }

    /// Return the collection frequency of the term.
    Xapian::termcount get_collection_freq() const { return collection_freq; }

    /// Return the length of the collection.
    totlen_t get_collection_len() const { return collection_len; }

    /// Return the size of the database.
    Xapian::doccount get_dbsize() const { return dbsize; }
};

/// Only for testing purposes. will delete after testing is over.
class dummy : public ExpandWeight {
public :

dummy(const Xapian::Database &db_,
      Xapian::doccount rsize_,
      bool use_exact_termfreq_)
      : ExpandWeight(db_, rsize_, use_exact_termfreq_){ }

void init() { }

double get_weight() const { return get_collection_freq() + get_dbsize(); }
};
}
}
#endif // XAPIAN_INCLUDED_EXPANDWEIGHT_H
