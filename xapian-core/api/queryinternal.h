/** @file queryinternal.h
 * @brief Xapian::Query internals
 */
/* Copyright (C) 2011,2012,2013,2014 Olly Betts
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

#ifndef XAPIAN_INCLUDED_QUERYINTERNAL_H
#define XAPIAN_INCLUDED_QUERYINTERNAL_H

#include "postlist.h"
#include "queryvector.h"
#include "xapian/intrusive_ptr.h"
#include "xapian/query.h"

/// Default set_size for OP_ELITE_SET:
const Xapian::termcount DEFAULT_ELITE_SET_SIZE = 10;

class QueryOptimiser;

namespace Xapian {
namespace Internal {

class QueryTerm : public Query::Internal {
    std::string term;

    Xapian::termcount wqf;

    Xapian::termpos pos;

  public:
    QueryTerm(const std::string & term_,
	      Xapian::termcount wqf_,
	      Xapian::termpos pos_)
	: term(term_), wqf(wqf_), pos(pos_) { }

    Xapian::Query::op get_type() const;

    PostingIterator::Internal * postlist(QueryOptimiser * qopt, double factor) const;

    termcount get_length() const { return wqf; }

    void serialise(std::string & result) const;

    std::string get_description() const;

    void gather_terms(void * void_terms) const;
};

class QueryPostingSource : public Query::Internal {
    PostingSource * source;

    bool owned;

  public:
    QueryPostingSource(PostingSource * source_, bool owned_ = false);

    ~QueryPostingSource();

    PostingIterator::Internal * postlist(QueryOptimiser *qopt, double factor) const;

    void serialise(std::string & result) const;

    Xapian::Query::op get_type() const;

    std::string get_description() const;
};

class QueryScaleWeight : public Query::Internal {
    double scale_factor;

    Query subquery;

  public:
    QueryScaleWeight(double factor, const Query & subquery_);

    PostingIterator::Internal * postlist(QueryOptimiser *qopt, double factor) const;

    termcount get_length() const { return subquery.internal->get_length(); }

    void serialise(std::string & result) const;

    Xapian::Query::op get_type() const;
    size_t get_num_subqueries() const;
    const Query get_subquery(size_t n) const;

    std::string get_description() const;

    void gather_terms(void * void_terms) const;
};

class QueryValueRange : public Query::Internal {
    Xapian::valueno slot;

    std::string begin, end;

  public:
    QueryValueRange(Xapian::valueno slot_,
		    const std::string &begin_,
		    const std::string &end_)
	: slot(slot_), begin(begin_), end(end_) { }

    PostingIterator::Internal * postlist(QueryOptimiser *qopt, double factor) const;

    void serialise(std::string & result) const;

    Xapian::Query::op get_type() const;

    std::string get_description() const;
};

class QueryValueLE : public Query::Internal {
    Xapian::valueno slot;

    std::string limit;

  public:
    QueryValueLE(Xapian::valueno slot_, const std::string &limit_)
	: slot(slot_), limit(limit_) { }

    PostingIterator::Internal * postlist(QueryOptimiser *qopt, double factor) const;

    void serialise(std::string & result) const;

    Xapian::Query::op get_type() const;

    std::string get_description() const;
};

class QueryValueGE : public Query::Internal {
    Xapian::valueno slot;

    std::string limit;

  public:
    QueryValueGE(Xapian::valueno slot_, const std::string &limit_)
	: slot(slot_), limit(limit_) { }

    PostingIterator::Internal * postlist(QueryOptimiser *qopt, double factor) const;

    void serialise(std::string & result) const;

    Xapian::Query::op get_type() const;

    std::string get_description() const;
};

class QueryBranch : public Query::Internal {
    virtual Xapian::Query::op get_op() const = 0;

  protected:
    QueryVector subqueries;

    QueryBranch(size_t n_subqueries) : subqueries(n_subqueries) { }

    void serialise_(string & result, Xapian::termcount parameter = 0) const;

    void do_or_like(OrContext& ctx, QueryOptimiser * qopt, double factor,
		    Xapian::termcount elite_set_size = 0, size_t first = 0) const;

    PostList * do_synonym(QueryOptimiser * qopt, double factor) const;

    PostList * do_max(QueryOptimiser * qopt, double factor) const;

    const std::string get_description_helper(const char * op,
					     Xapian::termcount window = 0) const;

  public:
    termcount get_length() const;
 
    void serialise(std::string & result) const;

    void gather_terms(void * void_terms) const;

    virtual void add_subquery(const Xapian::Query & subquery) = 0;

    Xapian::Query::op get_type() const;
    size_t get_num_subqueries() const;
    const Query get_subquery(size_t n) const;

    virtual Query::Internal * done() = 0;
};

class QueryAndLike : public QueryBranch {
  protected:
    QueryAndLike(size_t num_subqueries_) : QueryBranch(num_subqueries_) { }

  public:
    void add_subquery(const Xapian::Query & subquery);

    Query::Internal * done();

    PostingIterator::Internal * postlist(QueryOptimiser * qopt, double factor) const;

    void postlist_sub_and_like(AndContext& ctx, QueryOptimiser * qopt, double factor) const;
};

class QueryOrLike : public QueryBranch {
  protected:
    QueryOrLike(size_t num_subqueries_) : QueryBranch(num_subqueries_) { }

  public:
    void add_subquery(const Xapian::Query & subquery);

    Query::Internal * done();
};

class QueryAnd : public QueryAndLike {
    Xapian::Query::op get_op() const;

  public:
    QueryAnd(size_t n_subqueries) : QueryAndLike(n_subqueries) { }

    std::string get_description() const;
};

class QueryOr : public QueryOrLike {
    Xapian::Query::op get_op() const;

  public:
    QueryOr(size_t n_subqueries) : QueryOrLike(n_subqueries) { }

    PostingIterator::Internal * postlist(QueryOptimiser * qopt, double factor) const;

    void postlist_sub_or_like(OrContext& ctx, QueryOptimiser * qopt, double factor) const;

    std::string get_description() const;
};

class QueryAndNot : public QueryBranch {
    Xapian::Query::op get_op() const;

  public:
    QueryAndNot(size_t n_subqueries) : QueryBranch(n_subqueries) { }

    PostingIterator::Internal * postlist(QueryOptimiser * qopt, double factor) const;

    void add_subquery(const Xapian::Query & subquery);

    Query::Internal * done();

    std::string get_description() const;
};

class QueryXor : public QueryOrLike {
    Xapian::Query::op get_op() const;

  public:
    QueryXor(size_t n_subqueries) : QueryOrLike(n_subqueries) { }

    PostingIterator::Internal * postlist(QueryOptimiser * qopt, double factor) const;

    void postlist_sub_xor(XorContext& ctx, QueryOptimiser * qopt, double factor) const;

    std::string get_description() const;
};

class QueryAndMaybe : public QueryBranch {
    Xapian::Query::op get_op() const;

  public:
    QueryAndMaybe(size_t n_subqueries) : QueryBranch(n_subqueries) { }

    PostingIterator::Internal * postlist(QueryOptimiser * qopt, double factor) const;

    void add_subquery(const Xapian::Query & subquery);

    Query::Internal * done();

    std::string get_description() const;
};

class QueryFilter : public QueryAndLike {
    Xapian::Query::op get_op() const;

  public:
    QueryFilter(size_t n_subqueries) : QueryAndLike(n_subqueries) { }

    PostingIterator::Internal * postlist(QueryOptimiser * qopt, double factor) const;

    void postlist_sub_and_like(AndContext& ctx, QueryOptimiser * qopt, double factor) const;

    std::string get_description() const;
};

class QueryWindowed : public QueryAndLike {
  protected:
    Xapian::termcount window;

    QueryWindowed(size_t n_subqueries, Xapian::termcount window_)
	: QueryAndLike(n_subqueries), window(window_) { }
  
    void postlist_windowed(Xapian::Query::op op, AndContext& ctx,
			   QueryOptimiser * qopt, double factor) const;

  public:
    Query::Internal * done();
};

class QueryNear : public QueryWindowed {
    Xapian::Query::op get_op() const;

  public:
    QueryNear(size_t n_subqueries, Xapian::termcount window_)
	: QueryWindowed(n_subqueries, window_) { }

    void serialise(string & result) const;

    void postlist_sub_and_like(AndContext& ctx, QueryOptimiser * qopt, double factor) const;

    std::string get_description() const;
};

class QueryPhrase : public QueryWindowed {
    Xapian::Query::op get_op() const;

  public:
    QueryPhrase(size_t n_subqueries, Xapian::termcount window_)
	: QueryWindowed(n_subqueries, window_) { }

    void serialise(string & result) const;

    void postlist_sub_and_like(AndContext& ctx, QueryOptimiser * qopt, double factor) const;

    std::string get_description() const;
};

class QueryEliteSet : public QueryOrLike {
    Xapian::Query::op get_op() const;

    Xapian::termcount set_size;

  public:
    QueryEliteSet(size_t n_subqueries, Xapian::termcount set_size_)
	: QueryOrLike(n_subqueries),
	  set_size(set_size_ ? set_size_ : DEFAULT_ELITE_SET_SIZE) { }

    void serialise(string & result) const;

    PostingIterator::Internal * postlist(QueryOptimiser * qopt, double factor) const;

    void postlist_sub_or_like(OrContext& ctx, QueryOptimiser * qopt, double factor) const;

    std::string get_description() const;
};

class QuerySynonym : public QueryOrLike {
    Xapian::Query::op get_op() const;

  public:
    QuerySynonym(size_t n_subqueries) : QueryOrLike(n_subqueries) { }

    PostingIterator::Internal * postlist(QueryOptimiser * qopt, double factor) const;

    Query::Internal * done();

    std::string get_description() const;
};

class QueryMax : public QueryOrLike {
    Xapian::Query::op get_op() const;

  public:
    QueryMax(size_t n_subqueries) : QueryOrLike(n_subqueries) { }

    PostingIterator::Internal * postlist(QueryOptimiser * qopt, double factor) const;

    std::string get_description() const;
};

}

}

#endif // XAPIAN_INCLUDED_QUERYINTERNAL_H
