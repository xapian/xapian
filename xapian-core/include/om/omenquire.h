/* omenquire.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
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

#include "om/omtypes.h"
#include "om/omdocument.h"
#include "om/omdatabase.h"
#include "om/omerror.h"
#include <string>

class OmQuery;
class OmErrorHandler;

///////////////////////////////////////////////////////////////////
// OmMSet class
// =============
// Representaton of a match result set

/** An iterator pointing to items in an MSet.
 *  This is used for access to individual results of a match.
 */
class OmMSetIterator {
    public:
	friend class OmMSet;

	class Internal;
	Internal *internal; // reference counted internals

        friend bool operator==(const OmMSetIterator &a,
			       const OmMSetIterator &b);

    private:
	OmMSetIterator(Internal *internal_);

    public:
	/** Create an uninitialised iterator; this cannot be used, but is
	 *  convenient syntactically.
	 */
        OmMSetIterator();

        ~OmMSetIterator();

	/// Copying is allowed (and is cheap).
	OmMSetIterator(const OmMSetIterator &other);

        /// Assignment is allowed (and is cheap).
	void operator=(const OmMSetIterator &other);

	/// Advance the iterator
	OmMSetIterator & operator++();

	void operator++(int);

	/// Get the document ID for the current position.
	om_docid operator *() const;

	/** Get an OmDocument object for the current position.
	 *
	 *  This method returns an OmDocument object which provides the
	 *  information about the document pointed to by the MSetIterator.
	 *
	 *  If the underlying database has suitable support, using this call
	 *  (rather than asking the database for a document based on its
	 *  document ID) will enable the system to ensure that the correct
	 *  data is returned, and that the document has not been deleted
	 *  or changed since the query was performed.
	 *
	 *  @param it   The OmMSetIterator for which to retrieve the data.
	 *
	 *  @return      An OmDocument object containing the document data.
	 *
	 *  @exception OmDocNotFoundError  The document specified could not
	 *                                 be found in the database.
	 */
	OmDocument get_document() const;

	/** Get the rank of the document at the current position.
	 * 
	 *  The rank is the position that this document is at in the ordered
	 *  list of results of the query.  The document judged "most relevant"
	 *  will have rank of 0.
	 */
        om_doccount get_rank() const;

	/// Get the weight of the document at the current position
        om_weight get_weight() const;

	/** This returns the weight of the document as a percentage score
	 *  The return value will be in the range 0 to 100:  0 meaning
	 *  that the item did not match the query at all.
	 */
	om_percent get_percent() const;

	/** Returns a string describing this object.
	 *  Introspection method.
	 */
	std::string get_description() const;

	/// Allow use as an STL iterator
	//@{	
	typedef std::input_iterator_tag iterator_category;
	typedef om_docid value_type;
	typedef om_doccount_diff difference_type;
	typedef om_docid * pointer;
	typedef om_docid & reference;
	//@}
};

inline bool operator!=(const OmMSetIterator &a,
		       const OmMSetIterator &b)
{
    return !(a == b);
}

/** A match set (MSet).
 *  This class represents (a portion of) the results of a query.
 */
class OmMSet {
    public:
	class Internal;
	/// reference counted internals - do not modify externally
	Internal *internal;

    public:
	// FIXME: public for now, private would be better
	/// Constructor for internal use
	OmMSet(OmMSet::Internal * internal_);

	/// Create an empty OmMSet
	OmMSet();

	/// Destroy an OmMSet
	~OmMSet();

	/// Copying is allowed (and is cheap).
	OmMSet(const OmMSet & other);

        /// Assignment is allowed (and is cheap).
	void operator=(const OmMSet &other);

	/** Fetch the the document info for a set of items in the MSet.
	 *
	 *  This method causes the documents in the range specified by the
	 *  iterators to be fetched from the database, and cached in the
	 *  OmMSet object.  This has little effect when performing a search
	 *  across a local database, but will greatly speed up subsequent
	 *  access to the document contents when the documents are stored
	 *  in a remote database.
	 *
	 *  The iterators must be over this OmMSet: undefined behaviour
	 *  will result otherwise.
	 *
	 *  @param begin   OmMSetIterator for first item to fetch.
	 *  @param end     OmMSetIterator for item after last item to fetch.
	 */
	void fetch(const OmMSetIterator &begin,
		   const OmMSetIterator &end) const;

	/** Fetch the single item specified.
	 */
	void fetch(const OmMSetIterator &item) const;

	/** Fetch all the items in the MSet.
	 */
	void fetch() const;

	/** This converts the weight supplied to a percentage score.
	 *  The return value will be in the range 0 to 100, and will be 0 if
	 *  and only if the item did not match the query at all.
	 */
	om_percent convert_to_percent(om_weight wt) const;

	/// Return the percentage score for a particular item.
	om_percent convert_to_percent(const OmMSetIterator &it) const;

	/** Return the term frequency of the given query term.
	 *
	 *  @param tname The term to look for.
	 *
	 *  @exception OmInvalidArgumentError is thrown if the term was not
	 *             in the query.
	 */
	om_doccount get_termfreq(const om_termname &tname) const;

	/** Return the term weight of the given query term.
	 *
	 *  @param tname The term to look for.
	 *
	 *  @exception OmInvalidArgumentError is thrown if the term was not
	 *             in the query.
	 */
	om_weight get_termweight(const om_termname &tname) const;

	/** The index of the first item in the result which was put into the
	 *  mset.
	 *  This corresponds to the parameter "first" specified in
	 *  OmEnquire::get_mset().  A value of 0 corresponds to the highest
	 *  result being the first item in the mset.
	 */
	om_doccount get_firstitem() const;

	/** A lower bound on the number of documents in the database which
	 *  have a weight greater than zero.
	 *
	 *  This number is usually considerably less than the actual number
	 *  of documents which match the query.
	 */
	om_doccount get_matches_lower_bound() const;

	/** An estimate for the number of documents in the database which
	 *  have a weight greater than zero.
	 *
	 *  This value is returned because there is sometimes a request to
	 *  display such information.  However, our experience is that
	 *  presenting this value to users causes them to worry about the
	 *  large number of results, rather than how useful those at the top
	 *  of the result set are, and is thus undesirable.
	 */
	om_doccount get_matches_estimated() const;

	/** An upper bound on the number of documents in the database with
	 *  a weight greater than zero.
	 *
	 *  This number is usually considerably greater than the actual
	 *  number of documents which match the query.
	 */
	om_doccount get_matches_upper_bound() const;

	/** The maximum possible weight in the mset.
	 *  This weight is likely not to be attained in the set of results,
	 *  but represents an upper bound on the weight which a document
	 *  could attain for the given query.
	 */
	om_weight get_max_possible() const;

	/** The greatest weight which is attained by any document in the
	 *  database.  
	 *
	 *  If firstitem == 0, this is the weight of the first entry in
	 *  items.
	 *
	 *  If no documents are found by the query, this will be 0.
	 *
	 *  Note that calculation of max_attained requires calculation
	 *  of at least one result item - therefore, if no items were
	 *  requested when the query was performed (by specifying
	 *  maxitems = 0 in OmEnquire::get_mset()), this value will be 0.
	 */
	om_weight get_max_attained() const;

	om_doccount size() const;

	om_doccount max_size() const;

	bool empty() const;

	void swap(OmMSet & other);

	OmMSetIterator begin() const;

	OmMSetIterator end() const;

	OmMSetIterator back() const;
	
	/** This returns the document at position i in this MSet object.
	 *
	 *  Note that this is not the same as the document at rank i in the
	 *  query, unless the "first" parameter to OmEnquire::get_mset was
	 *  0.  Rather, it is the document at rank i + first.
	 *
	 *  In other words, the offset is into the documents represented by
	 *  this object, not into the set of documents matching the query.
	 */
	OmMSetIterator operator[](om_doccount i) const;

	/// Allow use as an STL container
	//@{	
	typedef std::input_iterator_tag iterator_category;
	typedef OmMSetIterator value_type; // FIXME: not assignable...
	typedef OmMSetIterator iterator;
	typedef OmMSetIterator const_iterator;
	typedef OmMSetIterator & reference; // Hmm
	typedef OmMSetIterator & const_reference;
	typedef OmMSetIterator * pointer; // Hmm
	typedef om_doccount_diff difference_type;
	typedef om_doccount size_type;
	//@}
	
	/** Returns a string representing the mset.
	 *  Introspection method.
	 */
	std::string get_description() const;
};

class OmESetIterator {
    public:
	friend class OmESet;
	class Internal;
	Internal *internal; // reference counted internals

        friend bool operator==(const OmESetIterator &a,
			       const OmESetIterator &b);

    private:

	OmESetIterator(Internal *internal_);

    public:
	/** Create an uninitialised iterator; this cannot be used, but is
	 *  convenient syntactically.
	 */
        OmESetIterator();

	/// Destructor
        ~OmESetIterator();

	/// Copying is allowed (and is cheap).
	OmESetIterator(const OmESetIterator &other);

        /// Assignment is allowed (and is cheap).
	void operator=(const OmESetIterator &other);

	OmESetIterator & operator++();

	void operator++(int);

	/// Get the term for the current position
	const om_termname & operator *() const;

	/// Get the weight of the term at the current position
        om_weight get_weight() const;

	/** Returns a string describing this object.
	 *  Introspection method.
	 */
	std::string get_description() const;

	/// Allow use as an STL iterator
	//@{	
	typedef std::input_iterator_tag iterator_category;
	typedef om_termname value_type;
	typedef om_termcount_diff difference_type;
	typedef om_termname * pointer;
	typedef om_termname & reference;
	//@}
};

inline bool
operator!=(const OmESetIterator &a, const OmESetIterator &b)
{
    return !(a == b);
}

/** Class representing an ordered set of expand terms (an ESet).
 *  This set represents the results of an expand operation, which is
 *  performed by OmEnquire::get_eset().
 */
class OmESet {
    public:
	class Internal;
	Internal *internal; // reference counted internals

	OmESet();

	~OmESet();

	/// Copying is allowed (and is cheap).
	OmESet(const OmESet & other);

        /// Assignment is allowed (and is cheap).
	void operator=(const OmESet &other);

	/** A lower bound on the number of terms which are in the full
	 *  set of results of the expand.  This will be greater than or
	 *  equal to size()
	 */
	om_termcount get_ebound() const;

	om_termcount size() const;

	bool empty() const;

	OmESetIterator begin() const;

	OmESetIterator end() const;

	/** Returns a string representing the eset.
	 *  Introspection method.
	 */
	std::string get_description() const;
};

///////////////////////////////////////////////////////////////////
// OmRSet class
// =============
/** A relevance set.
 *  This is the set of documents which are marked as relevant, for use
 *  in modifying the term weights, and in performing query expansion.
 */

class OmRSet {
    public:
	/// Class holding details of OmRSet
	class Internal;
	/// reference counted internals - do not modify externally
	Internal *internal;

	/// Copy constructor
	OmRSet(const OmRSet &rset);

	/// Assignment operator
	void operator=(const OmRSet &rset);

	/// Default constructor
	OmRSet();

	/// Destructor
	~OmRSet();

	om_doccount size() const;

	bool empty() const;

	/// Add a document to the relevance set.
	void add_document(om_docid did);
	void add_document(const OmMSetIterator & i) { add_document(*i); }

	/// Remove a document from the relevance set.
	void remove_document(om_docid did);
	void remove_document(const OmMSetIterator & i) { remove_document(*i); }

	/// Is a particular document in the relevance set.
	bool contains(om_docid did) const;
	bool contains(const OmMSetIterator & i) { return contains(*i); }

	/** Returns a string representing the rset.
	 *  Introspection method.
	 */
	std::string get_description() const;
};

/** Base class for matcher decision functor.
 */
class OmMatchDecider {
    public:
	/** Decide whether we want this document to be in the mset.
	 */
	virtual int operator()(const OmDocument &doc) const = 0;

	virtual ~OmMatchDecider() {}
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
	/// Copies are not allowed.
	OmEnquire(const OmEnquire &);

	/// Assignment is not allowed.
	void operator=(const OmEnquire &);
    public:
    public:
	/// Class holding details of OmEnquire
	class Internal;
	/// reference counted internals - do not modify externally
	Internal *internal;

	/** Create an OmEnquire object.
	 *
	 *  This specification cannot be changed once the OmEnquire is
	 *  opened: you must create a new OmEnquire object to access a
	 *  different database, or set of databases.
	 *
	 *  @param database Specification of the database or databases to
	 *         use.
	 *  @param errorhandler_  A pointer to the error handler to use.
	 *         Ownership of the object pointed to is not assumed by the
	 *         OmEnquire object - the user should delete the
	 *         OmErrorHandler object after the OmEnquire object is
	 *         deleted.  To use no error handler, this parameter
	 *         should be 0.
	 */
        OmEnquire(const OmDatabase &databases,
		  OmErrorHandler * errorhandler_ = 0);

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

	/** Get the query which has been set.
	 *  This is only valid after set_query() has been called.
	 *
	 *  @exception OmInvalidArgumentError will be thrown if query has
	 *             not yet been set.
	 */
	const OmQuery & get_query();

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
			const OmSettings * moptions = 0,
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
			const OmSettings * eoptions = 0,
			const OmExpandDecider * edecider = 0) const;

	/** Get terms which match a given document, by document id.
	 *
	 *  This method returns the terms in the current query which match
	 *  the given document.
	 *
	 *  It is possible for the document to have been removed from the
	 *  database between the time it is returned in an mset, and the
	 *  time that this call is made.  If possible, you should specify
	 *  an OmMSetIterator instead of a om_docid, since this will enable
	 *  database backends with suitable support to prevent this
	 *  occurring.
	 *
	 *  Note that a query does not need to have been run in order to
	 *  make this call.
	 *
	 *  @param did     The document id for which to retrieve the matching
	 *                 terms.
	 *
	 *  @return        An iterator returning the terms which match the
	 *                 document.  The terms will be returned (as far as this
	 *                 makes any sense) in the same order as the terms
	 *                 in the query.  Terms will not occur more than once,
	 *                 even if they do in the query.
	 *
	 *  @exception OmInvalidArgumentError  See class documentation.
	 *  @exception OmOpeningError          See class documentation.
	 *  @exception OmDocNotFoundError      The document specified could not
	 *                                     be found in the database.
	 */
	OmTermIterator get_matching_terms_begin(om_docid did) const;
	OmTermIterator get_matching_terms_end(om_docid did) const;

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
	 *  @param it   The iterator for which to retrieve the matching terms.
	 *
	 *  @return        An iterator returning the terms which match the
	 *                 document.  The terms will be returned (as far as this
	 *                 makes any sense) in the same order as the terms
	 *                 in the query.  Terms will not occur more than once,
	 *                 even if they do in the query.
	 *
	 *  @exception OmInvalidArgumentError  See class documentation.
	 *  @exception OmOpeningError          See class documentation.
	 *  @exception OmDocNotFoundError      The document specified could not
	 *                                     be found in the database.
	 */
	OmTermIterator get_matching_terms_begin(const OmMSetIterator &it) const;
	OmTermIterator get_matching_terms_end(const OmMSetIterator &it) const;

	/** Register an OmMatchDecider.
	 */
	void register_match_decider(const std::string &name,
				    const OmMatchDecider *mdecider = NULL);

	/** Returns a string representing the enquire object.
	 *  Introspection method.
	 */
	std::string get_description() const;
};

#endif /* OM_HGUARD_OMENQUIRE_H */
