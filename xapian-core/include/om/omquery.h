/* omquery.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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

#ifndef OM_HGUARD_OMQUERY_H
#define OM_HGUARD_OMQUERY_H

#ifndef OM_HGUARD_OMTYPES_H
#include "om/omtypes.h"
#endif
#ifndef OM_HGUARD_OMTERMLISTITERATOR_H
#include "om/omtermlistiterator.h"
#endif

///////////////////////////////////////////////////////////////////
// OmQuery class
// =============
// Representation of a query

/** Class representing a query.
 *  Queries are represented as a hierarchy of classes.
 */
class OmQuery {
    public:
	/// Class holding details of OmQuery
	class Internal;

	// FIXME: public for now, private would be better
	/// reference counted internals - do not modify externally
	Internal *internal;

    public:
	/// Enum of possible query operations
        typedef enum {
	    /// Return iff both subqueries are satisfied
	    OP_AND,

	    /// Return if either subquery is satisfied
	    OP_OR,

	    /// Return if left but not right satisfied
	    OP_AND_NOT,

	    /// Return if one query satisfied, but not both
	    OP_XOR,

	    /// Return iff left satisfied, but use weights from both
	    OP_AND_MAYBE,

	    /// As AND, but use only weights from left subquery
	    OP_FILTER,

	    /** Find occurrences of a list of terms with all the terms
	     *  occurring within a specified window of positions.
	     *  Each occurrence of a term must be at a different position,
	     *  but the order they appear in is irrelevant.
	     *
	     *  The window parameter should be specified for this operation,
	     *  but will default to the number of terms in the list.
	     */
	    OP_NEAR,

	    /** Find occurrences of a list of terms with all the terms
	     *  occurring within a specified window of positions, and all
	     *  the terms appearing in the order specified.  Each occurrence
	     *  of a term must be at a different position.
	     *
	     *  The window parameter should be specified for this operation,
	     *  but will default to the number of terms in the list.
	     */
	    OP_PHRASE,

	    /** Return only results with a weight greater than or equal to
	     *  a specified cutoff value.
	     *
	     *  The cutoff parameter should be specified for this operation,
	     *  and will default to 0 (no cutoff).
	     */
	    OP_WEIGHT_CUTOFF,

	    /** Return only results with a percentage weight greater than
	     *  a specified cutoff value.
	     *
	     *  The cutoff parameter should be specified for this operation,
	     *  and will default to 0 (ie, no cutoff).
	     */
	    OP_PERCENT_CUTOFF,

	    /** Select an elite set of terms from the subqueries, and perform
	     *  a query with all those terms combined as an OR query.
	     *  This replaces the old "match_max_or_terms" option.
	     */
	    OP_ELITE_SET
	} op;

	/** Copy constructor. */
	OmQuery(const OmQuery & copyme);

	/** Assignment. */
	OmQuery & operator=(const OmQuery & copyme);

	/** Default constructor: makes an undefined query which can't be used
	 *  directly.  Such queries should be thought of as placeholders:
	 *  they are provided merely for convenience.
	 *
	 *  An exception will be thrown if an attempt is made to run an
	 *  undefined query, or to use an undefined query when building up
	 *  a composite query.
	 */
	OmQuery();

	/** Destructor. */
	~OmQuery();

	/** A query consisting of a single term. */
	OmQuery(const om_termname & tname_,
		om_termcount wqf_ = 1,
		om_termpos term_pos_ = 0);

	/** A query consisting of two subqueries, opp-ed together. */
	OmQuery(OmQuery::op op_, const OmQuery & left, const OmQuery & right);

	/* A query consisting of two subquery pointers, opp-ed together. */
	// Don't have this because vector iterators are often implemented as
	// simple pointers, so this would override the template class and
	// produce unexpected results.  Only plausible solution we can think
	// of so far is to change to using construction methods (eg,
	// static OmQuery::create_vector(op, begin, end) and
	// static OmQuery::create_pair(op, begin, end)
	//OmQuery(OmQuery::op op_, const OmQuery * left, const OmQuery * right);

	/** A query consisting of two termnames opp-ed together. */
	OmQuery(OmQuery::op op_, const om_termname & left, const om_termname & right);

	/** A set of OmQuery's, merged together with specified operator.
	 *  (Takes begin and end iterators).
	 *  AND, OR, NEAR and PHRASE can take any number of subqueries.
	 *  WEIGHT_CUTOFF and PERCENT_CUTOFF take only one subquery.
	 *  If the operator is anything else then there must be exactly two
	 *  subqueries.
	 *
	 *  The iterators may be to any of OmQuery objects, OmQuery pointers,
	 *  or om_termname objects (ie, strings).
	 */
	template <class Iterator>
	OmQuery(OmQuery::op op_, Iterator qbegin, Iterator qend);

	/** A single OmQuery, modified by a specified operator.
	 *
	 *  The subquery may be any of: an OmQuery object, OmQuery pointer,
	 *  or om_termname.
	 */
	template <class SubQ>
	OmQuery(OmQuery::op op_, SubQ q);

	/** Check whether the query is defined. */
	bool is_defined() const;

	/** Set the window size, for a NEAR or PHRASE query.
	 */
	void set_window(om_termpos window);

	/** Set the cutoff parameter, for a WEIGHT_CUTOFF or PERCENT_CUTOFF
	 *  query.
	 */
	void set_cutoff(om_weight cutoff);

	/** Set the elite set size, for ELITE_SET queries.  */
	void set_elite_set_size(om_termcount size);

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

	/** Return an OmTermIterator returning all the terms in the query,
	 *  in order of termpos.  If multiple terms have the same term
	 *  position, their order is unspecified.  Duplicates (same term and
	 *  termpos) will be removed.
	 */
	OmTermIterator get_terms_begin() const;

	/** Return an OmTermIterator to the end of the list of terms in the
	 *  query.
	 */
	OmTermIterator get_terms_end() const;

	/** Returns a string representing the query.
	 *  Introspection method.
	 */
	std::string get_description() const;

    private:
	void add_subquery(const OmQuery & subq);
	void add_subquery(const OmQuery * subq);
	void add_subquery(const om_termname & tname);
	void start_construction(OmQuery::op op_);
	void end_construction();
	void abort_construction();
};

template <class Iterator>
OmQuery::OmQuery(OmQuery::op op_, Iterator qbegin, Iterator qend) : internal(0)
{
    try {
	start_construction(op_);

	/* Add all the elements */
	while (qbegin != qend) {
	    add_subquery(*qbegin);
	    ++qbegin;
	}

	end_construction();
    } catch (...) {
	abort_construction();
	throw;
    }
}

template <class SubQ>
OmQuery::OmQuery(OmQuery::op op_, SubQ q) : internal(0)
{
    try {
	start_construction(op_);
	add_subquery(q);
	end_construction();
    } catch (...) {
	abort_construction();
	throw;
    }
}

inline
OmQuery::OmQuery(OmQuery::op op_,
		 const om_termname & left,
		 const om_termname & right) : internal(0)
{
    try {
	start_construction(op_);
	add_subquery(left);
	add_subquery(right);
	end_construction();
    } catch (...) {
	abort_construction();
	throw;
    }
}

#endif /* OM_HGUARD_OMQUERY_H */

