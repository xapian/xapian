/* ombatchenquire.h
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

#ifndef OM_HGUARD_OMBATCHENQUIRE_H
#define OM_HGUARD_OMBATCHENQUIRE_H

#include "om/omtypes.h"
#include "om/omdocument.h"
#include "om/omdatabase.h"
#include "om/omerror.h"
#include "om/omenquire.h"
#include <string>
#include <vector>
#include <set>
#include <map>

///////////////////////////////////////////////////////////////////
// OmBatchEnquire class
// ====================

/** This class provides an interface to submit batches of queries.
 *
 *  FIXME: this class is currently deprecated, and not supported by omsee.
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
    public:
	class Internal;
    private:
	Internal *internal;

	// disallow copies
	OmBatchEnquire(const OmBatchEnquire &);
	void operator=(const OmBatchEnquire &);
    public:
        OmBatchEnquire(const OmDatabase &databases);
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
	    const OmSettings * moptions;

	    /** A pointer to the match decider function, or
	     *  0 for no decision functor.
	     */
	    const OmMatchDecider * mdecider;

	    /// Default constructor
	    query_desc() :
		    first(0), maxitems(10), omrset(0), moptions(0), mdecider(0)
		    {}

	    /// Constructor allowing initialisation with curly brace notation
	    query_desc(const OmQuery & query_,
		       om_doccount first_,
		       om_doccount maxitems_,
		       const OmRSet * omrset_ = 0,
		       const OmSettings * moptions_ = 0,
		       const OmMatchDecider * mdecider_ = 0)
		    : query(query_),
		      first(first_),
		      maxitems(maxitems_),
		      omrset(omrset_),
		      moptions(moptions_),
		      mdecider(mdecider_) {}
	};

	/** Type used to store a batch of queries to be performed.
	 *  This is essentially an array of query_desc objects.
	 */
	typedef std::vector<query_desc> query_batch;

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
		friend class OmBatchEnquire::Internal;
		bool isvalid;
		OmMSet result;

		/** Create a batch result.  This is used by
		 *  OmBatchEnquire::Internal.
		 */
		batch_result(const OmMSet &mset, bool isvalid_);
	    public:
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
	typedef std::vector<batch_result> mset_batch;

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

	/** Get the document info by match set iterator.
	 *  See OmEnquire::get_doc() for details
	 */
	const OmDocument get_doc(const OmMSetIterator &it) const;

	/** Get terms which match a given document, by document id.
	 *  See OmEnquire::get_matching_terms for details.
	 */
	OmTermIterator get_matching_terms(om_docid did) const;

	/** Get terms which match a given document, by match set iterator.
	 *  See OmEnquire::get_matching_terms for details.
	 */
	OmTermIterator get_matching_terms(const OmMSetIterator &it) const;

	/** Returns a string representing the batchenquire object.
	 *  Introspection method.
	 */
	std::string get_description() const;
};

#endif /* OM_HGUARD_OMBATCHENQUIRE_H */
