/* omenquireinternal.h: Internals
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010 Olly Betts
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
#include "xapian/keymaker.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <set>

using namespace std;

class OmExpand;
class MultiMatch;

namespace Xapian {

class ErrorHandler;
class TermIterator;

namespace Internal {

/** An item resulting from a query.
 *  This item contains the document id, and the weight calculated for
 *  the document.
 */
class MSetItem {
    public:
	MSetItem(Xapian::weight wt_, Xapian::docid did_)
		: wt(wt_), did(did_), collapse_count(0) {}

	MSetItem(Xapian::weight wt_, Xapian::docid did_, const string &key_)
		: wt(wt_), did(did_), collapse_key(key_), collapse_count(0) {}

	MSetItem(Xapian::weight wt_, Xapian::docid did_, const string &key_,
		 Xapian::doccount collapse_count_)
		: wt(wt_), did(did_), collapse_key(key_),
		  collapse_count(collapse_count_) {}

	void swap(MSetItem & o) {
	    std::swap(wt, o.wt);
	    std::swap(did, o.did);
	    std::swap(collapse_key, o.collapse_key);
	    std::swap(collapse_count, o.collapse_count);
	    std::swap(sort_key, o.sort_key);
	}

	/** Weight calculated. */
	Xapian::weight wt;

	/** Document id. */
	Xapian::docid did;

	/** Value which was used to collapse upon.
	 *
	 *  If the collapse option is not being used, this will always
	 *  have a null value.
	 *
	 *  If the collapse option is in use, this will contain the collapse
	 *  key's value for this particular item.  If the key is not present
	 *  for this item, the value will be a null string.  Only one instance
	 *  of each key value (apart from the null string) will be present in
	 *  the items in the returned Xapian::MSet.
	 */
	string collapse_key;

	/** Count of collapses done on collapse_key so far
	 *
	 * This is normally 0, and goes up for each collapse done
	 * It is not necessarily an indication of how many collapses
	 * might be done if an exhaustive match was done
	 */
	Xapian::doccount collapse_count;

	/** Used when sorting by value. */
	string sort_key;

	/// Return a string describing this object.
	string get_description() const;
};

}

/** Internals of enquire system.
 *  This allows the implementation of Xapian::Enquire to be hidden and reference
 *  counted.
 */
class Enquire::Internal : public Xapian::Internal::RefCntBase {
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

	percent percent_cutoff;

	Xapian::weight weight_cutoff;

	Xapian::valueno sort_key;
	sort_setting sort_by;
	bool sort_value_forward;

	KeyMaker * sorter;

	/** The error handler, if set.  (0 if not set).
	 */
	ErrorHandler * errorhandler;

	/** The weight to use for this query.
	 *
	 *  This is mutable so that the default BM25Weight object can be
	 *  created lazily when first required.
	 */
	mutable Weight * weight;

	vector<MatchSpy *> spies;

	Internal(const Xapian::Database &databases, ErrorHandler * errorhandler_);
	~Internal();

	/** Request a document from the database.
	 */
	void request_doc(const Xapian::Internal::MSetItem &item) const;

	/** Read a previously requested document from the database.
	 */
	Xapian::Document read_doc(const Xapian::Internal::MSetItem &item) const;

	void set_query(const Query & query_, termcount qlen_);
	const Query & get_query();
	MSet get_mset(Xapian::doccount first, Xapian::doccount maxitems,
		      Xapian::doccount check_at_least,
		      const RSet *omrset,
		      const MatchDecider *mdecider,
		      const MatchDecider *matchspy_legacy) const;

	ESet get_eset(Xapian::termcount maxitems, const RSet & omrset, int flags,
		      double k, const ExpandDecider *edecider, Xapian::weight min_wt) const;

	TermIterator get_matching_terms(Xapian::docid did) const;
	TermIterator get_matching_terms(const Xapian::MSetIterator &it) const;

	Xapian::doccount get_termfreq(const string &tname) const;

	string get_description() const;
};

class MSet::Internal : public Xapian::Internal::RefCntBase {
    public:
	/// Factor to multiply weights by to convert them to percentages.
	double percent_factor;

    private:
	/** The set of documents which have been requested but not yet
	 *  collected.
	 */
	mutable set<Xapian::doccount> requested_docs;

	/// Cache of documents, indexed by MSet index.
	mutable map<Xapian::doccount, Xapian::Document> indexeddocs;

	/// Read and cache the documents so far requested.
	void read_docs() const;

	/// Copy not allowed
	Internal(const Internal &);
	/// Assignment not allowed
	void operator=(const Internal &);

    public:
	/// Xapian::Enquire reference, for getting documents.
	Xapian::Internal::RefCntPtr<const Enquire::Internal> enquire;

	/** A structure containing the term frequency and weight for a
	 *  given query term.
	 */
	struct TermFreqAndWeight {
	    TermFreqAndWeight() { }
	    explicit TermFreqAndWeight(Xapian::doccount tf)
		: termfreq(tf), termweight(0.0) { }
	    TermFreqAndWeight(Xapian::doccount tf, Xapian::weight wt)
		: termfreq(tf), termweight(wt) { }
	    Xapian::doccount termfreq;
	    Xapian::weight termweight;
	};

	/** The term frequencies and weights returned by the match process.
	 *
	 *  This map contains information for each term which was in
	 *  the query.
	 */
	map<string, TermFreqAndWeight> termfreqandwts;

	/// A list of items comprising the (selected part of the) MSet.
	vector<Xapian::Internal::MSetItem> items;

	/// Rank of first item in MSet.
	Xapian::doccount firstitem;

	Xapian::doccount matches_lower_bound;

	Xapian::doccount matches_estimated;

	Xapian::doccount matches_upper_bound;

	Xapian::doccount uncollapsed_lower_bound;

	Xapian::doccount uncollapsed_estimated;

	Xapian::doccount uncollapsed_upper_bound;

	Xapian::weight max_possible;

	Xapian::weight max_attained;

	Internal()
		: percent_factor(0),
		  firstitem(0),
		  matches_lower_bound(0),
		  matches_estimated(0),
		  matches_upper_bound(0),
		  uncollapsed_lower_bound(0),
		  uncollapsed_estimated(0),
		  uncollapsed_upper_bound(0),
		  max_possible(0),
		  max_attained(0) {}

	/// Note: destroys parameter items.
	Internal(Xapian::doccount firstitem_,
	     Xapian::doccount matches_upper_bound_,
	     Xapian::doccount matches_lower_bound_,
	     Xapian::doccount matches_estimated_,
	     Xapian::doccount uncollapsed_upper_bound_,
	     Xapian::doccount uncollapsed_lower_bound_,
	     Xapian::doccount uncollapsed_estimated_,
	     Xapian::weight max_possible_,
	     Xapian::weight max_attained_,
	     vector<Xapian::Internal::MSetItem> &items_,
	     const map<string, TermFreqAndWeight> &termfreqandwts_,
	     Xapian::weight percent_factor_)
		: percent_factor(percent_factor_),
		  termfreqandwts(termfreqandwts_),
		  firstitem(firstitem_),
		  matches_lower_bound(matches_lower_bound_),
		  matches_estimated(matches_estimated_),
		  matches_upper_bound(matches_upper_bound_),
		  uncollapsed_lower_bound(uncollapsed_lower_bound_),
		  uncollapsed_estimated(uncollapsed_estimated_),
		  uncollapsed_upper_bound(uncollapsed_upper_bound_),
		  max_possible(max_possible_),
		  max_attained(max_attained_) {
	    std::swap(items, items_);
	}

	/// get a document by index in MSet, via the cache.
	Xapian::Document get_doc_by_index(Xapian::doccount index) const;

	/// Converts a weight to a percentage weight
	percent convert_to_percent_internal(Xapian::weight wt) const;

	/// Return a string describing this object.
	string get_description() const;

	/** Fetch items specified into the document cache.
	 */
	void fetch_items(Xapian::doccount first, Xapian::doccount last) const;
};

class RSet::Internal : public Xapian::Internal::RefCntBase {
    friend class Xapian::RSet;

    private:
	/// Items in the relevance set.
	set<Xapian::docid> items;

    public:
	const set<Xapian::docid> & get_items() const { return items; }

	/// Return a string describing this object.
	string get_description() const;
};

}

#endif // OM_HGUARD_OMENQUIREINTERNAL_H
