/** @file query.h
 * @brief Xapian::Query API class
 */
/* Copyright (C) 2011,2012,2013 Olly Betts
 * Copyright (C) 2008 Richard Boulton
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_QUERY_H
#define XAPIAN_INCLUDED_QUERY_H

#if !defined XAPIAN_INCLUDED_XAPIAN_H && !defined XAPIAN_LIB_BUILD
# error "Never use <xapian/query.h> directly; include <xapian.h> instead."
#endif

#include <string>

#include <xapian/attributes.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/postingiterator.h>
#include <xapian/registry.h>
#include <xapian/termiterator.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

class QueryOptimiser; // FIXME

namespace Xapian {

class PostingSource;

/// Class representing a query.
class XAPIAN_VISIBILITY_DEFAULT Query {
  public:
    /// Class representing the query internals.
    class Internal;
    /// @private @internal Reference counted internals.
    Xapian::Internal::intrusive_ptr<Internal> internal;

    static const Xapian::Query MatchNothing;
    static const Xapian::Query MatchAll;

    /** Query operators. */
    enum op {
	OP_AND = 0,
	OP_OR = 1,
	OP_AND_NOT = 2,
	OP_XOR = 3,
	OP_AND_MAYBE = 4,
	OP_FILTER = 5,
	OP_NEAR = 6,
	OP_PHRASE = 7,
	OP_VALUE_RANGE = 8,
	OP_SCALE_WEIGHT = 9,

	/** Pick the best N subqueries and combine with OP_OR.
	 *
	 *  If you want to implement a feature which finds documents similar to
	 *  a piece of text, an obvious approach is to build an "OR" query from
	 *  all the terms in the text, and run this query against a database
	 *  containing the documents.  However such a query can contain a lots
	 *  of terms and be quite slow to perform, yet many of these terms
	 *  don't contribute usefully to the results.
	 *
	 *  The OP_ELITE_SET operator can be used instead of OP_OR in this
	 *  situation.  OP_ELITE_SET selects the most important ''N'' terms and
	 *  then acts as an OP_OR query with just these, ignoring any other
	 *  terms.  This will usually return results just as good as the full
	 *  OP_OR query, but much faster.
	 *
	 *  In general, the OP_ELITE_SET operator can be used when you have a
	 *  large OR query, but it doesn't matter if the search completely
	 *  ignores some of the less important terms in the query.
	 *
	 *  The subqueries don't have to be terms, but if they aren't then
	 *  OP_ELITE_SET will look at the estimated frequencies of the
	 *  subqueries and so could pick a subset which don't actually
	 *  match any documents even if the full OR would match some.
	 *
	 *  You can specify a parameter to the query constructor which control
	 *  the number of terms which OP_ELITE_SET will pick.  If not
	 *  specified, this defaults to 10 (Xapian used to default to
	 *  <code>ceil(sqrt(number_of_subqueries))</code> if there are more
	 *  than 100 subqueries, but this rather arbitrary special case was
	 *  dropped in 1.3.0).  For example, this will pick the best 7 terms:
	 *
	 *  <pre>
	 *  Xapian::Query query(Xapian::Query::OP_ELITE_SET, subqs.begin(), subqs.end(), 7);
	 *  </pre>
	 *
	 * If the number of subqueries is less than this threshold,
	 * OP_ELITE_SET behaves identically to OP_OR.
	 */
	OP_ELITE_SET = 10,
	OP_VALUE_GE = 11,
	OP_VALUE_LE = 12,
	OP_SYNONYM = 13
    };

    /// Default constructor.
    XAPIAN_NOTHROW(Query())
	: internal(0) { }

    /// Destructor.
    ~Query() { }

    /** Copying is allowed.
     *
     *  The internals are reference counted, so copying is cheap.
     */
    Query(const Query & o) : internal(o.internal) { }

    /** Copying is allowed.
     *
     *  The internals are reference counted, so assignment is cheap.
     */
    Query & operator=(const Query & o) { internal = o.internal; return *this; }

    /** Construct a Query object for a term. */
    Query(const std::string & term,
	  Xapian::termcount wqf = 1,
	  Xapian::termpos pos = 0);

    /** Construct a Query object for a PostingSource. */
    Query(Xapian::PostingSource * source);

    // FIXME: new form for OP_SCALE_WEIGHT - do we want this?
    Query(double factor, const Xapian::Query & subquery);

    // FIXME: legacy form of above (assuming we want to add that...)
    Query(op op_, const Xapian::Query & subquery, double factor);

    // Pairwise.
    Query(op op_, const Xapian::Query & a, const Xapian::Query & b)
    {
	init(op_, 2);
	add_subquery(a);
	add_subquery(b);
	done();
    }

    // Pairwise with std::string.
    Query(op op_, const std::string & a, const std::string & b)
    {
	init(op_, 2);
	add_subquery(a);
	add_subquery(b);
	done();
    }

    // OP_VALUE_GE/OP_VALUE_LE
    Query(op op_, Xapian::valueno slot, const std::string & limit);

    // OP_VALUE_RANGE
    Query(op op_, Xapian::valueno slot,
	  const std::string & begin, const std::string & end);

    template<typename I>
    Query(op op_, I begin, I end, Xapian::termcount window = 0)
    {
	if (begin != end) {
	    typedef typename std::iterator_traits<I>::iterator_category iterator_category;
	    init(op_, window, begin, end, iterator_category());
	    for (I i = begin; i != end; ++i) {
		add_subquery(*i);
	    }
	}
	done();
    }

#ifdef SWIG
    // SWIG's %template doesn't seem to handle a templated ctor so we
    // provide this fake specialised form of the above prototype.
    Query::Query(op op_, XapianSWIGQueryItor qbegin, XapianSWIGQueryItor qend,
		 Xapian::termcount parameter = 0);

# ifdef SWIGJAVA
    Query::Query(op op_, XapianSWIGStrItor qbegin, XapianSWIGStrItor qend,
		 Xapian::termcount parameter = 0);
# endif
#endif

    const TermIterator get_terms_begin() const;

    const TermIterator XAPIAN_NOTHROW(get_terms_end() const) {
	return TermIterator();
    }

    Xapian::termcount get_length() const XAPIAN_PURE_FUNCTION;

    bool XAPIAN_NOTHROW(empty() const) XAPIAN_PURE_FUNCTION {
	return internal.get() == 0;
    }

    std::string serialise() const;

    static const Query unserialise(const std::string & serialised,
				   const Registry & reg = Registry());

    std::string get_description() const XAPIAN_PURE_FUNCTION;

    const Query operator&=(const Query & o) {
	return (*this = Query(OP_AND, *this, o));
    }

    const Query operator|=(const Query & o) {
	return (*this = Query(OP_OR, *this, o));
    }

    const Query operator^=(const Query & o) {
	return (*this = Query(OP_XOR, *this, o));
    }

    const Query operator*=(double factor) {
	return (*this = Query(factor, *this));
    }

    const Query operator/=(double factor) {
	return (*this = Query(1.0 / factor, *this));
    }

    /** @private @internal
     *
     *  Pass a reference to avoid ambiguity for Query(NULL) (not useful, but the
     *  testsuite does it...)  FIXME
     */
    Query(Internal & internal_) : internal(&internal_) { }

  private:
    void init(Query::op op_, size_t n_subqueries, Xapian::termcount window = 0);

    template<typename I>
    void init(Query::op op_, Xapian::termcount window,
	      const I & begin, const I & end, std::random_access_iterator_tag)
    {
	init(op_, end - begin, window);
    }

    template<typename I>
    void init(Query::op op_, Xapian::termcount window,
	      const I &, const I &, std::input_iterator_tag)
    {
	init(op_, 0, window);
    }

    void add_subquery(const Xapian::Query & subquery);

    void add_subquery(const std::string & subquery) {
	add_subquery(Xapian::Query(subquery));
    }

    void add_subquery(const Xapian::Query * subquery) {
	// FIXME: subquery NULL?
	add_subquery(*subquery);
    }

    void done();
};

inline const Query
operator&(const Query & a, const Query & b)
{
    return Query(Query::OP_AND, a, b);
}

inline const Query
operator|(const Query & a, const Query & b)
{
    return Query(Query::OP_OR, a, b);
}

inline const Query
operator^(const Query & a, const Query & b)
{
    return Query(Query::OP_XOR, a, b);
}

inline const Query
operator*(double factor, const Query & q)
{
    return Query(factor, q);
}

inline const Query
operator*(const Query & q, double factor)
{
    return Query(factor, q);
}

inline const Query
operator/(const Query & q, double factor)
{
    return Query(1.0 / factor, q);
}

class InvertedQuery_ {
    const Query & query;

    void operator=(const InvertedQuery_ &);

    InvertedQuery_(const Query & query_) : query(query_) { }

  public:
    // GCC 4.2 seems to needs a copy ctor.
    InvertedQuery_(const InvertedQuery_ & o) : query(o.query) { }

    operator Query() const {
	return Query(Query::OP_AND_NOT, Query::MatchAll, query);
    }

    friend const InvertedQuery_ operator~(const Query &q);

    friend const Query operator&(const Query & a, const InvertedQuery_ & b);
};

inline const Query
operator&(const Query & a, const InvertedQuery_ & b)
{
    return Query(Query::OP_AND_NOT, a, b.query);
}

inline const InvertedQuery_
operator~(const Query &q)
{
    return InvertedQuery_(q);
}

namespace Internal {
class AndContext;
class OrContext;
class XorContext;
}

class Query::Internal : public Xapian::Internal::intrusive_base {
  public:
    XAPIAN_NOTHROW(Internal()) { }

    virtual ~Internal();

    virtual PostingIterator::Internal * postlist(QueryOptimiser * qopt, double factor) const = 0;

    virtual void postlist_sub_and_like(Xapian::Internal::AndContext& ctx,
				       QueryOptimiser * qopt,
				       double factor) const;

    virtual void postlist_sub_or_like(Xapian::Internal::OrContext& ctx,
				      QueryOptimiser * qopt,
				      double factor) const;

    virtual void postlist_sub_xor(Xapian::Internal::XorContext& ctx,
				  QueryOptimiser * qopt,
				  double factor) const;

    virtual termcount get_length() const XAPIAN_PURE_FUNCTION;

    virtual void serialise(std::string & result) const = 0;

    static Query::Internal * unserialise(const char ** p, const char * end, const Registry & reg);

    virtual std::string get_description() const XAPIAN_PURE_FUNCTION = 0;

    // Pass argument as void* to avoid need to include <vector>.
    virtual void gather_terms(void * void_terms) const;
};

}

#endif // XAPIAN_INCLUDED_QUERY_H
