/* omenquireinternal.h: Internals
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002 Olly Betts
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

#ifndef OM_HGUARD_OMENQUIREINTERNAL_H
#define OM_HGUARD_OMENQUIREINTERNAL_H


#include "om/omenquire.h"
#include "refcnt.h"
#include <algorithm>
#include <math.h>
#include <map>
#include <set>

class OmErrorHandler;
class OmTermIterator;

/** An item in the ESet.
 *  This item contains the termname, and the weight calculated for
 *  the document.
 */
class OmESetItem {
    public:
	OmESetItem(om_weight wt_, string tname_)
		: wt(wt_), tname(tname_) {}
	/// Weight calculated.
	om_weight wt;
	/// Term suggested.
	string tname;
	
	/** Returns a string representing the eset item.
	 *  Introspection method.
	 */
	std::string get_description() const;
};

class OmESetIterator::Internal {
    private:
	friend class OmESetIterator; // allow access to it

	std::vector<OmESetItem>::const_iterator it;
	std::vector<OmESetItem>::const_iterator end;
    
    public:
        Internal(std::vector<OmESetItem>::const_iterator it_,
		 std::vector<OmESetItem>::const_iterator end_)
	    : it(it_), end(end_)
	{ }

        Internal(const Internal &other)
	    : it(other.it), end(other.end)
	{ }

	bool operator==(const OmESetIterator::Internal & other)
	{
	    return (it == other.it);
	}
};

/** An item resulting from a query.
 *  This item contains the document id, and the weight calculated for
 *  the document.
 */
class OmMSetItem {
    private:
    public:
	OmMSetItem(om_weight wt_, om_docid did_) 
		: wt(wt_), did(did_), collapse_count(0) {}

	OmMSetItem(om_weight wt_, om_docid did_, const string &key_)
		: wt(wt_), did(did_), collapse_key(key_), collapse_count(0) {}

	/** Weight calculated. */
	om_weight wt;

	/** Document id. */
	om_docid did;

	/** Value which was used to collapse upon.
	 *
	 *  If the collapse option is not being used, this will always
	 *  have a null value.
	 *
	 *  If the collapse option is in use, this will contain the collapse
	 *  key's value for this particular item.  If the key is not present
	 *  for this item, the value will be a null string.  Only one instance
	 *  of each key value (apart from the null string) will be present in
	 *  the items in the returned OmMSet.
	 */
	string collapse_key;

	/** Count of collapses done on collapse_key so far
	 *
	 * This is normally 0, and goes up for each collapse done
	 * It is not neccessarily an indication of how many collapses
	 * might be done if an exhaustive match was done
	 */
	 om_doccount collapse_count;

	/** For use by match_sort_key option - FIXME: document if this stays */
	/* FIXME: this being mutable is a gross hack */
	/* FIXME: why not just cache the OmDocument here!?! */
	mutable string sort_key;

	/** Returns a string representing the mset item.
	 *  Introspection method.
	 */
	std::string get_description() const;
};

/** Internals of enquire system.
 *  This allows the implementation of OmEnquire to be hidden and reference
 *  counted.
 */
class OmEnquire::Internal {
    public:
	/// Class holding the contents of the OmEnquire object.
	class Data;

	/// Constructor.
	Internal(RefCntPtr<Data> data_);

	// Default destructor, copy constructor and assignment.

	/** The contents of the omenquire object.  This pointer may never be
	 *  null.
	 */
	RefCntPtr<Data> data;
};

class OmEnquire::Internal::Data : public RefCntBase {
    private:
	/// The database which this enquire object uses.
	const OmDatabase db;

	/** The user's query.
	 *  This may need to be mutable in future so that it can be
	 *  replaced by an optimised version.
	 */
	OmQuery * query;

	/** Calculate the matching terms.
	 *  This method does the work for get_matching_terms().
	 */
	OmTermIterator calc_matching_terms(om_docid did) const;

	/// Copy not allowed
	Data(const Data &);
	/// Assignment not allowed
	void operator=(const Data &);

    public:
	om_valueno collapse_key;

	bool sort_forward;

	int percent_cutoff;

	om_weight weight_cutoff;

	om_valueno sort_key;
	int sort_bands;

	time_t bias_halflife;
	om_weight bias_weight;

	/** The error handler, if set.  (0 if not set).
	 */
	OmErrorHandler * errorhandler;

	std::map<std::string, const OmMatchDecider *> mdecider_map;

	mutable OmWeight * weight; // mutable so get_mset can set default

	Data(const OmDatabase &databases, OmErrorHandler * errorhandler_);
	~Data();

	/** Request a document from the database.
	 */
	void request_doc(const OmMSetItem &item) const;

	/** Read a previously requested document from the database.
	 */
	OmDocument read_doc(const OmMSetItem &item) const;

	void set_query(const OmQuery & query_);
	const OmQuery & get_query();
	OmMSet get_mset(om_doccount first, om_doccount maxitems,
			const OmRSet *omrset, 
			const OmMatchDecider *mdecider) const;
	OmESet get_eset(om_termcount maxitems, const OmRSet & omrset, int flags,
			double k, const OmExpandDecider *edecider) const;

	OmTermIterator get_matching_terms(om_docid did) const;
	OmTermIterator get_matching_terms(const OmMSetIterator &it) const;

	void register_match_decider(const std::string &name,
				    const OmMatchDecider *mdecider);
    
	std::string get_description() const;
};

class OmExpand;

class OmMSet::Internal {
    public:
	/// Class holding the actual data in the MSet.
	class Data;

	/// Constructor: makes a new empty mset.
	Internal();

	/// Constructor: makes a new mset from a pointer.
	Internal(RefCntPtr<Data> data_);

	// Default destructor, copy constructor and assignment.

	/** The actual data stored in the mset.  This pointer may never be
	 *  null.
	 */
	RefCntPtr<Data> data;
};

class OmMSet::Internal::Data : public RefCntBase {
    private:
	/// Factor to multiply weights by to convert them to percentages.
	double percent_factor;

	/// The set of documents which have been requested but not yet
	/// collected.
	mutable std::set<om_doccount> requested_docs;

	/// Cache of documents, indexed by rank.
	mutable std::map<om_doccount, OmDocument> rankeddocs;

	/// Read and cache the documents so far requested.
	void read_docs() const;

	/// Copy not allowed
	Data(const Data &);
	/// Assignment not allowed
	void operator=(const Data &);
    public:
	/// OmEnquire reference, for getting documents.
	RefCntPtr<const OmEnquire::Internal::Data> enquire;

	/** A structure containing the term frequency and weight for a
	 *  given query term.
	 */
	struct TermFreqAndWeight {
	    om_doccount termfreq;
	    om_weight termweight;
	};

	/** The term frequencies and weights returned by the match process.
	 *  This map will contain information for each term which was in                 *  the query.
	 */
	std::map<string, TermFreqAndWeight> termfreqandwts;

	/// A list of items comprising the (selected part of the) mset.
	std::vector<OmMSetItem> items;

	/// Rank of first item in Mset.
	om_doccount firstitem;

	om_doccount matches_lower_bound;

	om_doccount matches_estimated;

	om_doccount matches_upper_bound;

	om_weight max_possible;

	om_weight max_attained;

	Data()
		: percent_factor(0),
		  firstitem(0),
		  matches_lower_bound(0),
		  matches_estimated(0),
		  matches_upper_bound(0),
		  max_possible(0),
		  max_attained(0) {}

	Data(om_doccount firstitem_,
	     om_doccount matches_upper_bound_,
	     om_doccount matches_lower_bound_,
	     om_doccount matches_estimated_,
	     om_weight max_possible_,
	     om_weight max_attained_,
	     const std::vector<OmMSetItem> &items_,
	     const std::map<string, TermFreqAndWeight> &termfreqandwts_,
	     om_weight percent_factor_)
		: percent_factor(percent_factor_),
		  termfreqandwts(termfreqandwts_),
		  items(items_),
		  firstitem(firstitem_),
		  matches_lower_bound(matches_lower_bound_),
		  matches_estimated(matches_estimated_),
		  matches_upper_bound(matches_upper_bound_),
		  max_possible(max_possible_),
		  max_attained(max_attained_) {}

	/// get a document by rank, via the cache.
	OmDocument get_doc_by_rank(om_doccount rank) const;

	/// Converts a weight to a percentage weight
	om_percent convert_to_percent_internal(om_weight wt) const;

	/** Returns a string representing the mset.
	 *  Introspection method.
	 */
	std::string get_description() const;

	/** Fetch items specified into the document cache.
	 */
	void fetch_items(om_doccount rank,
			 std::vector<OmMSetItem>::const_iterator begin,
			 std::vector<OmMSetItem>::const_iterator end) const;
};

class OmMSetIterator::Internal {
    private:
	friend class OmMSetIterator; // allow access to it
	friend class OmMSet;

	std::vector<OmMSetItem>::const_iterator it;
	std::vector<OmMSetItem>::const_iterator end;

	om_doccount currrank;
	RefCntPtr<OmMSet::Internal::Data> msetdata;
    public:
        Internal(std::vector<OmMSetItem>::const_iterator it_,
		 std::vector<OmMSetItem>::const_iterator end_,
		 om_doccount currrank_,
		 RefCntPtr<OmMSet::Internal::Data> msetdata_)
	    : it(it_), end(end_), currrank(currrank_), msetdata(msetdata_)
	{ }

        Internal(const Internal &other)
	    : it(other.it), end(other.end),
	      currrank(other.currrank), msetdata(other.msetdata)
	{ }

	bool operator==(const OmMSetIterator::Internal & other)
	{
	    return (it == other.it);
	}
};

class OmESet::Internal {
    friend class OmESet;
    friend class OmExpand;
    private:
	/// A list of items comprising the (selected part of the) eset.
	std::vector<OmESetItem> items;

	/** A lower bound on the number of terms which are in the full
	 *  set of results of the expand.  This will be greater than or
	 *  equal to items.size()
	 */
	om_termcount ebound;

    public:
	Internal() : ebound(0) {}

	/** Returns a string representing the eset.
	 *  Introspection method.
	 */
	std::string get_description() const;
};

class RSet;

class OmRSet::Internal {
    friend class OmRSet;
    friend class RSet;
    friend class OmExpand;
    friend class MultiMatch;
    friend std::string omrset_to_string(const OmRSet &omrset);

    private:
	/// Items in the relevance set.
	std::set<om_docid> items;

    public:
	/** Returns a string representing the rset.
	 *  Introspection method.
	 */
	std::string get_description() const;
};



inline
OmEnquire::Internal::Internal(RefCntPtr<Data> data_)
	: data(data_)
{}

inline
OmMSet::Internal::Internal()
	: data(new OmMSet::Internal::Data())
{}

inline
OmMSet::Internal::Internal(RefCntPtr<Data> data_)
	: data(data_)
{}

#endif // OM_HGUARD_OMENQUIREINTERNAL_H
