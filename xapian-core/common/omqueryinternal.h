/* omqueryinternal.h
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

#ifndef OM_HGUARD_OMQUERYINTERNAL_H
#define OM_HGUARD_OMQUERYINTERNAL_H

#include <om/omtypes.h>
#include <om/omenquire.h>
#include <string>
#include <vector>
#include <set>
#include "omlocks.h"

class LocalMatch;

///////////////////////////////////////////////////////////////////
// OmQueryInternal class
// =====================

/// Internal class, implementing most of OmQuery
class OmQueryInternal {
    friend class LocalMatch;
    public:
    	OmLock mutex;
    private:
	bool isdefined;
	bool isbool;

	/// Operation to be performed at this node
	om_queryop op;

	/// The container type for storing pointers to subqueries
	typedef vector<OmQueryInternal *> subquery_list;
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

	/// Copy another OmQueryInternal into self.
	void initialise_from_copy(const OmQueryInternal & copyme);

	/** Set my vector of queries to be a memberwise copy of the
	 *  supplied vector of OmQueryInternal pointers. */
	void initialise_from_vector(
		const vector<OmQueryInternal *>::const_iterator qbegin,
		const vector<OmQueryInternal *>::const_iterator qend);

	/** swap the contents of this with another OmQueryInternal,
	 *  in a way which is guaranteed not to throw.  This is
	 *  used with the assignment operator to make it exception
	 *  safe.
	 *  It's important to adjust swap with any addition of
	 *  member variables!
	 */
	void swap(OmQueryInternal &other);

	/** Collapse lists of identical terms when possible
	 */
	void collapse_subqs();

	/** Private function used to implement get_terms() */
        void accumulate_terms(
	    vector<pair<om_termname, om_termpos> > &terms) const;
    public:
	/** A query consisting of a single term. */
	OmQueryInternal(const om_termname & tname_,
		om_termcount wqf_ = 1,
		om_termpos term_pos_ = 0);

	/** A query consisting of two subqueries, opp-ed together. */
	OmQueryInternal(om_queryop op_,
			const OmQueryInternal & left,
			const OmQueryInternal & right);

	/** A set of OmQueryInternals, merged together with specified
	 * operator.  (Takes begin and end iterators).
	 * The only operators allowed are AND and OR. */
#if 0
	OmQueryInternal(om_queryop op_,
		const vector<OmQueryInternal>::const_iterator qbegin,
		const vector<OmQueryInternal>::const_iterator qend);
#endif

	/** As before, but uses a vector of OmQueryInternal pointers. */
	OmQueryInternal(om_queryop op_,
		const vector<OmQueryInternal*>::const_iterator qbegin,
		const vector<OmQueryInternal*>::const_iterator qend);

	/** As before, except subqueries are all individual terms. */
	OmQueryInternal(om_queryop op_,
		const vector<om_termname>::const_iterator tbegin,
		const vector<om_termname>::const_iterator tend);

	/** Copy constructor. */
	OmQueryInternal(const OmQueryInternal & copyme);

	/** Assignment. */
	void operator=(const OmQueryInternal & copyme);

	/** Default constructor: makes an undefined query which can't be used
	 *  directly.  Such queries should be thought of as placeholders:
	 *  they are provided for convenience, and to help make certain
	 *  operations more natural.
	 *
	 *  An exception will be thrown if an attempt is made to run an
	 *  undefined query
	 */
	OmQueryInternal();

	/** Destructor. */
	~OmQueryInternal();

	/** Return a string in an easily parsed form
	 *  which contains all the information in a query.
	 */
	string serialise() const;

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

	/** Return an om_termname_list containing all the terms in the query,
	 *  in order of termpos.  If multiple terms have the same term
	 *  position, their order is unspecified.  Duplicates (same term and
	 *  termpos) will be removed.
	 */
	om_termname_list get_terms() const;
};

#endif // OM_HGUARD_OMQUERYINTERNAL_H
