/* omenquire.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#include <om/omtypes.h>
#include <om/omdocument.h>
#include <om/omdatabase.h>
#include <om/omerror.h>
#include <string>
#include <vector>
#include <set>

class OmEnquireInternal; // Internal state of enquire
class OmEnquire;         // Declare Enquire class
class OmMSetCmp;         // Declare mset item comparison class

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
    OM_MOP_FILTER,

    /// As AND, but also check terms are close using positional information
    OM_MOP_NEAR
};

/// Internals of query class
class OmQueryInternal;

/** Class representing a query.
 *  Queries are represented as a heirarchy of classes.
 */
class OmQuery {
    private:
	friend class OmEnquireInternal;
    	OmQueryInternal *internal;
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
	bool is_defined() const;

	/** Check whether the query is (pure) boolean. */
	bool is_bool() const;

	/** Set whether the query is a pure boolean.
	 *  Returns true iff the query was previously a boolean query.
	 */
	bool set_bool(bool isbool_);

	/** Get the length of the query, used by some ranking formulae.
	 *  This value is calculated automatically, but may be overridden
	 *  using set_length().
	 */
	om_termcount get_length() const;

	/** Set the length of the query.
	 *  This overrides the automatically calculated value, which may
	 *  be desirable in some situations.
	 *  Returns the old value of the query length.
	 */
	om_termcount set_length(om_termcount qlen_);

	/** Return an om_termname_list containing all the terms in the query,
	 *  in order of termpos.  If multiple terms have the same term
	 *  position, their order is unspecified.  Duplicates (same term and
	 *  termpos) will be removed.
	 */
	om_termname_list get_terms() const;
};

///////////////////////////////////////////////////////////////////
// OmMatchOptions class
// ====================

/** Class used to specify options for running a query.
 *  FIXME: make this into a struct?
 */
class OmMatchOptions {
    friend OmEnquireInternal;
    public:
        /** If true, duplicates will be removed based on a key.
	 *  This defaults to false.
	 */
	bool  do_collapse;

	/** The key number to collapse upon, if do_collapse is true.
	 */
	om_keyno collapse_key;

	/** If true, documents with the same weight will be
	 *  returned in ascending document order; if false, they will
	 *  be returned in descending order.
	 */
	bool  sort_forward;

	/** Minimum percentage score for returned documents.
	 *  If a document has a lower percentage score than this, it
	 *  will not appear in the results.
	 */
	int percent_cutoff;

	/** Maximum number of terms which will be used if the query
	 *  contains a large number of terms which are ORed together.
	 *
	 *  See set_max_or_terms() for more details.
	 */
	om_termcount max_or_terms;
    

	/** Create a match options object.
	 */
	OmMatchOptions();

	/** Set a key to collapse (remove duplicates) on.
	 *  There may only be one key in use at a time.
	 *  Each key value will appear only once in the result set.
	 */
	void set_collapse_key(om_keyno key_);

	/** Set no key to collapse on.
	 *  This is the default.
	 */
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

	/** Set the maximum number of terms which will be used if the query
	 *  contains a large number of terms which are ORed together.
	 *
	 *  The terms will be sorted according to termweight, and only
	 *  the top terms will be used.  Parts of the query which
	 *  do not involve terms ORed together will be unaffected by this
	 *  option.
	 *
	 *  This enables a query to be set which represents a document,
	 *  and only the elite set of terms which best distinguish that
	 *  document to be used to find similar documents, resulting in
	 *  a performance improvement.
	 *
	 *  If this is set to the default of zero, all terms will be used.
	 */
	void set_max_or_terms(om_termcount max_);

	/** Get appropriate comparator object.
	 *  This returns an object which can be used to compare two
	 *  OmMSetItems and decide which comes first: this is used by
	 *  the matcher to sort the results of a search.
	 */
	OmMSetCmp get_sort_comparator() const;
};

/** Base class for matcher decision functor.
 */
class OmMatchDecider {
    public:
	/** Decide whether we want this document to be in the mset.
	 *
	 *  Note: The parameters of this method are extremely likely to change
	 *  in the near future.
	 */
	virtual int operator()(const OmDocument *doc) const = 0;

	virtual ~OmMatchDecider() {}
};

///////////////////////////////////////////////////////////////////
// OmExpandOptions class
// =====================

/// A class holding options to be used when performing an expand operation.
class OmExpandOptions {
    friend OmEnquireInternal;
    private:
        /// See use_query_terms()
	bool use_query_terms;

	/// See use_exact_termfreq()
	bool use_exact_termfreq;
    public:
	/** Create an OmExpandOptions object.  This object is passed to
	 *  the OmEnquire::get_eset() method.
	 */
	OmExpandOptions();

	/** This sets whether terms which are already in the query will
	 *  be returned by the match.  By default, such terms will not
	 *  be returned.  A value of true will allow query terms to be
	 *  placed in the ESet.
	 *
	 *  @param use_query_terms_     The value to use for the option.
	 *                              The default is false.
	 */
	void set_use_query_terms(bool use_query_terms_);

	/** This sets whether term frequencies are to be calculated
	 *  exactly, or whether it is okay to use an approximation.
	 *  This approximation only comes into effect when multiple
	 *  databases are being used, and in many cases will serve to
	 *  improve efficiency greatly.  By default, exact term frequencies
	 *  will not be calculated and the approximation will be used.
	 *
	 *  @param use_exact_termfreq_  The value to use for the option.
	 *                              The default is false.
	 */
	void set_use_exact_termfreq(bool use_exact_termfreq_);
};

/** Base class for expand decision functor.
 */
class OmExpandDecider {
    public:
	/** Decide whether we want this term to be in the expand set.
	 */
	virtual int operator()(const om_termname & tname) const = 0;

	virtual ~OmExpandDecider() {};
};

/** One useful expand decision functor, which provides a way of
 *  filtering out a fixed list of terms from the expand set.
 */
class OmExpandDeciderFilterTerms : public OmExpandDecider {
    public:
        /** Constructor, which takes a list of terms which
	 *  will be filtered out.
	 */
        OmExpandDeciderFilterTerms(const om_termname_list &terms);

        virtual int operator()(const om_termname &tname) const;
    private:
        set<om_termname> tset;
};

/** An expand decision functor which can be used to join two
 *  functors with an AND operation.
 */
class OmExpandDeciderAnd : public OmExpandDecider {
    public:
    	/** Constructor, which takes as arguments the two
	 *  decision functors to AND together.
	 *  OmExpandDeciderAnd will not delete its sub-
	 *  functors.
	 */
	OmExpandDeciderAnd(const OmExpandDecider *left_,
	                   const OmExpandDecider *right_);
	
	virtual int operator()(const om_termname &tname) const;

    private:
        const OmExpandDecider *left;
	const OmExpandDecider *right;
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
    private:
    public:
	OmMSetItem(om_weight wt_, om_docid did_) : wt(wt_), did(did_) {}

	OmMSetItem(om_weight wt_, om_docid did_, OmKey key_)
		: wt(wt_), did(did_), collapse_key(key_) {}

	/** Weight calculated. */
	om_weight wt;

	/** Document id. */
	om_docid did;

	/** Key which was used to collapse upon.
	 *
	 *  If the collapse option is not being used, this will always
	 *  have a null value.
	 *
	 *  If a key of collapsing upon is specified, this will contain
	 *  the key for this particular item.  If the key is not present
	 *  for this item, the value will be a null string.  Only one
	 *  instance of each key value (apart from the null string) will
	 *  be present in the items in the returned OmMSet.
	 *
	 *  See OmMatchOptions::set_collapse_key() for more information
	 *  about setting a key to collapse upon.
	 */
	OmKey collapse_key;
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

	/** The index of the first item in the result which was put into the
	 *  mset.
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
	/// Internals, where most of the work is performed.
	OmEnquireInternal *internal;

	/// Copies are not allowed.
	OmEnquire(const OmEnquire &);

	/// Assignment is not allowed.
	void operator=(const OmEnquire &);
    public:
	/** Create an OmEnquire object.
	 *
	 *  This specification cannot be changed once the OmEnquire is
	 *  opened: you must create a new OmEnquire object to access a
	 *  different database, or set of databases.
	 *
	 *  @param database Specification of the database or databases to
	 *         use.
	 */
        OmEnquire(const OmDatabaseGroup &databases);

	/** Close the OmEnquire object.
	 *
	 *  This frees all resources associated with the OmEnquire object,
	 *  such as handles on the databases used.  As a result, any object
	 *  which refers to these databases, such as an OmDocument, will
	 *  become invalid after the destruction of the object, and must
	 *  not be used subsequently.
	 */
	~OmEnquire();

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


	/** Get the document info by document id.
	 *
	 *  This method returns an OmDocument object
	 *  which provides the information about a document.
	 *
	 *  It is possible for the document to have been removed from the
	 *  database between the time it is returned in an mset, and the
	 *  time that this call is made.  If possible, you should specify
	 *  an OmMSetItem instead of an om_docid, since this will enable
	 *  database backends with suitable support to prevent this
	 *  occurring.
	 *
	 *  Note that a query does not need to have been run in order to
	 *  make this call.
	 *
	 *  @param did   The document id for which to retrieve the data.
	 *
	 *  @return      An OmDocument object containing the document data
	 *
	 *  @exception OmInvalidArgumentError  See class documentation.
	 *  @exception OmOpeningError          See class documentation.
	 *  @exception OmDocNotFoundError      The document specified could not
	 *                                     be found in the database.
	 */
	const OmDocument get_doc(om_docid did) const;

	/** Get the document info by match set item.
	 *
	 *  This method returns an OmDocument object
	 *  which provides the information about a document.
	 *
	 *  If the underlying database has suitable support, using this call
	 *  (rather than passing an om_docid) will enable the system to
	 *  ensure that the correct data is returned, and that the document
	 *  has not been deleted or changed since the query was performed.
	 *
	 *  @param mitem   The item for which to retrieve the data.
	 *
	 *  @return      An OmDocument object containing the
	 *  document data.
	 *
	 *  @exception OmInvalidArgumentError  See class documentation.
	 *  @exception OmOpeningError          See class documentation.
	 *  @exception OmDocNotFoundError  The document specified could not
	 *  be found in the database.
	 */
	const OmDocument get_doc(const OmMSetItem &mitem) const;


	/** Get terms which match a given document, by document id.
	 * 
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

	/** Get terms which match a given document, by match set item.
	 *
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

///////////////////////////////////////////////////////////////////
// OmBatchEnquire class
// ====================

/** This class provides an interface to submit batches of queries.
 *
 *  Using this class will be no more expensive than simply using an
 *  OmEnquire object multiple times, and this is how it is currently
 *  implemented.  In future it may be reimplemented such that significant
 *  performance advantages result from running multiple queries using this
 *  class rather than running them as individual queries using OmEnquire.
 *
 *  If you are producing a system which will run batches of queries
 *  together (such as a nightly alerting system), we recommend use
 *  of this class.
 *
 *  @exception OmInvalidArgumentError will be thrown if an invalid
 *  argument is supplied, for example, an unknown database type.
 *
 *  @exception OmOpeningError will be thrown if the database cannot
 *  be opened (for example, a required file cannot be found).
 *  
 */
class OmBatchEnquire {
    private:
	class Internal;
	Internal *internal;

	// disallow copies
	OmBatchEnquire(const OmBatchEnquire &);
	void operator=(const OmBatchEnquire &);
    public:
        OmBatchEnquire(const OmDatabaseGroup &databases);
        ~OmBatchEnquire();

	/** This class stores a set of queries to be performed as a batch.
	 */
	struct query_desc {
	    /// The query to be executed
	    OmQuery query;

	    /// The first match to return
	    om_doccount first;

	    /// The maximum number of hits to be returned
	    om_doccount maxitems;

	    /// A pointer to the RSet for the query
	    const OmRSet * omrset;

	    /** A pointer to the match options for this query,
	     *  if any.
	     */
	    const OmMatchOptions * moptions;

	    /** A pointer to the match decider function, or
	     *  0 for no decision functor.
	     */
	    const OmMatchDecider * mdecider;
	};
	    
	/** Type used to store a batch of queries to be performed.
	 *  This is essentially an array of query_desc objects.
	 */
	typedef vector<query_desc> query_batch;

	/** Set up the queries to run.
	 *
	 *  @param queries_  A set of structures describing each query
	 *  to be performed.  See OmEnquire::set_query and
	 *  OmEnquire::get_mset for details of the meaning of each
	 *  member.
	 *
	 *  @exception OmInvalidArgumentError  See class documentation.
	 *  @exception OmOpeningError          See class documentation.
	 */
	void set_queries(const query_batch &queries_);

	/** This class stores the result of one of the queries in a
	 *  batch of queries.
	 */
	class batch_result {
	    private:
		bool isvalid;
		OmMSet result;
	    public:
		/** Create a batch result.  This is used by
		 *  OmBatchEnquire::Internal.
		 */
		batch_result(const OmMSet &mset, bool isvalid_);

		/** Return the OmMSet, if valid.
		 *  If not, then an OmInvalidResultError exception
		 *  is thrown.
		 */
		OmMSet value() const;

		/** Check to see if the result is valid without
		 *  causing an exception.  Returns true if the
		 *  result is valid.
		 */
		bool is_valid() const { return isvalid; }
	};

	/** Type used to store the results of a query batch.
	 */
	typedef vector<batch_result> mset_batch;

	/** Get (a portion of) the match sets for the current queries.
	 *
	 *  @return An collection of type OmBatchEnquire::mset_batch.
	 *          Each element is a batch_result object.  The actual OmMSet
	 *          result will be returned by batch_result::value().  If that
	 *          query failed, then value() will throw an exception.  The
	 *          validity can be checked with the is_valid() member.
	 *
	 *  @exception OmOpeningError          See class documentation.
	 */
	mset_batch get_msets() const;

	/** Get the document info by document id.
	 *  See OmEnquire::get_doc() for details.
	 */
	const OmDocument get_doc(om_docid did) const;

	/** Get the document info by match set item.
	 *  See OmEnquire::get_doc() for details
	 */
	const OmDocument get_doc(const OmMSetItem &mitem) const;


	/** Get terms which match a given document, by document id.
	 *  See OmEnquire::get_matching_terms for details.
	 */
	om_termname_list get_matching_terms(om_docid did) const;

	/** Get terms which match a given document, by match set item.
	 *  See OmEnquire::get_matching_terms for details.
	 */
	om_termname_list get_matching_terms(const OmMSetItem &mitem) const;
};
#endif /* OM_HGUARD_OMENQUIRE_H */
