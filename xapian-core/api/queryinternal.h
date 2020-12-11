/** @file
 * @brief Xapian::Query internals
 */
/* Copyright (C) 2011,2012,2013,2014,2015,2016,2017,2018,2019 Olly Betts
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

#include "api/editdistance.h"
#include "queryvector.h"
#include "stringutils.h"
#include "xapian/intrusive_ptr.h"
#include "xapian/query.h"

/// Default set_size for OP_ELITE_SET:
const Xapian::termcount DEFAULT_ELITE_SET_SIZE = 10;

namespace Xapian {
namespace Internal {

class PostList;
class QueryOptimiser;

class QueryTerm : public Query::Internal {
    std::string term;

    Xapian::termcount wqf;

    Xapian::termpos pos;

  public:
    // Construct a "MatchAll" QueryTerm.
    QueryTerm() : term(), wqf(1), pos(0) { }

    QueryTerm(const std::string & term_,
	      Xapian::termcount wqf_,
	      Xapian::termpos pos_)
	: term(term_), wqf(wqf_), pos(pos_) { }

    Xapian::Query::op get_type() const noexcept XAPIAN_PURE_FUNCTION;

    const std::string & get_term() const { return term; }

    termcount get_wqf() const { return wqf; }

    termpos get_pos() const { return pos; }

    PostList* postlist(QueryOptimiser * qopt, double factor) const;

    bool postlist_sub_and_like(AndContext& ctx,
			       QueryOptimiser* qopt,
			       double factor) const;

    termcount get_length() const noexcept XAPIAN_PURE_FUNCTION {
	return wqf;
    }

    void serialise(std::string & result) const;

    std::string get_description() const;

    void gather_terms(void * void_terms) const;
};

class QueryPostingSource : public Query::Internal {
    Xapian::Internal::opt_intrusive_ptr<PostingSource> source;

  public:
    explicit QueryPostingSource(PostingSource * source_);

    PostList* postlist(QueryOptimiser *qopt, double factor) const;

    void serialise(std::string & result) const;

    Xapian::Query::op get_type() const noexcept XAPIAN_PURE_FUNCTION;

    std::string get_description() const;
};

class QueryScaleWeight : public Query::Internal {
    double scale_factor;

    Query subquery;

  public:
    QueryScaleWeight(double factor, const Query & subquery_);

    PostList* postlist(QueryOptimiser *qopt, double factor) const;

    bool postlist_sub_and_like(AndContext& ctx,
			       QueryOptimiser* qopt,
			       double factor) const;

    termcount get_length() const noexcept XAPIAN_PURE_FUNCTION {
	return subquery.internal->get_length();
    }

    void serialise(std::string & result) const;

    Xapian::Query::op get_type() const noexcept XAPIAN_PURE_FUNCTION;
    size_t get_num_subqueries() const noexcept XAPIAN_PURE_FUNCTION;
    const Query get_subquery(size_t n) const;

    std::string get_description() const;

    void gather_terms(void * void_terms) const;
};

class QueryValueBase : public Query::Internal {
  protected:
    Xapian::valueno slot;

  public:
    explicit QueryValueBase(Xapian::valueno slot_)
	: slot(slot_) { }

    Xapian::valueno get_slot() const { return slot; }
};

class QueryValueRange : public QueryValueBase {
    std::string begin, end;

  public:
    QueryValueRange(Xapian::valueno slot_,
		    const std::string &begin_,
		    const std::string &end_)
	: QueryValueBase(slot_), begin(begin_), end(end_) { }

    PostList* postlist(QueryOptimiser *qopt, double factor) const;

    void serialise(std::string & result) const;

    Xapian::Query::op get_type() const noexcept XAPIAN_PURE_FUNCTION;

    std::string get_description() const;
};

class QueryValueLE : public QueryValueBase {
    std::string limit;

  public:
    QueryValueLE(Xapian::valueno slot_, const std::string &limit_)
	: QueryValueBase(slot_), limit(limit_) { }

    PostList* postlist(QueryOptimiser *qopt, double factor) const;

    void serialise(std::string & result) const;

    Xapian::Query::op get_type() const noexcept XAPIAN_PURE_FUNCTION;

    std::string get_description() const;
};

class QueryValueGE : public QueryValueBase {
    std::string limit;

  public:
    QueryValueGE(Xapian::valueno slot_, const std::string &limit_)
	: QueryValueBase(slot_), limit(limit_) { }

    PostList* postlist(QueryOptimiser *qopt, double factor) const;

    void serialise(std::string & result) const;

    Xapian::Query::op get_type() const noexcept XAPIAN_PURE_FUNCTION;

    std::string get_description() const;
};

class QueryBranch : public Query::Internal {
    virtual Xapian::Query::op get_op() const = 0;

  protected:
    QueryVector subqueries;

    explicit QueryBranch(size_t n_subqueries) : subqueries(n_subqueries) { }

    void serialise_(std::string & result, Xapian::termcount parameter = 0) const;

    void do_bool_or_like(BoolOrContext& ctx,
			 QueryOptimiser* qopt,
			 size_t first = 0) const;

    /** Process OR-like subqueries.
     *
     *  @param keep_zero_weight  By default zero-weight subqueries are kept,
     *				 but in some situations (such as on the right
     *				 side of OP_AND_MAYBE when not under
     *				 OP_SYNONYM) they can be ignored.
     */
    void do_or_like(OrContext& ctx, QueryOptimiser* qopt, double factor,
		    Xapian::termcount elite_set_size = 0, size_t first = 0,
		    bool keep_zero_weight = true) const;

    PostList* do_synonym(QueryOptimiser * qopt, double factor) const;

    PostList* do_max(QueryOptimiser * qopt, double factor) const;

    const std::string get_description_helper(const char * op,
					     Xapian::termcount window = 0) const;

  public:
    termcount get_length() const noexcept XAPIAN_PURE_FUNCTION;

    void serialise(std::string & result) const;

    void gather_terms(void * void_terms) const;

    virtual void add_subquery(const Xapian::Query & subquery) = 0;

    Xapian::Query::op get_type() const noexcept XAPIAN_PURE_FUNCTION;
    size_t get_num_subqueries() const noexcept XAPIAN_PURE_FUNCTION;
    const Query get_subquery(size_t n) const;

    virtual Query::Internal * done() = 0;
};

class QueryAndLike : public QueryBranch {
  protected:
    explicit QueryAndLike(size_t num_subqueries_)
	: QueryBranch(num_subqueries_) { }

  public:
    void add_subquery(const Xapian::Query & subquery);

    Query::Internal * done();

    PostList* postlist(QueryOptimiser * qopt, double factor) const;

    bool postlist_sub_and_like(AndContext& ctx,
			       QueryOptimiser* qopt,
			       double factor) const;
};

class QueryOrLike : public QueryBranch {
  protected:
    explicit QueryOrLike(size_t num_subqueries_)
	: QueryBranch(num_subqueries_) { }

  public:
    void add_subquery(const Xapian::Query & subquery);

    Query::Internal * done();
};

class QueryAnd : public QueryAndLike {
    Xapian::Query::op get_op() const;

  public:
    explicit QueryAnd(size_t n_subqueries) : QueryAndLike(n_subqueries) { }

    std::string get_description() const;
};

class QueryOr : public QueryOrLike {
    Xapian::Query::op get_op() const;

  public:
    explicit QueryOr(size_t n_subqueries) : QueryOrLike(n_subqueries) { }

    PostList* postlist(QueryOptimiser * qopt, double factor) const;

    void postlist_sub_or_like(OrContext& ctx, QueryOptimiser* qopt,
			      double factor, bool keep_zero_weight) const;

    void postlist_sub_bool_or_like(BoolOrContext& ctx,
				   QueryOptimiser* qopt) const;

    std::string get_description() const;
};

class QueryAndNot : public QueryBranch {
    Xapian::Query::op get_op() const;

  public:
    explicit QueryAndNot(size_t n_subqueries) : QueryBranch(n_subqueries) { }

    PostList* postlist(QueryOptimiser * qopt, double factor) const;

    bool postlist_sub_and_like(AndContext& ctx,
			       QueryOptimiser* qopt,
			       double factor) const;

    void add_subquery(const Xapian::Query & subquery);

    Query::Internal * done();

    std::string get_description() const;
};

class QueryXor : public QueryOrLike {
    Xapian::Query::op get_op() const;

  public:
    explicit QueryXor(size_t n_subqueries) : QueryOrLike(n_subqueries) { }

    PostList* postlist(QueryOptimiser * qopt, double factor) const;

    void postlist_sub_xor(XorContext& ctx, QueryOptimiser * qopt, double factor) const;

    std::string get_description() const;
};

class QueryAndMaybe : public QueryBranch {
    Xapian::Query::op get_op() const;

  public:
    explicit QueryAndMaybe(size_t n_subqueries) : QueryBranch(n_subqueries) { }

    PostList* postlist(QueryOptimiser * qopt, double factor) const;

    bool postlist_sub_and_like(AndContext& ctx,
			       QueryOptimiser* qopt,
			       double factor) const;

    void add_subquery(const Xapian::Query & subquery);

    Query::Internal * done();

    std::string get_description() const;
};

class QueryFilter : public QueryAndLike {
    Xapian::Query::op get_op() const;

  public:
    explicit QueryFilter(size_t n_subqueries) : QueryAndLike(n_subqueries) { }

    PostList* postlist(QueryOptimiser * qopt, double factor) const;

    bool postlist_sub_and_like(AndContext& ctx,
			       QueryOptimiser* qopt,
			       double factor) const;

    std::string get_description() const;
};

class QueryWindowed : public QueryAndLike {
  protected:
    Xapian::termcount window;

    QueryWindowed(size_t n_subqueries, Xapian::termcount window_)
	: QueryAndLike(n_subqueries), window(window_) { }

    bool postlist_windowed(Xapian::Query::op op, AndContext& ctx,
			   QueryOptimiser * qopt, double factor) const;

  public:
    size_t get_window() const { return window; }

    Query::Internal * done();
};

class QueryNear : public QueryWindowed {
    Xapian::Query::op get_op() const;

  public:
    QueryNear(size_t n_subqueries, Xapian::termcount window_)
	: QueryWindowed(n_subqueries, window_) { }

    void serialise(std::string & result) const;

    bool postlist_sub_and_like(AndContext& ctx,
			       QueryOptimiser* qopt,
			       double factor) const;

    std::string get_description() const;
};

class QueryPhrase : public QueryWindowed {
    Xapian::Query::op get_op() const;

  public:
    QueryPhrase(size_t n_subqueries, Xapian::termcount window_)
	: QueryWindowed(n_subqueries, window_) { }

    void serialise(std::string & result) const;

    bool postlist_sub_and_like(AndContext& ctx,
			       QueryOptimiser* qopt,
			       double factor) const;

    std::string get_description() const;
};

class QueryEliteSet : public QueryOrLike {
    Xapian::Query::op get_op() const;

    Xapian::termcount set_size;

  public:
    QueryEliteSet(size_t n_subqueries, Xapian::termcount set_size_)
	: QueryOrLike(n_subqueries),
	  set_size(set_size_ ? set_size_ : DEFAULT_ELITE_SET_SIZE) { }

    void serialise(std::string & result) const;

    PostList* postlist(QueryOptimiser * qopt, double factor) const;

    void postlist_sub_or_like(OrContext& ctx, QueryOptimiser* qopt,
			      double factor, bool keep_zero_weight) const;

    std::string get_description() const;
};

class QuerySynonym : public QueryOrLike {
    Xapian::Query::op get_op() const;

  public:
    explicit QuerySynonym(size_t n_subqueries) : QueryOrLike(n_subqueries) { }

    PostList* postlist(QueryOptimiser * qopt, double factor) const;

    Query::Internal * done();

    std::string get_description() const;
};

class QueryMax : public QueryOrLike {
    Xapian::Query::op get_op() const;

  public:
    explicit QueryMax(size_t n_subqueries) : QueryOrLike(n_subqueries) { }

    PostList* postlist(QueryOptimiser * qopt, double factor) const;

    std::string get_description() const;
};

class QueryWildcard : public Query::Internal {
    std::string pattern;

    Xapian::termcount max_expansion;

    int flags;

    Query::op combiner;

    /** Fixed head and tail lengths, and min/max length term that can match.
     *
     *  All in bytes.
     */
    size_t head = 0, tail = 0, min_len = 0, max_len = 0;

    // If the pattern is fixed apart from `*` or `*?` or `?*` then the length
    // checks and head/tail checks are sufficient.  This covers a lot of common
    // cases, so special-case it.  Note that we can't handle cases like `*??`
    // here since `?` matches a single UTF-8 character, which can be more than
    // one byte.
    bool check_pattern = false;

    std::string prefix, suffix;

    bool test_wildcard_(const std::string& candidate, size_t o, size_t p,
			size_t i) const;

  public:
    QueryWildcard(const std::string &pattern_,
		  Xapian::termcount max_expansion_,
		  int flags_,
		  Query::op combiner_);

    /// Perform wildcard test on candidate known to match prefix.
    bool test_prefix_known(const std::string& candidate) const;

    /// Perform full wildcard test on candidate.
    bool test(const std::string& candidate) const {
	return startswith(candidate, prefix) && test_prefix_known(candidate);
    }

    Xapian::Query::op get_type() const noexcept XAPIAN_PURE_FUNCTION;

    std::string get_pattern() const { return pattern; }

    Xapian::termcount get_max_expansion() const { return max_expansion; }

    int get_just_flags() const {
	return flags &~ Xapian::Query::WILDCARD_LIMIT_MASK_;
    }

    int get_max_type() const {
	return flags & Xapian::Query::WILDCARD_LIMIT_MASK_;
    }

    PostList* postlist(QueryOptimiser * qopt, double factor) const;

    termcount get_length() const noexcept XAPIAN_PURE_FUNCTION;

    void serialise(std::string & result) const;

    /** Change the combining operator.
     *
     *  If there's only one reference to this object we change in-place
     *  and return a pointer to the existing object; otherwise we create and
     *  return a new QueryWildcard object.
     */
    QueryWildcard* change_combiner(Xapian::Query::op new_op) {
	if (_refs == 1) {
	    combiner = new_op;
	    return this;
	}
	return new QueryWildcard(pattern,
				 max_expansion,
				 flags,
				 new_op);
    }

    /// Return the fixed prefix from the wildcard pattern.
    std::string get_fixed_prefix() const { return prefix; }

    std::string get_description() const;
};

class QueryEditDistance : public Query::Internal {
    std::string pattern;

    Xapian::termcount max_expansion;

    int flags;

    Query::op combiner;

    EditDistanceCalculator edcalc;

    unsigned edit_distance;

    size_t fixed_prefix_len;

  public:
    QueryEditDistance(const std::string& pattern_,
		      Xapian::termcount max_expansion_,
		      int flags_,
		      Query::op combiner_,
		      unsigned edit_distance_ = 2,
		      size_t fixed_prefix_len_ = 0)
	: pattern(pattern_),
	  max_expansion(max_expansion_),
	  flags(flags_),
	  combiner(combiner_),
	  edcalc(pattern),
	  edit_distance(edit_distance_),
	  fixed_prefix_len(fixed_prefix_len_) { }

    /** Perform edit distance test.
     *
     *  @return edit_distance + 1, or 0 for a non-match.
     */
    int test(const std::string& candidate) const;

    Xapian::Query::op get_type() const noexcept XAPIAN_PURE_FUNCTION;

    std::string get_pattern() const { return pattern; }

    size_t get_fixed_prefix_len() const { return fixed_prefix_len; }

    Xapian::termcount get_max_expansion() const { return max_expansion; }

    int get_just_flags() const {
	return flags &~ Xapian::Query::WILDCARD_LIMIT_MASK_;
    }

    int get_max_type() const {
	return flags & Xapian::Query::WILDCARD_LIMIT_MASK_;
    }

    unsigned get_threshold() const {
	return edit_distance;
    }

    PostList* postlist(QueryOptimiser * qopt, double factor) const;

    termcount get_length() const noexcept XAPIAN_PURE_FUNCTION;

    void serialise(std::string & result) const;

    /** Change the combining operator.
     *
     *  If there's only one reference to this object we change in-place
     *  and return a pointer to the existing object; otherwise we create and
     *  return a new QueryEditDistance object.
     */
    QueryEditDistance* change_combiner(Xapian::Query::op new_op) {
	if (_refs == 1) {
	    combiner = new_op;
	    return this;
	}
	return new QueryEditDistance(pattern,
				     max_expansion,
				     flags,
				     new_op,
				     edit_distance,
				     fixed_prefix_len);
    }

    std::string get_description() const;
};

class QueryInvalid : public Query::Internal {
  public:
    QueryInvalid() { }

    Xapian::Query::op get_type() const noexcept XAPIAN_PURE_FUNCTION;

    PostList* postlist(QueryOptimiser * qopt, double factor) const;

    void serialise(std::string & result) const;

    std::string get_description() const;
};

}

}

#endif // XAPIAN_INCLUDED_QUERYINTERNAL_H
