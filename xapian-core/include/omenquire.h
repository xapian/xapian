/* omenquire.h
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

#ifndef OM_HGUARD_OMENQUIRE_H
#define OM_HGUARD_OMENQUIRE_H

#include "omtypes.h"
#include <string>
#include <vector>
#include <set>

class OmEnquireInternal; // Internal state of enquire
class OmEnquire;         // Declare Enquire class
class OmMatch;           // Class which performs queries

///////////////////////////////////////////////////////////////////
// OmQuery class
// =============
// Representation of a query

/// Enum of possible query operations
enum om_queryop {
    OM_MOP_LEAF,     /// For internal use - must never be specified as parameter

    OM_MOP_AND,      /// Return iff both subqueries are satisfied
    OM_MOP_OR,       /// Return if either subquery is satisfied
    OM_MOP_AND_NOT,  /// Return if left but not right satisfied
    OM_MOP_XOR,      /// Return if one query satisfied, but not both
    OM_MOP_AND_MAYBE,/// Return iff left satisfied, but use weights from both
    OM_MOP_FILTER    /// As AND, but use only weights from left subquery
};

/** Class representing a query.
 *  Queries are represented as a heirarchy of classes.
 */
class OmQuery {
    friend class OmMatch;
    private:
	bool isnull;
	bool isbool;

	om_queryop op;       /// Operation to be performed at this node
	vector<OmQuery *> subqs;  /// Sub queries on which to perform operation
	om_termcount qlen;   /// Length of query

	// Fields used only for leaf nodes.
	om_termname tname;   /// Term that this leaf represents
	om_termpos term_pos; /// Position in query of this term
	om_termcount wqf;    /// Within query frequency of this term

	void initialise_from_copy(const OmQuery & copyme);
	void initialise_from_vector(const vector<OmQuery>::const_iterator qbegin,
				    const vector<OmQuery>::const_iterator qend);
	void initialise_from_vector(const vector<OmQuery *>::const_iterator qbegin,
				    const vector<OmQuery *>::const_iterator qend);
    public:
	/** A query consisting of a single term. */
	OmQuery(const om_termname & tname_,
		om_termcount wqf_ = 1,
		om_termpos term_pos_ = 0);

	/** A query consisting of two subqueries, opp-ed together. */
	OmQuery(om_queryop op_, const OmQuery & left, const OmQuery & right);

	/** A set of OmQuery's, merged together with specified operator.
	 * (Takes begin and end iterators).
	 * The only operators allowed are AND and OR. */
	OmQuery(om_queryop op_,
		const vector<OmQuery>::const_iterator qbegin,
		const vector<OmQuery>::const_iterator qend);

	/** As before, but uses a vector of OmQuery pointers. */
	OmQuery(om_queryop op_,
		const vector<OmQuery *>::const_iterator qbegin,
		const vector<OmQuery *>::const_iterator qend);

	/** As before, except subqueries are all individual terms. */
	OmQuery(om_queryop op_,
		const vector<om_termname>::const_iterator tbegin,
		const vector<om_termname>::const_iterator tend);

	/** Copy constructor. */
	OmQuery(const OmQuery & copyme);

	/** Assignment. */
	OmQuery & operator=(const OmQuery & copyme);

	/** Default constructor: makes a null query which can't be used
	 * (Convenient to have a default constructor though)
	 */
	OmQuery();

	/** Destructor. */
	~OmQuery();

	/** Returns a string representing the query.
	 * Introspection method.
	 */
	string get_description() const;

	/** Check whether the query is null. */
	bool is_null() const { return isnull; }

	/** Check whether the query is (pure) boolean. */
	bool is_bool() const { return isbool; }

	/** Set whether the query is a pure boolean.
	 *  Returns true iff the query was previously a boolean query.
	 */
	bool set_bool(bool isbool_);

	/** Get the length of the query, used by some ranking formulae.
	 *  This value is calculated automatically, but may be overridden
	 *  using set_length().
	 */
	om_termcount get_length() const { return qlen; }

	/** Set the length of the query.
	 *  This overrides the automatically calculated value, which may
	 *  be desirable in some situations.
	 *  Returns the old value of the query length.
	 */
	om_termcount set_length(om_termcount qlen_);
};

///////////////////////////////////////////////////////////////////
// OmMatchOptions class
// ====================
/// Used to specify options for running a query

class OmMatchOptions {
    friend OmEnquire;
    private:
	bool  do_collapse;
	om_keyno collapse_key;

	bool  sort_forward;
    public:
	OmMatchOptions();

	/** Set a key to collapse (remove duplicates) on.
	 *  There may only be one key in use at a time.
	 */
	void set_collapse_key(om_keyno key_);

	/// Set no key to collapse on.  This is the default.
	void set_no_collapse();

	/** Set direction of sorting.  This applies only to documents which 
	 *  have the same weight, which will only ever occur with some
	 *  weighting schemes.
	 */
	void set_sort_forward(bool forward_ = true);
};

///////////////////////////////////////////////////////////////////
// OmExpandOptions class
// =====================
/// Used to specify options for performing expand

class OmExpandOptions {
    friend OmEnquire;
    private:
	bool  allow_query_terms;
    public:
	OmExpandOptions();

	/** This sets whether terms which are already in the query will
	 *  be returned by the match.  By default, such terms will not
	 *  be returned.  A value of true will allow query terms to be
	 *  placed in the ESet.
	 */
	void use_query_terms(bool allow_query_terms_);
};

/** Base class for expand decision functions.
 */
class OmExpandDecider {
    public:
	virtual bool want_term(const om_termname & tname) const = 0;
};

///////////////////////////////////////////////////////////////////
// OmRSet class
// =============
/** A relevance set.
 *  This is the set of documents which are marked as relevant, for use
 *  in modifying the term weights, and in performing query expansion.
 */

class OmRSet {
    private:
    public:
	/** Items in the relevance set.
	 *  These can be altered directly if desired. */
	set<om_docid> items;

	/** Add a document to the relevance set. */
	void add_document(om_docid did);

	/** Remove a document from the relevance set. */
	void remove_document(om_docid did);
};

inline void
OmRSet::add_document(om_docid did)
{
    items.insert(did);
}

inline void
OmRSet::remove_document(om_docid did)
{
    set<om_docid>::iterator i = items.find(did);
    if(i != items.end()) items.erase(i);
}

///////////////////////////////////////////////////////////////////
// OmMSet class
// =============
// Representaton of a match result set

/** An item resulting from a query.
 *  This item contains the document id, and the weight calculated for
 *  the document.
 */
class OmMSetItem {
    friend class OmMatch;
    private:
	OmMSetItem(om_weight wt_, om_docid did_) : wt(wt_), did(did_) {}
    public:
	om_weight wt; /// Weight calculated
	om_docid did; /// Document id
};

/** A match set (MSet).
 *  This class represents (a portion of) the results of a query.
 */
class OmMSet {
    private:
    public:
	OmMSet() : mbound(0) {}

	/** This converts the weight supplied to a percentage score.
	 * The return value will be in the range 0 to 100, and will be 0 if
	 * and only if the item did not match the query at all.
	 */
	int convert_to_percent(om_weight wt) const;

	/// Return the percentage score for the given item.
	int convert_to_percent(const OmMSetItem & item) const;

	/// A list of items comprising the mset.
	vector<OmMSetItem> items;

	/** The index of the first item in the result to put into the mset.
	 *  This corresponds to the parameter "first" specified in
	 *  OmEnquire::get_mset().  A value of 0 corresponds to the highest
	 *  result being the first item in the mset.
	 */
	om_doccount firstitem;

	/** A lower bound on the number of documents in the database which
	 *  have a weight greater than zero.
	 *
	 *  This is currently equal to the number of such documents "seen"
	 *  by the enquiry system while searching through the database,
	 *  although it should not be relied upon as being such.  This
	 *  number is usually considerably less than the total number of
	 *  documents which match the query to any extent, due to certain
	 *  optimisations applied in calculating the M set.  The exact
	 *  value is not returned since it would be too expensive to calculate
	 *  it.
	 *
	 *  This value is returned because there is sometimes a request to
	 *  display such information.  However, our experience is that
	 *  presenting this value to users causes them to worry about the
	 *  large number of results, rather than how useful those at the top
	 *  of the result set are, and is thus undesirable.
	 */
	om_doccount mbound;

	/** The maximum possible weight in the mset.
	 *  This weight is likely not to be obtained in the set of results,
	 *  but represents an upper bound on the weight which a document
	 *  could attain for the given query.
	 */
	om_weight max_possible;

	/** The greatest weight which is attained in the mset.
	 *  This is useful when firstitem != 0.
	 */
	om_weight max_attained;
};

///////////////////////////////////////////////////////////////////
// OmESet class
// =============
// Representation a set of expand terms

/// An item in the ESet
class OmESetItem {
    friend class OmExpand;
    private:
	OmESetItem(om_weight wt_new, om_termname tname_new)
		: wt(wt_new), tname(tname_new) {}
    public:
	om_weight wt;
	om_termname tname;
};

/// Class representing an ESet
class OmESet {
    private:
    public:
	OmESet() : etotal(0) {}
	vector<OmESetItem> items;
	om_termcount etotal;
};

///////////////////////////////////////////////////////////////////
// OmData class
// ============
// Representing the document data

/** @memo Retrieve the data in a document.
 *  @doc This retrieves the arbitrary chunk of data which is associated
 *  with each document in the database: it is up to the user to define
 *  the format of this data, and to set it at indexing time.
 */
class OmData {
    public:
	/// The data.
	string value;
};

///////////////////////////////////////////////////////////////////
// OmEnquire class
// ===============
// This class provides an interface to the information retrieval
// system for the purpose of searching.

class OmEnquire {
    private:
	OmEnquireInternal *internal;
    public:
        OmEnquire();
        ~OmEnquire();

	// Add a new database to use.
	//
	// First parameter is a string describing the database type.
	// Second parameter is a vector of parameters to be used to open the
	// database: meaning and number required depends on database type.
	//
	// The database will always be opened read-only.
	void add_database(const string & type,
			  const vector<string> & params);

	// Set the query to run.
	void set_query(const OmQuery & query_);

	// Get (a portion of) the match set for the current query
	OmMSet get_mset(om_doccount first,
                        om_doccount maxitems,
			const OmRSet * omrset = 0,
			const OmMatchOptions * moptions = 0) const;

	// Get the expand set for the given rset
	OmESet get_eset(om_termcount maxitems,
			const OmRSet & omrset,
			const OmExpandOptions * eoptions = 0,
			const OmExpandDecider * decider = 0) const;

	/** Get the document data by document id.
	 */
	OmData get_doc_data(om_docid did) const;

	/** Get the document data by match set item.
	 */
	OmData get_doc_data(const OmMSetItem &mitem) const;
};

#endif /* OM_HGUARD_OMENQUIRE_H */
