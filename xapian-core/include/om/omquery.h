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
#include <vector>

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

	/** A query consisting of a single term. */
	OmQuery(const om_termname & tname_,
		om_termcount wqf_ = 1,
		om_termpos term_pos_ = 0);

	/** A query consisting of two subqueries, opp-ed together. */
	OmQuery(OmQuery::op op_, const OmQuery & left, const OmQuery & right);

	/** A set of OmQuery's, merged together with specified operator.
	 * (Takes begin and end iterators).
	 * If the operator is anything other than AND, OR, NEAR, and PHRASE,
	 * then there must be exactly two subqueries.
	 */
	OmQuery(OmQuery::op op_,
		const std::vector<OmQuery>::const_iterator qbegin,
		const std::vector<OmQuery>::const_iterator qend,
		om_termpos window = 0);

	/** As before, but uses a vector of OmQuery pointers. */
	OmQuery(OmQuery::op op_,
		const std::vector<OmQuery *>::const_iterator qbegin,
		const std::vector<OmQuery *>::const_iterator qend,
		om_termpos window = 0);

	/** As before, except subqueries are all individual terms. */
	OmQuery(OmQuery::op op_,
		const std::vector<om_termname>::const_iterator tbegin,
		const std::vector<om_termname>::const_iterator tend,
		om_termpos window = 0);

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

	/** Return an OmTermIterator returning all the terms in the query,
	 *  in order of termpos.  If multiple terms have the same term
	 *  position, their order is unspecified.  Duplicates (same term and
	 *  termpos) will be removed.
	 */
	OmTermIterator get_terms_begin() const;
	OmTermIterator get_terms_end() const;

	/** Returns a string representing the query.
	 *  Introspection method.
	 */
	std::string get_description() const;
};

#endif /* OM_HGUARD_OMQUERY_H */

