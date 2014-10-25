/** @file weightinternal.h
 * @brief Xapian::Weight::Internal class, holding database and term statistics.
 */
/* Copyright (C) 2007 Lemur Consulting Ltd
 * Copyright (C) 2009,2010,2011,2013,2014 Olly Betts
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

#ifndef XAPIAN_INCLUDED_WEIGHTINTERNAL_H
#define XAPIAN_INCLUDED_WEIGHTINTERNAL_H

#include "xapian/weight.h"

#include "xapian/database.h"
#include "xapian/query.h"

#include "backends/database.h"
#include "internaltypes.h"
#include "omassert.h"

#include <map>
#include <string>

/// The frequencies for a term.
struct TermFreqs {
    Xapian::doccount termfreq;
    Xapian::doccount reltermfreq;
    Xapian::termcount collfreq;
    double max_part;

    TermFreqs() : termfreq(0), reltermfreq(0), collfreq(0), max_part(0.0) {}
    TermFreqs(Xapian::doccount termfreq_,
	      Xapian::doccount reltermfreq_,
	      Xapian::termcount collfreq_,
	      double max_part_ = 0.0)
	: termfreq(termfreq_),
	  reltermfreq(reltermfreq_),
	  collfreq(collfreq_),
	  max_part(max_part_) {}

    void operator +=(const TermFreqs & other) {
	termfreq += other.termfreq;
	reltermfreq += other.reltermfreq;
	collfreq += other.collfreq;
	max_part += other.max_part;
    }

    /// Return a std::string describing this object.
    std::string get_description() const;
};

namespace Xapian {

class RSet;

/** Class to hold statistics for a given collection. */
class Weight::Internal {
  public:
    /** Total length of all documents in the collection. */
    totlen_t total_length;

    /** Number of documents in the collection. */
    Xapian::doccount collection_size;

    /** Number of relevant documents in the collection. */
    Xapian::doccount rset_size;

    /** Number of terms in the collection. */
    Xapian::termcount total_term_count;

    /** Has max_part been set for any term?
     *
     *  If not, we can avoid having to serialise max_part.
     */
    bool have_max_part;

    /** Database to get the bounds on doclength and wdf from. */
    Xapian::Database db;

    /** Map of term frequencies and relevant term frequencies for the
     *  collection. */
    std::map<std::string, TermFreqs> termfreqs;

    Internal()
	: total_length(0), collection_size(0), rset_size(0),
	  total_term_count(0), have_max_part(false) { }

    /** Add in the supplied statistics from a sub-database.
     *
     *  Used for remote databases, where we pass across a serialised stats
     *  object, unserialise it, and add it to our total.
     */
    Internal & operator +=(const Internal & inc);

    /// Mark the terms we need to collate stats for.
    void mark_wanted_terms(const Xapian::Query &query) {
	Xapian::TermIterator t;
	for (t = query.get_terms_begin(); t != Xapian::TermIterator(); ++t) {
	    termfreqs.insert(make_pair(*t, TermFreqs()));
	}
    }

    /// Accumulate the rtermfreqs for terms marked by mark_wanted_terms().
    void accumulate_stats(const Xapian::Database::Internal &sub_db,
			  const Xapian::RSet &rset);

    /** Get the frequencies for the given term.
     *
     *  termfreq is "n_t", the number of documents in the collection indexed by
     *  the given term.
     *
     *  reltermfreq is "r_t", the number of relevant documents in the
     *  collection indexed by the given term.
     *
     *  collfreq is the total number of occurrences of the term in all
     *  documents.
     */
    bool get_stats(const std::string & term,
		   Xapian::doccount & termfreq,
		   Xapian::doccount & reltermfreq,
		   Xapian::termcount & collfreq) const {
	// We pass an empty std::string for term when calculating the extra
	// weight.
	if (term.empty()) {
	    termfreq = collection_size;
	    collfreq = collection_size;
	    reltermfreq = rset_size;
	    return true;
	}

	map<string, TermFreqs>::const_iterator i = termfreqs.find(term);
	if (i == termfreqs.end()) {
	    termfreq = reltermfreq = collfreq = 0;
	    return false;
	}

	termfreq = i->second.termfreq;
	reltermfreq = i->second.reltermfreq;
	collfreq = i->second.collfreq;
	return true;
    }

    /// Get just the termfreq.
    bool get_stats(const std::string & term,
		   Xapian::doccount & termfreq) const {
	Xapian::doccount dummy1;
	Xapian::termcount dummy2;
	return get_stats(term, termfreq, dummy1, dummy2);
    }

    /// Get the termweight.
    bool get_termweight(const std::string & term, double & termweight) {
	termweight = 0.0;
	if (term.empty()) {
	    return false;
	}

	map<string, TermFreqs>::const_iterator i = termfreqs.find(term);
	if (i == termfreqs.end()) {
	    return false;
	}

	termweight = i->second.max_part;
	return true;
    }

    /// Set max_part for a term.
    void set_max_part(const std::string & term, double max_part) {
	have_max_part = true;
	Assert(!term.empty());
	map<string, TermFreqs>::iterator i = termfreqs.find(term);
	Assert(i != termfreqs.end());
	i->second.max_part += max_part;
    }

    Xapian::doclength get_average_length() const {
	if (rare(collection_size == 0)) return 0;
	return Xapian::doclength(total_length) / collection_size;
    }

    /** Set the "bounds" stats from Database @a db. */
    void set_bounds_from_db(const Xapian::Database &db_) { db = db_; }

    /// Return a std::string describing this object.
    std::string get_description() const;
};

}

#endif // XAPIAN_INCLUDED_WEIGHTINTERNAL_H
