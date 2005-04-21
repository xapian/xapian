/** \file query.h
 * \brief Classes for representing a query
 */
/* ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004,2005 Olly Betts
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

#ifndef XAPIAN_INCLUDED_QUERY_H
#define XAPIAN_INCLUDED_QUERY_H

#include <string>
#include <vector>

#include <xapian/base.h>
#include <xapian/types.h>

// FIXME: sort this out so we avoid exposing Xapian::Query::Internal
// - we need to at present so that the Xapian::Query's template ctors
// compile.
class MultiMatch;
class LocalSubMatch;
class SortPosName;

namespace Xapian {

class TermIterator;

/** Class representing a query.
 * 
 *  Queries are represented as a tree of objects.
 */
class Query {
    public:
	/// Class holding details of the query
	class Internal;
	/// @internal Reference counted internals.
	Xapian::Internal::RefCntPtr<Internal> internal;

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

	    /** Select an elite set from the subqueries, and perform
	     *  a query with these combined as an OR query.
	     */
	    OP_ELITE_SET
	} op;

	/** Copy constructor. */
	Query(const Query & copyme);

	/** Assignment. */
	Query & operator=(const Query & copyme);

	/** Default constructor: makes an empty query which matches no
	 *  documents.
	 *
	 *  Also useful for defining a Query object to be assigned to later.
	 *
	 *  An exception will be thrown if an attempt is made to use an
	 *  undefined query when building up a composite query.
	 */
	Query();

	/** Destructor. */
	~Query();

	/** A query consisting of a single term. */
	Query(const std::string & tname_, Xapian::termcount wqf_ = 1,
	      Xapian::termpos pos_ = 0);

	/** A query consisting of two subqueries, opp-ed together. */
	Query(Query::op op_, const Query & left, const Query & right);

	/** A query consisting of two termnames opp-ed together. */
	Query(Query::op op_,
	      const std::string & left, const std::string & right);

	/** Combine a number of Xapian::Query-s with the specified operator.
	 *
	 *  The Xapian::Query objects are specified with begin and end
	 *  iterators.
	 * 
	 *  AND, OR, NEAR and PHRASE can take any number of subqueries.
	 *  Other operators take exactly two subqueries.
	 *
	 *  The iterators may be to Xapian::Query objects, pointers to
	 *  Xapian::Query objects, or termnames (std::string-s).
	 *
	 *  For NEAR and PHRASE, a window size can be specified in parameter.
	 *
	 *  For ELITE_SET, the elite set size can be specified in parameter.
	 */
	template <class Iterator>
	Query(Query::op op_, Iterator qbegin, Iterator qend,
	      Xapian::termcount parameter = 0);

	/** Apply the specified operator to a single Xapian::Query object. */
	Query(Query::op op_, Xapian::Query q);

	/** Get the length of the query, used by some ranking formulae.
	 *  This value is calculated automatically - if you want to override
	 *  it you can pass a different value to Enquire::set_query().
	 */
	Xapian::termcount get_length() const;

	/** Return a Xapian::TermIterator returning all the terms in the query,
	 *  in order of termpos.  If multiple terms have the same term
	 *  position, their order is unspecified.  Duplicates (same term and
	 *  termpos) will be removed.
	 */
	TermIterator get_terms_begin() const;

	/** Return a Xapian::TermIterator to the end of the list of terms in the
	 *  query.
	 */
	TermIterator get_terms_end() const;

	/** Test is the query is empty (i.e. was constructed using
	 *  the default ctor or with an empty iterator ctor).
	 */
	bool empty() const;

	/** Deprecated alias for empty() */
	bool is_empty() const { return empty(); }

	/** Returns a string representing the query.
	 *  Introspection method.
	 */
	std::string get_description() const;

    private:
	void add_subquery(const Query & subq);
	void add_subquery(const Query * subq);
	void add_subquery(const std::string & tname);
	void start_construction(Query::op op_, Xapian::termcount parameter);
	void end_construction();
	void abort_construction();
};

template <class Iterator>
Query::Query(Query::op op_, Iterator qbegin, Iterator qend, termcount parameter)
    : internal(0)
{
    try {
	start_construction(op_, parameter);

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

/// Internal class, implementing most of Xapian::Query
class Query::Internal : public Xapian::Internal::RefCntBase {
    friend class ::MultiMatch;
    friend class ::LocalSubMatch;
    friend class ::SortPosName;
    public:
        static const int OP_LEAF = -1;

	/// The container type for storing pointers to subqueries
	typedef std::vector<Internal *> subquery_list;

	/// Type storing the operation
	typedef int op_t;

    private:
	/// Operation to be performed at this node
	op_t op;

	/// Sub queries on which to perform operation
	subquery_list subqs;
	
	/** For NEAR or PHRASE, how close terms must be to match: all terms
	 *  within the operation must occur in a window of this size.
	 *
	 * For ELITE_SET, the number of terms to select from those specified.
	 */
	Xapian::termcount parameter;

	/// Term that this node represents - leaf node only
	std::string tname;

	/// Position in query of this term - leaf node only
	Xapian::termpos term_pos;

	/// Within query frequency of this term - leaf node only
	Xapian::termcount wqf;

	/** swap the contents of this with another Xapian::Query::Internal,
	 *  in a way which is guaranteed not to throw.  This is
	 *  used with the assignment operator to make it exception
	 *  safe.
	 *  It's important to adjust swap with any addition of
	 *  member variables!
	 */
	void swap(Query::Internal &other);

	/// Copy another Xapian::Query::Internal into self.
	void initialise_from_copy(const Query::Internal & copyme);

        void accumulate_terms(
	    std::vector<std::pair<std::string, Xapian::termpos> > &terms) const;

	/** Simplify the query.
	 *  For example, an AND query with only one subquery would become the
	 *  subquery itself.
	 */
	Internal * simplify_query();

	/** Preliminary checks that query is valid. (eg, has correct number of
	 *  sub queries.) Throw an exception if not.  This is initially called
	 *  on the query before any simplifications have been made.
	 */
	void prevalidate_query() const;

	/** Check query is well formed.
	 *  Throw an exception if not.
	 *  This is called at construction time, so doesn't check parameters
	 *  which must be set separately.
	 *  This performs all checks in prevalidate_query(), and some others
	 *  as well.
	 */
	void validate_query() const;

	/** Get a string describing the given query type.
	 */
	static std::string get_op_name(Xapian::Query::Internal::op_t op);

	/** Collapse the subqueryies together if appropriate.
	 */
	void collapse_subqs();

	/** Flatten a query structure, by changing, for example,
	 *  "A NEAR (B AND C)" to "(A NEAR B) AND (A NEAR C)"
	 */
	void flatten_subqs();

    public:
	/** Copy constructor. */
	Internal(const Query::Internal & copyme);

	/** Assignment. */
	void operator=(const Query::Internal & copyme);

	/** A query consisting of a single term. */
	Internal(const std::string & tname_, Xapian::termcount wqf_ = 1,
		 Xapian::termpos term_pos_ = 0);

	/** Create internals given only the operator and a parameter. */
	Internal(op_t op_, Xapian::termcount parameter);

	/** Destructor. */
	~Internal();

	static Xapian::Query::Internal * unserialise(const std::string &s);

	/** Add a subquery.
	 */
	void add_subquery(const Query::Internal & subq);

	/** Finish off the construction.
	 */
	Query::Internal * end_construction();

	/** Return a string in an easily parsed form
	 *  which contains all the information in a query.
	 */
	std::string serialise() const;

	/** Returns a string representing the query.
	 * Introspection method.
	 */
	std::string get_description() const;

	/** Get the length of the query, used by some ranking formulae.
	 *  This value is calculated automatically - if you want to override
	 *  it you can pass a different value to Enquire::set_query().
	 */
	Xapian::termcount get_length() const;

	/** Return an iterator over all the terms in the query,
	 *  in order of termpos.  If multiple terms have the same term
	 *  position, their order is unspecified.  Duplicates (same term and
	 *  termpos) will be removed.
	 */
	TermIterator get_terms() const;
};

}

#endif /* XAPIAN_INCLUDED_QUERY_H */
