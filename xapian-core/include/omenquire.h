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
    /// For internal use - must never be specified as parameter
    OM_MOP_LEAF,

    /// Return iff both subqueries are satisfied
    OM_MOP_AND,

    /// Return if either subquery is satisfied
    OM_MOP_OR,

    /// Return if left but not right satisfied
    OM_MOP_AND_NOT,

    /// Return if one query satisfied, but not both
    OM_MOP_XOR,

    /// Return iff left satisfied, but use weights from both
    OM_MOP_AND_MAYBE,

    /// As AND, but use only weights from left subquery
    OM_MOP_FILTER
};


/** Class representing a query.
 *  Queries are represented as a heirarchy of classes.
 */
class OmQuery {
    friend class OmMatch;
    friend class OmEnquire;
    private:
	bool isdefined;
	bool isbool;

	/// Operation to be performed at this node
	om_queryop op;

	/// The container type for storing pointers to subqueries
	typedef vector<OmQuery *> subquery_list;
	/// Sub queries on which to perform operation
	subquery_list subqs;

	/// Length of query
	om_termcount qlen;

	// Fields used only for leaf nodes.

	/// Term that this leaf represents
	om_termname tname;

	/// Position in query of this term
	om_termpos term_pos;

	/// Within query frequency of this term
	om_termcount wqf;

	/// Copy another OmQuery into self.
	void initialise_from_copy(const OmQuery & copyme);

	/** Set my vector of queries to be a memberwise copy of the
	 *  supplied vector of OmQuery objects. */
	void initialise_from_vector(const vector<OmQuery>::const_iterator qbegin,
				    const vector<OmQuery>::const_iterator qend);

	/** Set my vector of queries to be a memberwise copy of the
	 *  supplied vector of OmQuery pointers. */
	void initialise_from_vector(const vector<OmQuery *>::const_iterator qbegin,
				    const vector<OmQuery *>::const_iterator qend);

	/** Private function implementing get_terms() */
        om_termname_list internal_get_terms() const;

	/** Return an om_termname_list containing all the terms in the query,
	 *  in left to right order.
	 */
	om_termname_list get_terms() const;
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

	/** Default constructor: makes an undefined query which can't be used
	 *  directly.  Such queries should be thought of as placeholders:
	 *  they are provided for convenience, and to help make certain
	 *  operations more natural.
	 *
	 *  An exception will be thrown if an attempt is made to run an
	 *  undefined query
	 */
	OmQuery();

	/** Destructor. */
	~OmQuery();

	/** Returns a string representing the query.
	 * Introspection method.
	 */
	string get_description() const;

	/** Check whether the query is defined. */
	bool is_defined() const { return isdefined; }

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

	int percent_cutoff;
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

	/** Set a percentage cutoff for the match.  Only documents
	 *  with a percent weight of at least this percentage will
	 *  be returned in the mset.  (If the intention is to return
	 *  only matches which contain all the terms in the query,
	 *  then consider using OM_MOP_AND instead of OM_MOP_OR in
	 *  the query.)  The percentage must be between 0 and 100, or
	 *  an OmInvalidArgumentError will be thrown.
	 */
	void set_percentage_cutoff(int percent_);
};

/** Base class for matcher decision functor.
 *
 *  Note:  Matcher decision functors are not yet implemented!
 */
// FIXME - implement
class OmMatchDecider {
    public:
	/** Decide whether we want this document to be in the mset.
	 *
	 *  Note: The parameters of this method are extremely likely to change
	 *  in the near future.
	 */
	virtual int operator()(om_docid did) const = 0;
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

/** Base class for expand decision functor.
 */
class OmExpandDecider {
    public:
	/** Decide whether we want this term to be in the expand set.
	 */
	virtual int operator()(const om_termname & tname) const = 0;
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

	/// A list of items comprising the (selected part of the) mset.
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

/** An item in the ESet.
 *  This item contains the termname, and the weight calculated for
 *  the document.
 */
class OmESetItem {
    friend class OmExpand;
    private:
	OmESetItem(om_weight wt_new, om_termname tname_new)
		: wt(wt_new), tname(tname_new) {}
    public:
	/// Weight calculated.
	om_weight wt;
	/// Term suggested.
	om_termname tname;
};

/** Class representing an ESet.
 *  This set represents the results of an expand operation, which can be
 *  performed by OmEnquire::get_eset().
 */
class OmESet {
    private:
    public:
	OmESet() : ebound(0) {}

	/// A list of items comprising the (selected part of the) eset.
	vector<OmESetItem> items;

	/** A lower bound on the number of terms which are in the full
	 *  set of results of the expand.  This will be greater than or
	 *  equal to items.size()
	 */
	om_termcount ebound;
};

///////////////////////////////////////////////////////////////////
// OmData class
// ============
// Representing the document data

/** @memo The data in a document.
 *  @doc This contains the arbitrary chunk of data which is associated
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

/** This class provides an interface to the information retrieval
 *  system for the purpose of searching.
 *
 *  Databases are usually opened lazily, so exceptions may not be
 *  thrown where you would expect them to be.  You should catch
 *  OmError exceptions when calling any method in OmEnquire.
 *
 *  @exception OmInvalidArgumentError will be thrown if an invalid
 *  argument is supplied, for example, an unknown database type.
 *
 *  @exception OmOpeningError will be thrown if the database cannot
 *  be opened (for example, a required file cannot be found).
 */
class OmEnquire {
    private:
	OmEnquireInternal *internal;
    public:
        OmEnquire();
        ~OmEnquire();

	/** Add a new database to use.
	 *
	 *  The database may not be opened by this call: the system may wait
	 *  until a get_mset (or similar call).
	 *  Thus, failure to open the database may not result in an
	 *  OmOpeningError exception being thrown until the database is used.
	 *
	 *  The database will always be opened read-only.
	 *
	 *  @param type    a string describing the database type.
	 *  @param params  a vector of parameters to be used to open the
	 *  database: meaning and number required depends on database type.
	 *
	 *  @exception OmInvalidArgumentError  See class documentation.
	 *  @exception OmOpeningError          See class documentation.
	 */
	void add_database(const string & type,
			  const vector<string> & params);

	/** Set the query to run.
	 *
	 *  @param query_  the new query to run.
	 *
	 *  @exception OmInvalidArgumentError  See class documentation.
	 *  @exception OmOpeningError          See class documentation.
	 */
	void set_query(const OmQuery & query_);

	/** Get (a portion of) the match set for the current query.
	 *
	 *  @param first     the first item in the result set to return.
	 *                   A value of zero corresponds to the first item
	 *                   returned being that with the highest score.
	 *                   A value of 10 corresponds to the first 10 items
	 *                   being ignored, and the returned items starting
	 *                   at the eleventh.
	 *  @param maxitems  the maximum number of items to return.
	 *  @param omrset    the relevance set to use when performing the query.
	 *  @param moptions  options to use when performing the match.
	 *  @param mdecider  a decision functor to use to decide whether a
	 *                   given document should be put in the MSet
	 *
	 *
	 *  @return          An OmMSet object containing the results of the
	 *                   query.
	 *
	 *  @exception OmInvalidArgumentError  See class documentation.
	 *  @exception OmOpeningError          See class documentation.
	 */
	OmMSet get_mset(om_doccount first,
                        om_doccount maxitems,
			const OmRSet * omrset = 0,
			const OmMatchOptions * moptions = 0,
			const OmMatchDecider * mdecider = 0) const;

	/** Get the expand set for the given rset.
	 *
	 *  @param maxitems  the maximum number of items to return.
	 *  @param omrset    the relevance set to use when performing
	 *                   the expand operation.
	 *  @param eoptions  options to use when performing the expand.
	 *  @param edecider  a decision functor to use to decide whether a
	 *                   given term should be put in the ESet
	 *
	 *  @return          An OmESet object containing the results of the
	 *                   expand.
	 *
	 *  @exception OmInvalidArgumentError  See class documentation.
	 *  @exception OmOpeningError          See class documentation.
	 */
	OmESet get_eset(om_termcount maxitems,
			const OmRSet & omrset,
			const OmExpandOptions * eoptions = 0,
			const OmExpandDecider * edecider = 0) const;


	/** @memo Get the document data by document id.
	 *
	 *  @doc This method returns the data associated with the given
	 *  document.
	 *
	 *  It is possible for the document to have been removed from the
	 *  database between the time it is returned in an mset, and the
	 *  time that this call is made.  If possible, you should specify
	 *  an OmMSetItem instead of a om_docid, since this will enable
	 *  database backends with suitable support to prevent this
	 *  occurring.
	 *
	 *  Note that a query does not need to have been run in order to
	 *  make this call.
	 *
	 *  @param did   The document id for which to retrieve the data.
	 *
	 *  @return      An OmData object containing the document data.
	 *
	 *  @exception OmInvalidArgumentError  See class documentation.
	 *  @exception OmOpeningError          See class documentation.
	 *  @exception OmDocNotFoundError      The document specified could not
	 *                                     be found in the database.
	 *  @exception
	 */
	OmData get_doc_data(om_docid did) const;

	/** @memo Get the document data by match set item.
	 *
	 *  @doc This method returns the data associated with the given
	 *  document.
	 *
	 *  If the underlying database has suitable support, using this call
	 *  (rather than passing an om_docid) will enable the system to
	 *  ensure that the correct data is returned, and that the document
	 *  has not been deleted or changed since the query was performed.
	 *
	 *  @param mitem   The item for which to retrieve the data.
	 *
	 *  @return      An OmData object containing the document data.
	 *
	 *  @exception OmInvalidArgumentError  See class documentation.
	 *  @exception OmOpeningError          See class documentation.
	 *  @exception OmDocNotFoundError  The document specified could not
	 *  be found in the database.
	 */
	OmData get_doc_data(const OmMSetItem &mitem) const;


	/** @memo Get terms which match a given document, by document id.
	 * 
	 *  @doc
	 *  This method returns the terms in the current query which match
	 *  the given document.
	 *
	 *  It is possible for the document to have been removed from the
	 *  database between the time it is returned in an mset, and the
	 *  time that this call is made.  If possible, you should specify
	 *  an OmMSetItem instead of a om_docid, since this will enable
	 *  database backends with suitable support to prevent this
	 *  occurring.
	 *
	 *  Note that a query does not need to have been run in order to
	 *  make this call.
	 *
	 *  @param did     The document id for which to retrieve the matching
	 *                 terms.
	 *
	 *  @return        A vector containing the terms which match the
	 *                 document.  This vector will be, as far as this
	 *                 makes any sense, in the same order as the terms
	 *                 in the query.  Terms will not occur more than once,
	 *                 even if they do in the query.
	 *
	 *  @exception OmInvalidArgumentError  See class documentation.
	 *  @exception OmOpeningError          See class documentation.
	 *  @exception OmDocNotFoundError      The document specified could not
	 *                                     be found in the database.
	 */
	om_termname_list get_matching_terms(om_docid did) const;

	/** @memo Get terms which match a given document, by match set item.
	 *
	 *  @doc
	 *  This method returns the terms in the current query which match
	 *  the given document.
	 *
	 *  If the underlying database has suitable support, using this call
	 *  (rather than passing an om_docid) will enable the system to
	 *  ensure that the correct data is returned, and that the document
	 *  has not been deleted or changed since the query was performed.
	 *
	 *  @param mitem   The item for which to retrieve the matching terms.
	 *
	 *  @return        A list containing the terms which match the
	 *                 document.  Terms will not occur more than once,
	 *                 even if they do in the query.
	 *
	 *  @exception OmInvalidArgumentError  See class documentation.
	 *  @exception OmOpeningError          See class documentation.
	 *  @exception OmDocNotFoundError      The document specified could not
	 *                                     be found in the database.
	 */
	om_termname_list get_matching_terms(const OmMSetItem &mitem) const;
};

#endif /* OM_HGUARD_OMENQUIRE_H */
