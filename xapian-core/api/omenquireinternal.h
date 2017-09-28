/** @file omenquireinternal.h
 * @brief Internals
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2011,2014,2015,2016 Olly Betts
 * Copyright 2009 Lemur Consulting Ltd
 * Copyright 2011 Action Without Borders
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

#ifndef OM_HGUARD_OMENQUIREINTERNAL_H
#define OM_HGUARD_OMENQUIREINTERNAL_H

#include "xapian/database.h"
#include "xapian/document.h"
#include "xapian/enquire.h"
#include "xapian/query.h"
#include "xapian/rset.h"
#include "xapian/keymaker.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>

#include "result.h"
#include "weight/weightinternal.h"

using namespace std;

class OmExpand;
class MultiMatch;

namespace Xapian {

class TermIterator;

/** Internals of enquire system.
 *  This allows the implementation of Xapian::Enquire to be hidden and reference
 *  counted.
 */
class Enquire::Internal : public Xapian::Internal::intrusive_base {
    friend class MSet::Internal;
    private:
	/// The database which this enquire object uses.
	const Xapian::Database db;

	/// The user's query.
	Query query;

	/// The query length.
	termcount qlen;

	/// Copy not allowed
	Internal(const Internal &);
	/// Assignment not allowed
	void operator=(const Internal &);

    public:
	typedef enum { REL, VAL, VAL_REL, REL_VAL } sort_setting;

	Xapian::valueno collapse_key;

	Xapian::doccount collapse_max;

	Xapian::Enquire::docid_order order;

	int percent_cutoff;

	double weight_cutoff;

	Xapian::valueno sort_key;
	sort_setting sort_by;
	bool sort_value_forward;

	Xapian::Internal::opt_intrusive_ptr<KeyMaker> sorter;

	double time_limit;

	/** The weight to use for this query.
	 *
	 *  This is mutable so that the default BM25Weight object can be
	 *  created lazily when first required.
	 */
	mutable Weight * weight;

	/// The weighting scheme to use for query expansion.
	std::string eweightname;

	/// The parameter required for TradWeight query expansion.
	double expand_k;

	vector<Xapian::Internal::opt_intrusive_ptr<MatchSpy>> spies;

	explicit Internal(const Xapian::Database &databases);
	~Internal();

	/** Request a document from the database.
	 */
	void request_doc(Xapian::docid did) const;

	Xapian::Document get_document(Xapian::docid did) const;

	void set_query(const Query & query_, termcount qlen_);
	const Query & get_query() const;
	MSet get_mset(Xapian::doccount first, Xapian::doccount maxitems,
		      Xapian::doccount check_at_least,
		      const RSet *omrset,
		      const MatchDecider *mdecider) const;

	ESet get_eset(Xapian::termcount maxitems, const RSet & omrset, int flags,
		      const ExpandDecider *edecider, double min_wt) const;

	TermIterator get_matching_terms(Xapian::docid did) const;
	TermIterator get_matching_terms(const Xapian::MSetIterator &it) const;

	Xapian::doccount get_termfreq(const string &tname) const;

	string get_description() const;
};

}

#endif // OM_HGUARD_OMENQUIREINTERNAL_H
