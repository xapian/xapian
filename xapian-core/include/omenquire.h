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

#ifndef _omenquire_h_
#define _omenquire_h_

#include "omtypes.h"
#include <string>
#include <vector>
#include <set>

class OMEnquireInternal; // Internal state of enquire
class OMEnquire;         // Declare Enquire class
class OMMatch;           // Class which performs queries

///////////////////////////////////////////////////////////////////
// OMQuery class
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

/// Class representing a query
class OMQuery {
    friend class OMMatch;
    private:
	bool isnull;
	bool isbool;
	vector<OMQuery *> subqs;
	om_termname tname;
	om_queryop op;

	void initialise_from_copy(const OMQuery & copyme);
	void initialise_from_vector(const vector<OMQuery>::const_iterator qbegin,
				    const vector<OMQuery>::const_iterator qend);
	void initialise_from_vector(const vector<OMQuery *>::const_iterator qbegin,
				    const vector<OMQuery *>::const_iterator qend);
    public:
	/// A query consisting of a single term
	OMQuery(const om_termname & tname_);

	/// A query consisting of two subqueries, opp-ed together
	OMQuery(om_queryop op_, const OMQuery & left, const OMQuery & right);

	/** A set of OMQuery's, merged together with specified operator.
	 * (Takes begin and end iterators).
	 * The only operators allowed are AND and OR. */
	OMQuery(om_queryop op_,
		const vector<OMQuery>::const_iterator qbegin,
		const vector<OMQuery>::const_iterator qend);

	/// As before, but uses a vector of OMQuery pointers
	OMQuery(om_queryop op_,
		const vector<OMQuery *>::const_iterator qbegin,
		const vector<OMQuery *>::const_iterator qend);

	/// As before, except subqueries are all individual terms.
	OMQuery(om_queryop op_,
		const vector<om_termname>::const_iterator tbegin,
		const vector<om_termname>::const_iterator tend);

	/// Copy constructor
	OMQuery(const OMQuery & copyme);

	/// Assignment
	OMQuery & operator=(const OMQuery & copyme);

	/** Default constructor: makes a null query which can't be used
	 * (Convenient to have a default constructor though) */
	OMQuery();

	/// Destructor
	~OMQuery();

	/** Introspection method
	 * Returns a string representing the query. */
	string get_description() const;

	/// Check whether the query is null
	bool is_null() const { return isnull; };

	/// Check whether the query is (pure) boolean
	bool is_bool() const { return isbool; };
};

///////////////////////////////////////////////////////////////////
// OMMatchOptions class
// ====================
/// Used to specify options for running a query

class OMMatchOptions {
    friend OMEnquire;
    private:
	bool  do_collapse;
	om_keyno collapse_key;

	bool  sort_forward;
    public:
	OMMatchOptions();

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
// OMExpandOptions class
// =====================
/// Used to specify options for performing expand

class OMExpandOptions {
    friend OMEnquire;
    private:
	bool  allow_query_terms;
    public:
	OMExpandOptions();

	/** This sets whether terms which are already in the query will
	 *  be returned by the match.  By default, such terms will not
	 *  be returned.  A value of true will allow query terms to be
	 *  placed in the ESet.
	 */
	void use_query_terms(bool allow_query_terms_);
};

/** Base class for expand decision functions.
 */
class OMExpandDecider {
    public:
	virtual bool want_term(const om_termname & tname) const = 0;
};

///////////////////////////////////////////////////////////////////
// OMRSet class
// =============
/** A relevance set.
 *  This is the set of documents which are marked as relevant, for use
 *  in modifying the term weights, and in performing query expansion.
 */

class OMRSet {
    private:
    public:
	/** Items in the relevance set.  These can be altered directly if
	 * desired.  */
	set<om_docid> items;
	void add_document(om_docid did);
	void remove_document(om_docid did);
};

/// Add a document to the relevance set.
inline void
OMRSet::add_document(om_docid did)
{
    items.insert(did);
}

/// Remove a document from the relevance set.
inline void
OMRSet::remove_document(om_docid did)
{
    set<om_docid>::iterator i = items.find(did);
    if(i != items.end()) items.erase(i);
}

///////////////////////////////////////////////////////////////////
// OMMSet class
// =============
// Representaton of a match result set

/// An item resulting from a query
class OMMSetItem {
    friend class OMMatch;
    private:
	OMMSetItem(om_weight wt_, om_docid did_) : wt(wt_), did(did_) {}
    public:
	om_weight wt; /// Weight calculated
	om_docid did; /// Document id
};

/** A match set (MSet).
 *  This class represents (a portion of) the results of a query.
 */
class OMMSet {
    private:
    public:
	OMMSet() : mbound(0) {}

	/** This converts the weight supplied to a percentage score.
	 * The return value will be in the range 0 to 100, and will be 0 if
	 * and only if the item did not match the query at all.
	 */
	int convert_to_percent(om_weight wt) const;

	/// Return the percentage score for the given item.
	int convert_to_percent(const OMMSetItem & item) const;

	/// A list of items comprising the mset.
	vector<OMMSetItem> items;

	/** The index of the first item in the result to put into the mset.
	 *  This corresponds to the parameter "first" specified in
	 *  OMEnquire::get_mset().  A value of 0 corresponds to the highest
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
// OMESet class
// =============
// Representation a set of expand terms

/// An item in the ESet
class OMESetItem {
    friend class OMExpand;
    private:
	OMESetItem(om_weight wt_new, om_termname tname_new)
		: wt(wt_new), tname(tname_new) {}
    public:
	om_weight wt;
	om_termname tname;
};

/// Class representing an ESet
class OMESet {
    private:
    public:
	OMESet() : etotal(0) {}
	vector<OMESetItem> items;
	om_termcount etotal;
};

///////////////////////////////////////////////////////////////////
// OMEnquire class
// ===============
// This class provides an interface to the information retrieval
// system for the purpose of searching.

class OMEnquire {
    private:
	OMEnquireInternal *internal;
    public:
        OMEnquire();
        ~OMEnquire();

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
	void set_query(const OMQuery & query_);

	// Get (a portion of) the match set for the current query
	void get_mset(OMMSet & mset,
                      om_doccount first,
                      om_doccount maxitems,
		      const OMRSet * omrset = 0,
	              const OMMatchOptions * moptions = 0) const;

	// Get the expand set for the given rset
	void get_eset(OMESet & eset,
                      om_termcount maxitems,
                      const OMRSet & omrset,
                      const OMExpandOptions * eoptions = 0,
		      const OMExpandDecider * decider = 0) const;
};

#endif /* _omenquire_h_ */
