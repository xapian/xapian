/** \file query.h
 * \brief Classes for representing a query
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004,2005,2006,2007,2008,2009,2012 Olly Betts
 * Copyright 2006,2007,2008,2009 Lemur Consulting Ltd
 * Copyright 2008 Richard Boulton
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef XAPIAN_INCLUDED_QUERY_H
#define XAPIAN_INCLUDED_QUERY_H

#include <string>
#include <vector>

#include <xapian/base.h>
#include <xapian/types.h>
#include <xapian/termiterator.h>
#include <xapian/visibility.h>

// FIXME: sort this out so we avoid exposing Xapian::Query::Internal
// - we need to at present so that the Xapian::Query's template ctors
// compile.
class LocalSubMatch;
class MultiMatch;
class QueryOptimiser;
struct SortPosName;

namespace Xapian {

class PostingSource;
class Registry;

/** Class representing a query.
 *
 *  Queries are represented as a tree of objects.
 */
class XAPIAN_VISIBILITY_DEFAULT Query {
    public:
	/// Class holding details of the query
	class Internal;
	/// @private @internal Reference counted internals.
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
	     *
	     *  Each occurrence of a term must be at a different position,
	     *  but the order they appear in is irrelevant.
	     *
	     *  The window parameter should be specified for this operation,
	     *  but will default to the number of terms in the list.
	     */
	    OP_NEAR,

	    /** Find occurrences of a list of terms with all the terms
	     *  occurring within a specified window of positions, and all
	     *  the terms appearing in the order specified.
	     *
	     *  Each occurrence of a term must be at a different position.
	     *
	     *  The window parameter should be specified for this operation,
	     *  but will default to the number of terms in the list.
	     */
	    OP_PHRASE,

	    /** Filter by a range test on a document value. */
	    OP_VALUE_RANGE,

	    /** Scale the weight of a subquery by the specified factor.
	     *
	     *  A factor of 0 means this subquery will contribute no weight to
	     *  the query - it will act as a purely boolean subquery.
	     *
	     *  If the factor is negative, Xapian::InvalidArgumentError will
	     *  be thrown.
	     */
	    OP_SCALE_WEIGHT,

	    /** Pick the best N subqueries and combine with OP_OR.
	     *
	     *  If you want to implement a feature which finds documents
	     *  similar to a piece of text, an obvious approach is to build an
	     *  "OR" query from all the terms in the text, and run this query
	     *  against a database containing the documents.  However such a
	     *  query can contain a lots of terms and be quite slow to perform,
	     *  yet many of these terms don't contribute usefully to the
	     *  results.
	     *
	     *  The OP_ELITE_SET operator can be used instead of OP_OR in this
	     *  situation.  OP_ELITE_SET selects the most important ''N'' terms
	     *  and then acts as an OP_OR query with just these, ignoring any
	     *  other terms.  This will usually return results just as good as
	     *  the full OP_OR query, but much faster.
	     *
	     *  In general, the OP_ELITE_SET operator can be used when you have
	     *  a large OR query, but it doesn't matter if the search
	     *  completely ignores some of the less important terms in the
	     *  query.
	     *
	     *  The subqueries don't have to be terms, but if they aren't then
	     *  OP_ELITE_SET will look at the estimated frequencies of the
	     *  subqueries and so could pick a subset which don't actually
	     *  match any documents even if the full OR would match some.
	     *
	     *  You can specify a parameter to the query constructor which
	     *  control the number of terms which OP_ELITE_SET will pick.  If
	     *  not specified, this defaults to 10 (or
	     *  <code>ceil(sqrt(number_of_subqueries))</code> if there are more
	     *  than 100 subqueries, but this rather arbitrary special case
	     *  will be dropped in 1.3.0).  For example, this will pick the
	     *  best 7 terms:
	     *
	     *  <pre>
	     *  Xapian::Query query(Xapian::Query::OP_ELITE_SET, subqs.begin(), subqs.end(), 7);
	     *  </pre>
	     *
	     * If the number of subqueries is less than this threshold,
	     * OP_ELITE_SET behaves identically to OP_OR.
	     */
	    OP_ELITE_SET,

	    /** Filter by a greater-than-or-equal test on a document value. */
	    OP_VALUE_GE,

	    /** Filter by a less-than-or-equal test on a document value. */
	    OP_VALUE_LE,

	    /** Treat a set of queries as synonyms.
	     *
	     *  This returns all results which match at least one of the
	     *  queries, but weighting as if all the sub-queries are instances
	     *  of the same term: so multiple matching terms for a document
	     *  increase the wdf value used, and the term frequency is based on
	     *  the number of documents which would match an OR of all the
	     *  subqueries.
	     *
	     *  The term frequency used will usually be an approximation,
	     *  because calculating the precise combined term frequency would
	     *  be overly expensive.
	     *
	     *  Identical to OP_OR, except for the weightings returned.
	     */
	    OP_SYNONYM
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
	 *  AND, OR, XOR, ELITE_SET, SYNONYM, NEAR and PHRASE can take any
	 *  number of subqueries.  Other operators take exactly two subqueries.
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

	/** Apply the specified operator to a single Xapian::Query object, with
	 *  a double parameter.
	 */
	Query(Query::op op_, Xapian::Query q, double parameter);

	/** Construct a value range query on a document value.
	 *
	 *  A value range query matches those documents which have a value
	 *  stored in the slot given by @a slot which is in the range
	 *  specified by @a begin and @a end (in lexicographical
	 *  order), including the endpoints.
	 *
	 *  @param op_   The operator to use for the query.  Currently, must
	 *               be OP_VALUE_RANGE.
	 *  @param slot  The slot number to get the value from.
	 *  @param begin The start of the range.
	 *  @param end   The end of the range.
	 */
	Query(Query::op op_, Xapian::valueno slot,
	      const std::string &begin, const std::string &end);

	/** Construct a value comparison query on a document value.
	 *
	 *  This query matches those documents which have a value stored in the
	 *  slot given by @a slot which compares, as specified by the
	 *  operator, to @a value.
	 *
	 *  @param op_   The operator to use for the query.  Currently, must
	 *               be OP_VALUE_GE or OP_VALUE_LE.
	 *  @param slot  The slot number to get the value from.
	 *  @param value The value to compare.
	 */
	Query(Query::op op_, Xapian::valueno slot, const std::string &value);

	/** Construct an external source query.
	 *
	 *  An attempt to clone the posting source will be made immediately, so
	 *  if the posting source supports clone(), the source supplied may be
	 *  safely deallocated after this call.  If the source does not support
	 *  clone(), the caller must ensure that the posting source remains
	 *  valid until the Query is deallocated.
	 *
	 *  @param external_source The source to use in the query.
	 */
	explicit Query(Xapian::PostingSource * external_source);

	/** A query which matches all documents in the database. */
	static const Xapian::Query MatchAll;

	/** A query which matches no documents. */
	static const Xapian::Query MatchNothing;

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
	TermIterator get_terms_end() const {
	    return TermIterator();
	}

	/** Test if the query is empty (i.e. was constructed using
	 *  the default ctor or with an empty iterator ctor).
	 */
	bool empty() const;

	/** Serialise query into a string.
	 *
	 *  The query representation may change between Xapian releases:
	 *  even between minor versions.  However, it is guaranteed not to
	 *  change unless the remote database protocol has also changed between
	 *  releases.
	 */
	std::string serialise() const;

	/** Unserialise a query from a string produced by serialise().
	 *
	 *  This method will fail if the query contains any external
	 *  PostingSource leaf nodes.
	 *
	 *  @param s The string representing the serialised query.
	 */
	static Query unserialise(const std::string &s);

	/** Unserialise a query from a string produced by serialise().
	 *
	 *  The supplied registry will be used to attempt to unserialise any
	 *  external PostingSource leaf nodes.  This method will fail if the
	 *  query contains any external PostingSource leaf nodes which are not
	 *  registered in the registry.
	 *
	 *  @param s The string representing the serialised query.
	 *  @param registry Xapian::Registry to use.
	 */
	static Query unserialise(const std::string & s,
				 const Registry & registry);

	/// Return a string describing this object.
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

#ifndef SWIG // SWIG has no interest in the internal class, so hide it completely.

/// @private @internal Internal class, implementing most of Xapian::Query.
class XAPIAN_VISIBILITY_DEFAULT Query::Internal : public Xapian::Internal::RefCntBase {
    friend class ::LocalSubMatch;
    friend class ::MultiMatch;
    friend class ::QueryOptimiser;
    friend struct ::SortPosName;
    friend class Query;
    public:
        static const int OP_LEAF = -1;
        static const int OP_EXTERNAL_SOURCE = -2;

	/// The container type for storing pointers to subqueries
	typedef std::vector<Internal *> subquery_list;

	/// Type storing the operation
	typedef int op_t;

    private:
	/// Operation to be performed at this node
	Xapian::Query::Internal::op_t op;

	/// Sub queries on which to perform operation
	subquery_list subqs;

	/** For NEAR or PHRASE, how close terms must be to match: all terms
	 *  within the operation must occur in a window of this size.
	 *
	 * For ELITE_SET, the number of terms to select from those specified.
	 *
	 * For RANGE, the value number to apply the range test to.
	 *
	 * For a leaf node, this is the within query frequency of the term.
	 */
	Xapian::termcount parameter;

	/** Term that this node represents, or start of a range query.
	 *
	 *  For a leaf node, this holds the term name.  For an OP_VALUE_RANGE
	 *  query this holds the start of the range.  For an OP_VALUE_GE or
	 *  OP_VALUE_LE query this holds the value to compare against.
	 */
	std::string tname;

	/** Used to store the end of a range query. */
	std::string str_parameter;

	/// Position in query of this term - leaf node only
	Xapian::termpos term_pos;

	/// External posting source.
	Xapian::PostingSource * external_source;

	/// Flag, indicating whether the external source is owned by the query.
	bool external_source_owned;

	/// Copy another Xapian::Query::Internal into self.
	void initialise_from_copy(const Query::Internal & copyme);

        void accumulate_terms(
	    std::vector<std::pair<std::string, Xapian::termpos> > &terms) const;

	/** Simplify the query.
	 *  For example, an AND query with only one subquery would become the
	 *  subquery itself.
	 */
	Internal * simplify_query();

	/** Perform checks that query is valid. (e.g., has correct number of
	 *  sub queries.)  Throw an exception if not.  This is initially called
	 *  on the query before any simplifications have been made, and after
	 *  simplifications.
	 */
	void validate_query() const;

	/** Simplify any matchnothing subqueries, either eliminating them,
	 *  or setting this query to matchnothing, depending on the query
	 *  operator.  Returns true if simplification resulted in a
	 *  matchnothing query.
	 */
	bool simplify_matchnothing();

	/** Get a string describing the given query type.
	 */
	static std::string get_op_name(Xapian::Query::Internal::op_t op);

	/** Collapse the subqueries together if appropriate.
	 */
	void collapse_subqs();

	/** Flatten a query structure, by changing, for example,
	 *  "A NEAR (B AND C)" to "(A NEAR B) AND (A NEAR C)"
	 */
	Xapian::Query::Internal * flatten_subqs();

        /** Implementation of serialisation; called recursively.
         */
	std::string serialise(Xapian::termpos & curpos) const;

    public:
	/** Copy constructor. */
	Internal(const Query::Internal & copyme);

	/** Assignment. */
	void operator=(const Query::Internal & copyme);

	/** A query consisting of a single term. */
	explicit Internal(const std::string & tname_, Xapian::termcount wqf_ = 1,
			  Xapian::termpos term_pos_ = 0);

	/** Create internals given only the operator and a parameter. */
	Internal(op_t op_, Xapian::termcount parameter);

	/** Construct a range query on a document value. */
	Internal(op_t op_, Xapian::valueno slot,
		 const std::string &begin, const std::string &end);

	/** Construct a value greater-than-or-equal query on a document value.
	 */
	Internal(op_t op_, Xapian::valueno slot, const std::string &value);

	/// Construct an external source query.
	explicit Internal(Xapian::PostingSource * external_source_, bool owned);

	/** Destructor. */
	~Internal();

	static Xapian::Query::Internal * unserialise(const std::string &s,
						     const Registry & registry);

	/** Add a subquery. */
	void add_subquery(const Query::Internal * subq);

	/** Add a subquery without copying it.
	 *
	 *  subq is owned by the object this is called on after the call.
	 */
	void add_subquery_nocopy(Query::Internal * subq);

	void set_dbl_parameter(double dbl_parameter_);

	double get_dbl_parameter() const;

	/** Finish off the construction.
	 */
	Query::Internal * end_construction();

	/** Return a string in an easily parsed form
	 *  which contains all the information in a query.
	 */
	std::string serialise() const {
            Xapian::termpos curpos = 1;
            return serialise(curpos);
        }

	/// Return a string describing this object.
	std::string get_description() const;

	/** Get the numeric parameter used in this query.
	 *
	 *  This is used by the QueryParser to get the value number for
	 *  VALUE_RANGE queries.  It should be replaced by a public method on
	 *  the Query class at some point, but the API which should be used for
	 *  that is unclear, so this is a temporary workaround.
	 */
	Xapian::termcount get_parameter() const { return parameter; }

	Xapian::termcount get_wqf() const { return parameter; }

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

#endif // SWIG

}

#endif /* XAPIAN_INCLUDED_QUERY_H */
