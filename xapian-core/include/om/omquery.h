/* omquery.h
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

#ifndef OM_HGUARD_OMQUERY_H
#define OM_HGUARD_OMQUERY_H

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
	     */
	    OP_NEAR,

	    /** Find occurrences of a list of terms with all the terms
	     *  occurring within a specified window of positions, and all
	     *  the terms appearing in the order specified.  Each occurrence
	     *  of a term must be at a different position.
	     */
	    OP_PHRASE
	} op;

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
	 *  undefined query.
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
	 *  AND, OR, NEAR and PHRASE can take any number of subqueries
	 *  If the operator is anything other than AND, OR, NEAR, and PHRASE,
	 *  then there must be exactly two subqueries.
	 *
	 *  The iterators may be to any of OmQuery objects, OmQuery pointers,
	 *  or om_termname objects (ie, strings).
	 */
	template <class Iterator>
	OmQuery(OmQuery::op op_, Iterator qbegin, Iterator qend);

	/** Check whether the query is defined. */
	bool is_defined() const;

	/** Check whether the query is (pure) boolean. */
	bool is_bool() const;

	/** Set whether the query is a pure boolean.
	 *  Returns true iff the query was previously a boolean query.
	 */
	bool set_bool(bool isbool_);

	/** Set the window size, for a NEAR or PHRASE query.
	 */
	void set_window(om_termpos window);

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
OmQuery::OmQuery(OmQuery::op op_, Iterator qbegin, Iterator qend)
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

inline
OmQuery::OmQuery(OmQuery::op op_,
		 const om_termname & left,
		 const om_termname & right)
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

