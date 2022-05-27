/** @file
 * @brief Xapian::Query internals
 */
/* Copyright (C) 2007,2008,2009,2010,2011,2012,2013,2014,2015,2016,2017 Olly Betts
 * Copyright (C) 2008,2009 Lemur Consulting Ltd
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

#include <config.h>

#include "queryinternal.h"

#include "xapian/error.h"
#include "xapian/postingsource.h"
#include "xapian/query.h"

#include "leafpostlist.h"
#include "matcher/andmaybepostlist.h"
#include "matcher/andnotpostlist.h"
#include "emptypostlist.h"
#include "matcher/exactphrasepostlist.h"
#include "matcher/externalpostlist.h"
#include "matcher/maxpostlist.h"
#include "matcher/multiandpostlist.h"
#include "matcher/multixorpostlist.h"
#include "matcher/nearpostlist.h"
#include "matcher/orpospostlist.h"
#include "matcher/orpostlist.h"
#include "matcher/phrasepostlist.h"
#include "matcher/queryoptimiser.h"
#include "matcher/valuerangepostlist.h"
#include "matcher/valuegepostlist.h"
#include "net/length.h"
#include "serialise-double.h"
#include "stringutils.h"
#include "termlist.h"

#include "autoptr.h"
#include "debuglog.h"
#include "omassert.h"
#include "str.h"
#include "unicode/description_append.h"

#include <algorithm>
#include <list>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;

using Xapian::Internal::AndContext;
using Xapian::Internal::OrContext;
using Xapian::Internal::XorContext;

namespace Xapian {

namespace Internal {

/** Class providing an operator which sorts postlists to select max or terms.
 *  This returns true if a has a (strictly) greater termweight than b,
 *  unless a or b contain no documents, in which case the other one is
 *  selected.
 */
struct CmpMaxOrTerms {
    /** Return true if and only if a has a strictly greater termweight than b. */
    bool operator()(const PostList *a, const PostList *b) {
#if (defined(__i386__) && !defined(__SSE_MATH__)) || \
    defined(__mc68000__) || defined(__mc68010__) || \
    defined(__mc68020__) || defined(__mc68030__)
	// On some architectures, most common of which is x86, floating point
	// values are calculated and stored in registers with excess precision.
	// If the two get_maxweight() calls below return identical values in a
	// register, the excess precision may be dropped for one of them but
	// not the other (e.g. because the compiler saves the first calculated
	// weight to memory while calculating the second, then reloads it to
	// compare).  This leads to both a > b and b > a being true, which
	// violates the antisymmetry property of the strict weak ordering
	// required by nth_element().  This can have serious consequences (e.g.
	// segfaults).
	//
	// Note that m68k only has excess precision in earlier models - 68040
	// and later are OK:
	// https://gcc.gnu.org/ml/gcc-patches/2008-11/msg00105.html
	//
	// To avoid this, we store each result in a volatile double prior to
	// comparing them.  This means that the result of this test should
	// match that on other architectures with the same double format (which
	// is desirable), and actually has less overhead than rounding both
	// results to float (which is another approach which works).
	volatile double a_max_wt = a->get_maxweight();
	volatile double b_max_wt = b->get_maxweight();
	return a_max_wt > b_max_wt;
#else
	return a->get_maxweight() > b->get_maxweight();
#endif
    }
};

/// Comparison functor which orders PostList* by descending get_termfreq_est().
struct ComparePostListTermFreqAscending {
    /// Order by descending get_termfreq_est().
    bool operator()(const PostList *a, const PostList *b) const {
	return a->get_termfreq_est() > b->get_termfreq_est();
    }
};

class Context {
  protected:
    QueryOptimiser* qopt;

    vector<PostList*> pls;

  public:
    Context(QueryOptimiser* qopt_, size_t reserve);

    ~Context();

    void add_postlist(PostList * pl) {
	pls.push_back(pl);
    }

    bool empty() const {
	return pls.empty();
    }

    size_t size() const {
	return pls.size();
    }

    void shrink(size_t new_size);
};

Context::Context(QueryOptimiser* qopt_, size_t reserve)
    : qopt(qopt_)
{
    pls.reserve(reserve);
}

void
Context::shrink(size_t new_size)
{
    AssertRel(new_size, <=, pls.size());
    if (new_size >= pls.size())
	return;

    for (auto&& i = pls.begin() + new_size; i != pls.end(); ++i) {
	qopt->destroy_postlist(*i);
    }
    pls.resize(new_size);
}

Context::~Context()
{
    shrink(0);
}

class OrContext : public Context {
  public:
    OrContext(QueryOptimiser* qopt_, size_t reserve)
	: Context(qopt_, reserve) { }

    /// Select the best set_size postlists from the last out_of added.
    void select_elite_set(size_t set_size, size_t out_of);

    /// Select the set_size postlists with the highest term frequency.
    void select_most_frequent(size_t set_size);

    PostList * postlist();
    PostList * postlist_max();
};

void
OrContext::select_elite_set(size_t set_size, size_t out_of)
{
    // Call recalc_maxweight() as otherwise get_maxweight()
    // may not be valid before next() or skip_to()
    auto begin = pls.begin() + pls.size() - out_of;
    for (auto i = begin; i != pls.end(); ++i) {
	(*i)->recalc_maxweight();
    }
    nth_element(begin, begin + set_size - 1, pls.end(), CmpMaxOrTerms());
    shrink(pls.size() - out_of + set_size);
}

void
OrContext::select_most_frequent(size_t set_size)
{
    vector<PostList*>::iterator begin = pls.begin();
    nth_element(begin, begin + set_size - 1, pls.end(),
		ComparePostListTermFreqAscending());
    shrink(set_size);
}

PostList *
OrContext::postlist()
{
    Assert(!pls.empty());

    if (pls.size() == 1) {
	PostList * pl = pls[0];
	pls.clear();
	return pl;
    }

    // Make postlists into a heap so that the postlist with the greatest term
    // frequency is at the top of the heap.
    make_heap(pls.begin(), pls.end(), ComparePostListTermFreqAscending());

    // Now build a tree of binary OrPostList objects.
    //
    // The algorithm used to build the tree is like that used to build an
    // optimal Huffman coding tree.  If we called next() repeatedly, this
    // arrangement would minimise the number of method calls.  Generally we
    // don't actually do that, but this arrangement is still likely to be a
    // good one, and it does minimise the work in the worst case.
    while (true) {
	// We build the tree such that at each branch:
	//
	//   l.get_termfreq_est() >= r.get_termfreq_est()
	//
	// We do this so that the OrPostList class can be optimised assuming
	// that this is the case.
	PostList * r = pls.front();
	pop_heap(pls.begin(), pls.end(), ComparePostListTermFreqAscending());
	pls.pop_back();
	PostList * pl;
	pl = new OrPostList(pls.front(), r, qopt->matcher, qopt->db_size);

	if (pls.size() == 1) {
	    pls.clear();
	    return pl;
	}

	pop_heap(pls.begin(), pls.end(), ComparePostListTermFreqAscending());
	pls.back() = pl;
	push_heap(pls.begin(), pls.end(), ComparePostListTermFreqAscending());
    }
}

PostList *
OrContext::postlist_max()
{
    Assert(!pls.empty());

    if (pls.size() == 1) {
	PostList * pl = pls[0];
	pls.clear();
	return pl;
    }

    // Sort the postlists so that the postlist with the greatest term frequency
    // is first.
    sort(pls.begin(), pls.end(), ComparePostListTermFreqAscending());

    PostList * pl;
    pl = new MaxPostList(pls.begin(), pls.end(), qopt->matcher, qopt->db_size);

    pls.clear();
    return pl;
}

class XorContext : public Context {
  public:
    XorContext(QueryOptimiser* qopt_, size_t reserve)
	: Context(qopt_, reserve) { }

    PostList * postlist();
};

PostList *
XorContext::postlist()
{
    Xapian::doccount db_size = qopt->db_size;
    PostList * pl;
    pl = new MultiXorPostList(pls.begin(), pls.end(), qopt->matcher, db_size);

    // Empty pls so our destructor doesn't delete them all!
    pls.clear();
    return pl;
}

class AndContext : public Context {
    class PosFilter {
	Xapian::Query::op op_;

	/// Start and end indices for the PostLists this positional filter uses.
	size_t begin, end;

	Xapian::termcount window;

      public:
	PosFilter(Xapian::Query::op op__, size_t begin_, size_t end_,
		  Xapian::termcount window_)
	    : op_(op__), begin(begin_), end(end_), window(window_) { }

	PostList * postlist(PostList * pl, const vector<PostList*>& pls) const;
    };

    list<PosFilter> pos_filters;

    AutoPtr<OrContext> not_ctx;

    AutoPtr<OrContext> maybe_ctx;

  public:
    AndContext(QueryOptimiser* qopt_, size_t reserve)
	: Context(qopt_, reserve) { }

    void add_pos_filter(Query::op op_,
			size_t n_subqs,
			Xapian::termcount window);

    OrContext& get_not_ctx(size_t reserve) {
	if (!not_ctx) {
	    not_ctx.reset(new OrContext(qopt, reserve));
	}
	return *not_ctx;
    }

    OrContext& get_maybe_ctx(size_t reserve) {
	if (!maybe_ctx) {
	    maybe_ctx.reset(new OrContext(qopt, reserve));
	}
	return *maybe_ctx;
    }

    PostList * postlist();
};

PostList *
AndContext::PosFilter::postlist(PostList * pl, const vector<PostList*>& pls) const
try {
    vector<PostList *>::const_iterator terms_begin = pls.begin() + begin;
    vector<PostList *>::const_iterator terms_end = pls.begin() + end;

    if (op_ == Xapian::Query::OP_NEAR) {
	pl = new NearPostList(pl, window, terms_begin, terms_end);
    } else if (window == end - begin) {
	AssertEq(op_, Xapian::Query::OP_PHRASE);
	pl = new ExactPhrasePostList(pl, terms_begin, terms_end);
    } else {
	AssertEq(op_, Xapian::Query::OP_PHRASE);
	pl = new PhrasePostList(pl, window, terms_begin, terms_end);
    }
    return pl;
} catch (...) {
    delete pl;
    throw;
}

void
AndContext::add_pos_filter(Query::op op_,
			   size_t n_subqs,
			   Xapian::termcount window)
{
    Assert(n_subqs > 1);
    size_t end = pls.size();
    size_t begin = end - n_subqs;
    pos_filters.push_back(PosFilter(op_, begin, end, window));
}

PostList *
AndContext::postlist()
{
    if (pls.empty()) {
	// This case only happens if this sub-database has no positional data
	// (but another sub-database does).
	Assert(pos_filters.empty());
	return new EmptyPostList;
    }

    auto matcher = qopt->matcher;
    auto db_size = qopt->db_size;

    AutoPtr<PostList> pl(new MultiAndPostList(pls.begin(), pls.end(),
					      matcher, db_size));

    if (not_ctx) {
	PostList* rhs = not_ctx->postlist();
	pl.reset(new AndNotPostList(pl.release(), rhs, matcher, db_size));
	not_ctx.reset();
    }

    // Sort the positional filters to try to apply them in an efficient order.
    // FIXME: We need to figure out what that is!  Try applying lowest cf/tf
    // first?

    // Apply any positional filters.
    for (const PosFilter& filter : pos_filters) {
	pl.reset(filter.postlist(pl.release(), pls));
    }

    // Empty pls so our destructor doesn't delete them all!
    pls.clear();

    if (maybe_ctx) {
	PostList* rhs = maybe_ctx->postlist();
	pl.reset(new AndMaybePostList(pl.release(), rhs, matcher, db_size));
	maybe_ctx.reset();
    }

    return pl.release();
}

}

Query::Internal::~Internal() { }

size_t
Query::Internal::get_num_subqueries() const XAPIAN_NOEXCEPT
{
    return 0;
}

const Query
Query::Internal::get_subquery(size_t) const
{
    throw Xapian::InvalidArgumentError("get_subquery() not meaningful for this Query object");
}

void
Query::Internal::gather_terms(void *) const
{
}

Xapian::termcount
Query::Internal::get_length() const XAPIAN_NOEXCEPT
{
    return 0;
}

Query::Internal *
Query::Internal::unserialise(const char ** p, const char * end,
			     const Registry & reg)
{
    if (*p == end)
	return NULL;
    unsigned char ch = *(*p)++;
    switch (ch >> 5) {
	case 4: case 5: case 6: case 7: {
	    // Multi-way branch
	    //
	    // 1ccccnnn where:
	    //   nnn -> n_subqs (0 means encoded value follows)
	    //   cccc -> code (which OP_XXX)
	    size_t n_subqs = ch & 0x07;
	    if (n_subqs == 0) {
		decode_length(p, end, n_subqs);
		n_subqs += 8;
	    }
	    unsigned char code = (ch >> 3) & 0x0f;
	    Xapian::termcount parameter = 0;
	    if (code >= 13)
		decode_length(p, end, parameter);
	    Xapian::Internal::QueryBranch * result;
	    switch (code) {
		case 0: // OP_AND
		    result = new Xapian::Internal::QueryAnd(n_subqs);
		    break;
		case 1: // OP_OR
		    result = new Xapian::Internal::QueryOr(n_subqs);
		    break;
		case 2: // OP_AND_NOT
		    result = new Xapian::Internal::QueryAndNot(n_subqs);
		    break;
		case 3: // OP_XOR
		    result = new Xapian::Internal::QueryXor(n_subqs);
		    break;
		case 4: // OP_AND_MAYBE
		    result = new Xapian::Internal::QueryAndMaybe(n_subqs);
		    break;
		case 5: // OP_FILTER
		    result = new Xapian::Internal::QueryFilter(n_subqs);
		    break;
		case 6: // OP_SYNONYM
		    result = new Xapian::Internal::QuerySynonym(n_subqs);
		    break;
		case 7: // OP_MAX
		    result = new Xapian::Internal::QueryMax(n_subqs);
		    break;
		case 13: // OP_ELITE_SET
		    result = new Xapian::Internal::QueryEliteSet(n_subqs,
								 parameter);
		    break;
		case 14: // OP_NEAR
		    result = new Xapian::Internal::QueryNear(n_subqs,
							     parameter);
		    break;
		case 15: // OP_PHRASE
		    result = new Xapian::Internal::QueryPhrase(n_subqs,
							       parameter);
		    break;
		default:
		    // 8 to 12 are currently unused.
		    throw SerialisationError("Unknown multi-way branch Query operator");
	    }
	    do {
		result->add_subquery(Xapian::Query(unserialise(p, end, reg)));
	    } while (--n_subqs);
	    result->done();
	    return result;
	}
	case 2: case 3: { // Term
	    // Term
	    //
	    // 01ccLLLL where:
	    //   LLLL -> length (0 means encoded value follows)
	    //   cc -> code:
	    //     0: wqf = 0; pos = 0
	    //     1: wqf = 1; pos = 0
	    //     2: wqf = 1; pos -> encoded value follows
	    //     3: wqf -> encoded value follows; pos -> encoded value follows
	    size_t len = ch & 0x0f;
	    if (len == 0) {
		decode_length(p, end, len);
		len += 16;
	    }
	    if (size_t(end - *p) < len)
		throw SerialisationError("Not enough data");
	    string term(*p, len);
	    *p += len;

	    int code = ((ch >> 4) & 0x03);

	    Xapian::termcount wqf = static_cast<Xapian::termcount>(code > 0);
	    if (code == 3)
		decode_length(p, end, wqf);

	    Xapian::termpos pos = 0;
	    if (code >= 2)
		decode_length(p, end, pos);

	    return new Xapian::Internal::QueryTerm(term, wqf, pos);
	}
	case 1: {
	    // OP_VALUE_RANGE or OP_VALUE_GE or OP_VALUE_LE
	    //
	    // 001tssss where:
	    //   ssss -> slot number (15 means encoded value follows)
	    //   t -> op:
	    //     0: OP_VALUE_RANGE (or OP_VALUE_LE if begin empty)
	    //     1: OP_VALUE_GE
	    Xapian::valueno slot = ch & 15;
	    if (slot == 15) {
		decode_length(p, end, slot);
		slot += 15;
	    }
	    size_t len;
	    decode_length_and_check(p, end, len);
	    string begin(*p, len);
	    *p += len;
	    if (ch & 0x10) {
		// OP_VALUE_GE
		return new Xapian::Internal::QueryValueGE(slot, begin);
	    }

	    // OP_VALUE_RANGE
	    decode_length_and_check(p, end, len);
	    string end_(*p, len);
	    *p += len;
	    if (begin.empty()) // FIXME: is this right?
		return new Xapian::Internal::QueryValueLE(slot, end_);
	    return new Xapian::Internal::QueryValueRange(slot, begin, end_);
	}
	case 0: {
	    // Other operators
	    //
	    //   000ttttt where:
	    //     ttttt -> encodes which OP_XXX
	    switch (ch & 0x1f) {
		case 0x00: // OP_INVALID
		    return new Xapian::Internal::QueryInvalid();
		case 0x0b: { // Wildcard
		    if (*p == end)
			throw SerialisationError("not enough data");
		    Xapian::termcount max_expansion;
		    decode_length(p, end, max_expansion);
		    if (end - *p < 2)
			throw SerialisationError("not enough data");
		    int max_type = static_cast<unsigned char>(*(*p)++);
		    op combiner = static_cast<op>(*(*p)++);
		    size_t len;
		    decode_length_and_check(p, end, len);
		    string pattern(*p, len);
		    *p += len;
		    return new Xapian::Internal::QueryWildcard(pattern,
							       max_expansion,
							       max_type,
							       combiner);
		}
		case 0x0c: { // PostingSource
		    size_t len;
		    decode_length_and_check(p, end, len);
		    string name(*p, len);
		    *p += len;

		    const PostingSource * reg_source = reg.get_posting_source(name);
		    if (!reg_source) {
			string m = "PostingSource ";
			m += name;
			m += " not registered";
			throw SerialisationError(m);
		    }

		    decode_length_and_check(p, end, len);
		    PostingSource * source =
			reg_source->unserialise_with_registry(string(*p, len),
							      reg);
		    *p += len;
		    return new Xapian::Internal::QueryPostingSource(source->release());
		}
		case 0x0d: {
		    using Xapian::Internal::QueryScaleWeight;
		    double scale_factor = unserialise_double(p, end);
		    return new QueryScaleWeight(scale_factor,
						Query(unserialise(p, end, reg)));
		}
		case 0x0e: {
		    Xapian::termcount wqf;
		    Xapian::termpos pos;
		    decode_length(p, end, wqf);
		    decode_length(p, end, pos);
		    return new Xapian::Internal::QueryTerm(string(), wqf, pos);
		}
		case 0x0f:
		    return new Xapian::Internal::QueryTerm();
		default: // Others currently unused.
		    break;
	    }
	    break;
	}
    }
    string msg = "Unknown Query serialisation: ";
    msg += str(ch);
    throw SerialisationError(msg);
}

void
Query::Internal::postlist_sub_and_like(AndContext& ctx,
				       QueryOptimiser * qopt,
				       double factor) const
{
    ctx.add_postlist(postlist(qopt, factor));
}

void
Query::Internal::postlist_sub_or_like(OrContext& ctx,
				      QueryOptimiser * qopt,
				      double factor) const
{
    ctx.add_postlist(postlist(qopt, factor));
}

void
Query::Internal::postlist_sub_xor(XorContext& ctx,
				  QueryOptimiser * qopt,
				  double factor) const
{
    ctx.add_postlist(postlist(qopt, factor));
}

namespace Internal {

Query::op
QueryTerm::get_type() const XAPIAN_NOEXCEPT
{
    return term.empty() ? Query::LEAF_MATCH_ALL : Query::LEAF_TERM;
}

string
QueryTerm::get_description() const
{
    string desc;
    if (term.empty()) {
	desc = "<alldocuments>";
    } else {
	description_append(desc, term);
    }
    if (wqf != 1) {
	desc += '#';
	desc += str(wqf);
    }
    if (pos) {
	desc += '@';
	desc += str(pos);
    }
    return desc;
}

QueryPostingSource::QueryPostingSource(PostingSource * source_)
    : source(source_)
{
    if (!source_)
	throw Xapian::InvalidArgumentError("source parameter can't be NULL");
    if (source->_refs == 0) {
	// source_ isn't reference counted, so try to clone it.  If clone()
	// isn't implemented, just use the object provided and it's the
	// caller's responsibility to ensure it stays valid while in use.
	PostingSource * cloned_source = source->clone();
	if (cloned_source) source = cloned_source->release();
    }
}

Query::op
QueryPostingSource::get_type() const XAPIAN_NOEXCEPT
{
    return Query::LEAF_POSTING_SOURCE;
}

string
QueryPostingSource::get_description() const
{
    string desc = "PostingSource(";
    desc += source->get_description();
    desc += ')';
    return desc;
}

QueryScaleWeight::QueryScaleWeight(double factor, const Query & subquery_)
    : scale_factor(factor), subquery(subquery_)
{
    if (rare(scale_factor < 0.0))
	throw Xapian::InvalidArgumentError("OP_SCALE_WEIGHT requires factor >= 0");
}

Query::op
QueryScaleWeight::get_type() const XAPIAN_NOEXCEPT
{
    return Query::OP_SCALE_WEIGHT;
}

size_t
QueryScaleWeight::get_num_subqueries() const XAPIAN_NOEXCEPT
{
    return 1;
}

const Query
QueryScaleWeight::get_subquery(size_t) const
{
    return subquery;
}

string
QueryScaleWeight::get_description() const
{
    Assert(subquery.internal.get());
    string desc = str(scale_factor);
    desc += " * ";
    desc += subquery.internal->get_description();
    return desc;
}

PostingIterator::Internal *
QueryTerm::postlist(QueryOptimiser * qopt, double factor) const
{
    LOGCALL(QUERY, PostingIterator::Internal *, "QueryTerm::postlist", qopt | factor);
    if (factor != 0.0)
	qopt->inc_total_subqs();
    RETURN(qopt->open_post_list(term, wqf, factor));
}

PostingIterator::Internal *
QueryPostingSource::postlist(QueryOptimiser * qopt, double factor) const
{
    LOGCALL(QUERY, PostingIterator::Internal *, "QueryPostingSource::postlist", qopt | factor);
    Assert(source.get());
    if (factor != 0.0)
	qopt->inc_total_subqs();
    // Casting away const on the Database::Internal here is OK, as we wrap
    // them in a const Xapian::Database so non-const methods can't actually
    // be called on the Database::Internal object.
    const Xapian::Database wrappeddb(
	    const_cast<Xapian::Database::Internal*>(&(qopt->db)));
    RETURN(new ExternalPostList(wrappeddb, source.get(), factor,
				qopt->matcher,
				qopt->shard_index));
}

PostingIterator::Internal *
QueryScaleWeight::postlist(QueryOptimiser * qopt, double factor) const
{
    LOGCALL(QUERY, PostingIterator::Internal *, "QueryScaleWeight::postlist", qopt | factor);
    RETURN(subquery.internal->postlist(qopt, factor * scale_factor));
}

void
QueryTerm::gather_terms(void * void_terms) const
{
    // Skip Xapian::Query::MatchAll (aka Xapian::Query("")).
    if (!term.empty()) {
	vector<pair<Xapian::termpos, string>> &terms =
	    *static_cast<vector<pair<Xapian::termpos, string>>*>(void_terms);
	terms.push_back(make_pair(pos, term));
    }
}

PostingIterator::Internal *
QueryValueRange::postlist(QueryOptimiser *qopt, double factor) const
{
    LOGCALL(QUERY, PostingIterator::Internal *, "QueryValueRange::postlist", qopt | factor);
    if (factor != 0.0)
	qopt->inc_total_subqs();
    const Xapian::Database::Internal & db = qopt->db;
    const string & lb = db.get_value_lower_bound(slot);
    if (lb.empty()) {
	// This should only happen if there are no values in this slot (which
	// could be because the backend just doesn't support values at all).
	// If there were values in the slot, the backend should have a
	// non-empty lower bound, even if it isn't a tight one.
	AssertEq(db.get_value_freq(slot), 0);
	RETURN(new EmptyPostList);
    }
    if (end < lb) {
	RETURN(new EmptyPostList);
    }
    const string & ub = db.get_value_upper_bound(slot);
    if (begin > ub) {
	RETURN(new EmptyPostList);
    }
    if (end >= ub) {
	if (begin <= lb) {
	    // The range check isn't needed, but we do still need to consider
	    // which documents have a value set in this slot.  If this value is
	    // set for all documents, we can replace it with the MatchAll
	    // postlist, which is especially efficient if there are no gaps in
	    // the docids.
	    if (db.get_value_freq(slot) == qopt->db_size) {
		RETURN(db.open_post_list(string()));
	    }
	    // Otherwise we can at least replace the lower bound with an empty
	    // string for a small efficiency gain.
	    RETURN(new ValueGePostList(&db, slot, string()));
	}
	RETURN(new ValueGePostList(&db, slot, begin));
    }
    RETURN(new ValueRangePostList(&db, slot, begin, end));
}

void
QueryValueRange::serialise(string & result) const
{
    if (slot < 15) {
	result += static_cast<char>(0x20 | slot);
    } else {
	result += static_cast<char>(0x20 | 15);
	result += encode_length(slot - 15);
    }
    result += encode_length(begin.size());
    result += begin;
    result += encode_length(end.size());
    result += end;
}

Query::op
QueryValueRange::get_type() const XAPIAN_NOEXCEPT
{
    return Query::OP_VALUE_RANGE;
}

string
QueryValueRange::get_description() const
{
    string desc = "VALUE_RANGE ";
    desc += str(slot);
    desc += ' ';
    description_append(desc, begin);
    desc += ' ';
    description_append(desc, end);
    return desc;
}

PostingIterator::Internal *
QueryValueLE::postlist(QueryOptimiser *qopt, double factor) const
{
    LOGCALL(QUERY, PostingIterator::Internal *, "QueryValueLE::postlist", qopt | factor);
    if (factor != 0.0)
	qopt->inc_total_subqs();
    const Xapian::Database::Internal & db = qopt->db;
    const string & lb = db.get_value_lower_bound(slot);
    if (lb.empty()) {
	// This should only happen if there are no values in this slot (which
	// could be because the backend just doesn't support values at all).
	// If there were values in the slot, the backend should have a
	// non-empty lower bound, even if it isn't a tight one.
	AssertEq(db.get_value_freq(slot), 0);
	RETURN(new EmptyPostList);
    }
    if (limit < lb) {
	RETURN(new EmptyPostList);
    }
    if (limit >= db.get_value_upper_bound(slot)) {
	// The range check isn't needed, but we do still need to consider
	// which documents have a value set in this slot.  If this value is
	// set for all documents, we can replace it with the MatchAll
	// postlist, which is especially efficient if there are no gaps in
	// the docids.
	if (db.get_value_freq(slot) == qopt->db_size) {
	    RETURN(db.open_post_list(string()));
	}
    }
    RETURN(new ValueRangePostList(&db, slot, string(), limit));
}

void
QueryValueLE::serialise(string & result) const
{
    // Encode as a range with an empty start (which only takes a single byte to
    // encode).
    if (slot < 15) {
	result += static_cast<char>(0x20 | slot);
    } else {
	result += static_cast<char>(0x20 | 15);
	result += encode_length(slot - 15);
    }
    result += encode_length(0);
    result += encode_length(limit.size());
    result += limit;
}

Query::op
QueryValueLE::get_type() const XAPIAN_NOEXCEPT
{
    return Query::OP_VALUE_LE;
}

string
QueryValueLE::get_description() const
{
    string desc = "VALUE_LE ";
    desc += str(slot);
    desc += ' ';
    description_append(desc, limit);
    return desc;
}

PostingIterator::Internal *
QueryValueGE::postlist(QueryOptimiser *qopt, double factor) const
{
    LOGCALL(QUERY, PostingIterator::Internal *, "QueryValueGE::postlist", qopt | factor);
    if (factor != 0.0)
	qopt->inc_total_subqs();
    const Xapian::Database::Internal & db = qopt->db;
    const string & lb = db.get_value_lower_bound(slot);
    if (lb.empty()) {
	// This should only happen if there are no values in this slot (which
	// could be because the backend just doesn't support values at all).
	// If there were values in the slot, the backend should have a
	// non-empty lower bound, even if it isn't a tight one.
	AssertEq(db.get_value_freq(slot), 0);
	RETURN(new EmptyPostList);
    }
    if (limit > db.get_value_upper_bound(slot)) {
	RETURN(new EmptyPostList);
    }
    if (limit <= lb) {
	// The range check isn't needed, but we do still need to consider
	// which documents have a value set in this slot.  If this value is
	// set for all documents, we can replace it with the MatchAll
	// postlist, which is especially efficient if there are no gaps in
	// the docids.
	if (db.get_value_freq(slot) == qopt->db_size) {
	    RETURN(db.open_post_list(string()));
	}
    }
    RETURN(new ValueGePostList(&db, slot, limit));
}

void
QueryValueGE::serialise(string & result) const
{
    if (slot < 15) {
	result += static_cast<char>(0x20 | 0x10 | slot);
    } else {
	result += static_cast<char>(0x20 | 0x10 | 15);
	result += encode_length(slot - 15);
    }
    result += encode_length(limit.size());
    result += limit;
}

Query::op
QueryValueGE::get_type() const XAPIAN_NOEXCEPT
{
    return Query::OP_VALUE_GE;
}

string
QueryValueGE::get_description() const
{
    string desc = "VALUE_GE ";
    desc += str(slot);
    desc += ' ';
    description_append(desc, limit);
    return desc;
}

PostingIterator::Internal *
QueryWildcard::postlist(QueryOptimiser * qopt, double factor) const
{
    LOGCALL(QUERY, PostingIterator::Internal *, "QueryWildcard::postlist", qopt | factor);
    Query::op op = combiner;
    double or_factor = 0.0;
    if (factor == 0.0) {
	// If we have a factor of 0, we don't care about the weights, so
	// we're just like a normal OR query.
	op = Query::OP_OR;
    } else if (op != Query::OP_SYNONYM) {
	or_factor = factor;
    }

    bool old_in_synonym = qopt->in_synonym;
    if (!old_in_synonym) {
	qopt->in_synonym = (op == Query::OP_SYNONYM);
    }

    OrContext ctx(qopt, 0);
    AutoPtr<TermList> t(qopt->db.open_allterms(pattern));
    Xapian::termcount expansions_left = max_expansion;
    // If there's no expansion limit, set expansions_left to the maximum
    // value Xapian::termcount can hold.
    if (expansions_left == 0)
	--expansions_left;
    while (true) {
	t->next();
	if (t->at_end())
	    break;
	if (max_type < Xapian::Query::WILDCARD_LIMIT_MOST_FREQUENT) {
	    if (expansions_left-- == 0) {
		if (max_type == Xapian::Query::WILDCARD_LIMIT_FIRST)
		    break;
		string msg("Wildcard ");
		msg += pattern;
		msg += "* expands to more than ";
		msg += str(max_expansion);
		msg += " terms";
		throw Xapian::WildcardError(msg);
	    }
	}
	const string & term = t->get_termname();
	ctx.add_postlist(qopt->open_lazy_post_list(term, 1, or_factor));
    }

    if (max_type == Xapian::Query::WILDCARD_LIMIT_MOST_FREQUENT) {
	// FIXME: open_lazy_post_list() results in the term getting registered
	// for stats, so we still incur an avoidable cost from the full
	// expansion size of the wildcard, which is most likely to be visible
	// with the remote backend.  Perhaps we should split creating the lazy
	// postlist from registering the term for stats.
	if (ctx.size() > max_expansion)
	    ctx.select_most_frequent(max_expansion);
    }

    if (factor != 0.0) {
	if (op != Query::OP_SYNONYM) {
	    qopt->set_total_subqs(qopt->get_total_subqs() + ctx.size());
	} else {
	    qopt->inc_total_subqs();
	}
    }

    qopt->in_synonym = old_in_synonym;

    if (ctx.empty())
	RETURN(new EmptyPostList);

    if (op == Query::OP_MAX)
	RETURN(ctx.postlist_max());

    PostList * pl = ctx.postlist();
    if (op == Query::OP_OR)
	RETURN(pl);

    // We build an OP_OR tree for OP_SYNONYM and then wrap it in a
    // SynonymPostList, which supplies the weights.
    //
    // We know the subqueries from a wildcard expansion are wdf-disjoint
    // (i.e. each wdf from the document contributes at most itself to the
    // wdf of the subquery).
    RETURN(qopt->make_synonym_postlist(pl, factor, true));
}

termcount
QueryWildcard::get_length() const XAPIAN_NOEXCEPT
{
    // We currently assume wqf is 1 for calculating the synonym's weight
    // since conceptually the synonym is one "virtual" term.  If we were
    // to combine multiple occurrences of the same synonym expansion into
    // a single instance with wqf set, we would want to track the wqf.
    return 1;
}

void
QueryWildcard::serialise(string & result) const
{
    result += static_cast<char>(0x0b);
    result += encode_length(max_expansion);
    result += static_cast<unsigned char>(max_type);
    result += static_cast<unsigned char>(combiner);
    result += encode_length(pattern.size());
    result += pattern;
}

Query::op
QueryWildcard::get_type() const XAPIAN_NOEXCEPT
{
    return Query::OP_WILDCARD;
}

string
QueryWildcard::get_description() const
{
    string desc = "WILDCARD ";
    switch (combiner) {
	case Query::OP_SYNONYM:
	    desc += "SYNONYM ";
	    break;
	case Query::OP_MAX:
	    desc += "MAX ";
	    break;
	case Query::OP_OR:
	    desc += "OR ";
	    break;
	default:
	    desc += "BAD ";
	    break;
    }
    description_append(desc, pattern);
    return desc;
}

Xapian::termcount
QueryBranch::get_length() const XAPIAN_NOEXCEPT
{
    // Sum results from all subqueries.
    Xapian::termcount result = 0;
    QueryVector::const_iterator i;
    for (i = subqueries.begin(); i != subqueries.end(); ++i) {
	// MatchNothing subqueries should have been removed by done(), but we
	// can't use Assert in a XAPIAN_NOEXCEPT function.  But we'll get a
	// segfault anyway.
	result += (*i).internal->get_length();
    }
    return result;
}

#define MULTIWAY(X) static_cast<unsigned char>(0x80 | (X) << 3)
#define MISC(X) static_cast<unsigned char>(X)
void
QueryBranch::serialise_(string & result, Xapian::termcount parameter) const
{
    static const unsigned char first_byte[] = {
	MULTIWAY(0),	// OP_AND
	MULTIWAY(1),	// OP_OR
	MULTIWAY(2),	// OP_AND_NOT
	MULTIWAY(3),	// OP_XOR
	MULTIWAY(4),	// OP_AND_MAYBE
	MULTIWAY(5),	// OP_FILTER
	MULTIWAY(14),	// OP_NEAR
	MULTIWAY(15),	// OP_PHRASE
	0,		// OP_VALUE_RANGE
	MISC(3),	// OP_SCALE_WEIGHT
	MULTIWAY(13),	// OP_ELITE_SET
	0,		// OP_VALUE_GE
	0,		// OP_VALUE_LE
	MULTIWAY(6),	// OP_SYNONYM
	MULTIWAY(7)	// OP_MAX
    };
    Xapian::Query::op op_ = get_op();
    AssertRel(size_t(op_),<,sizeof(first_byte));
    unsigned char ch = first_byte[op_];
    if (ch & 0x80) {
	// Multi-way operator.
	if (subqueries.size() < 8)
	    ch |= subqueries.size();
	result += ch;
	if (subqueries.size() >= 8)
	    result += encode_length(subqueries.size() - 8);
	if (ch >= MULTIWAY(13))
	    result += encode_length(parameter);
    } else {
	result += ch;
    }

    QueryVector::const_iterator i;
    for (i = subqueries.begin(); i != subqueries.end(); ++i) {
	// MatchNothing subqueries should have been removed by done().
	Assert((*i).internal.get());
	(*i).internal->serialise(result);
    }

    // For OP_NEAR, OP_PHRASE, and OP_ELITE_SET, the window/set size gets
    // appended next by an overloaded serialise() method in the subclass.
}

void
QueryBranch::serialise(string & result) const
{
    QueryBranch::serialise_(result);
}

void
QueryNear::serialise(string & result) const
{
    // FIXME: window - subqueries.size() ?
    QueryBranch::serialise_(result, window);
}

void
QueryPhrase::serialise(string & result) const
{
    // FIXME: window - subqueries.size() ?
    QueryBranch::serialise_(result, window);
}

void
QueryEliteSet::serialise(string & result) const
{
    // FIXME: set_size - subqueries.size() ?
    QueryBranch::serialise_(result, set_size);
}

void
QueryBranch::gather_terms(void * void_terms) const
{
    // Gather results from all subqueries.
    QueryVector::const_iterator i;
    for (i = subqueries.begin(); i != subqueries.end(); ++i) {
	// MatchNothing subqueries should have been removed by done().
	Assert((*i).internal.get());
	(*i).internal->gather_terms(void_terms);
    }
}

void
QueryBranch::do_or_like(OrContext& ctx, QueryOptimiser * qopt, double factor,
			Xapian::termcount elite_set_size, size_t first) const
{
    LOGCALL_VOID(MATCH, "QueryBranch::do_or_like", ctx | qopt | factor | elite_set_size);

    // FIXME: we could optimise by merging OP_ELITE_SET and OP_OR like we do
    // for AND-like operations.

    // OP_SYNONYM with a single subquery is only simplified by
    // QuerySynonym::done() if the single subquery is a term or MatchAll.
    Assert(subqueries.size() >= 2 || get_op() == Query::OP_SYNONYM);

    size_t size_before = ctx.size();
    QueryVector::const_iterator q;
    for (q = subqueries.begin() + first; q != subqueries.end(); ++q) {
	// MatchNothing subqueries should have been removed by done().
	Assert((*q).internal.get());
	(*q).internal->postlist_sub_or_like(ctx, qopt, factor);
    }

    size_t out_of = ctx.size() - size_before;
    if (elite_set_size && elite_set_size < out_of) {
	ctx.select_elite_set(elite_set_size, out_of);
	// FIXME: This isn't quite right as we flatten ORs under the ELITE_SET
	// and then pick from amongst all the subqueries.  Consider:
	//
	// Query subqs[] = {q1 | q2, q3 | q4};
	// Query q(OP_ELITE_SET, begin(subqs), end(subqs), 1);
	//
	// Here q should be either q1 | q2 or q3 | q4, but actually it'll be
	// just one of q1 or q2 or q3 or q4 (assuming those aren't themselves
	// OP_OR or OP_OR-like queries).
    }
}

PostList *
QueryBranch::do_synonym(QueryOptimiser * qopt, double factor) const
{
    LOGCALL(MATCH, PostList *, "QueryBranch::do_synonym", qopt | factor);
    OrContext ctx(qopt, subqueries.size());
    if (factor == 0.0) {
	// If we have a factor of 0, we don't care about the weights, so
	// we're just like a normal OR query.
	do_or_like(ctx, qopt, 0.0);
	return ctx.postlist();
    }

    bool old_in_synonym = qopt->in_synonym;
    qopt->in_synonym = true;
    do_or_like(ctx, qopt, 0.0);
    PostList * pl = ctx.postlist();
    qopt->in_synonym = old_in_synonym;

    bool wdf_disjoint = false;
    Assert(!subqueries.empty());
    auto type = (*subqueries.begin()).get_type();
    if (type == Query::OP_WILDCARD) {
	// Detect common easy case where all subqueries are OP_WILDCARD whose
	// constant prefixes form a prefix-free set.
	wdf_disjoint = true;
	vector<string> prefixes;
	for (auto&& q : subqueries) {
	    if (q.get_type() != Query::OP_WILDCARD) {
		wdf_disjoint = false;
		break;
	    }
	    auto qw = static_cast<const QueryWildcard*>(q.internal.get());
	    prefixes.push_back(qw->get_pattern());
	}

	if (wdf_disjoint) {
	    sort(prefixes.begin(), prefixes.end());
	    const string* prev = nullptr;
	    for (const auto& i : prefixes) {
		if (prev) {
		    if (startswith(i, *prev)) {
			wdf_disjoint = false;
			break;
		    }
		}
		prev = &i;
	    }
	}
    } else if (type == Query::LEAF_TERM) {
	// Detect common easy case where all subqueries are terms, none of
	// which are the same.
	wdf_disjoint = true;
	unordered_set<string> terms;
	for (auto&& q : subqueries) {
	    if (q.get_type() != Query::LEAF_TERM) {
		wdf_disjoint = false;
		break;
	    }
	    auto qt = static_cast<const QueryTerm*>(q.internal.get());
	    if (!terms.insert(qt->get_term()).second) {
		wdf_disjoint = false;
		break;
	    }
	}
    }

    // We currently assume wqf is 1 for calculating the synonym's weight
    // since conceptually the synonym is one "virtual" term.  If we were
    // to combine multiple occurrences of the same synonym expansion into
    // a single instance with wqf set, we would want to track the wqf.

    // We build an OP_OR tree for OP_SYNONYM and then wrap it in a
    // SynonymPostList, which supplies the weights.
    RETURN(qopt->make_synonym_postlist(pl, factor, wdf_disjoint));
}

PostList *
QueryBranch::do_max(QueryOptimiser * qopt, double factor) const
{
    LOGCALL(MATCH, PostList *, "QueryBranch::do_max", qopt | factor);
    OrContext ctx(qopt, subqueries.size());
    do_or_like(ctx, qopt, factor);
    if (factor == 0.0) {
	// If we have a factor of 0, we don't care about the weights, so
	// we're just like a normal OR query.
	RETURN(ctx.postlist());
    }

    // We currently assume wqf is 1 for calculating the OP_MAX's weight
    // since conceptually the OP_MAX is one "virtual" term.  If we were
    // to combine multiple occurrences of the same OP_MAX expansion into
    // a single instance with wqf set, we would want to track the wqf.
    RETURN(ctx.postlist_max());
}

Xapian::Query::op
QueryBranch::get_type() const XAPIAN_NOEXCEPT
{
    return get_op();
}

size_t
QueryBranch::get_num_subqueries() const XAPIAN_NOEXCEPT
{
    return subqueries.size();
}

const Query
QueryBranch::get_subquery(size_t n) const
{
    return subqueries[n];
}

const string
QueryBranch::get_description_helper(const char * op,
				    Xapian::termcount parameter) const
{
    string desc = "(";
    QueryVector::const_iterator i;
    for (i = subqueries.begin(); i != subqueries.end(); ++i) {
	if (desc.size() > 1) {
	    desc += op;
	    if (parameter) {
		desc += str(parameter);
		desc += ' ';
	    }
	}
	Assert((*i).internal.get());
	// MatchNothing subqueries should have been removed by done(), and we
	// shouldn't get called before done() is, since that happens at the
	// end of the Xapian::Query constructor.
	desc += (*i).internal->get_description();
    }
    desc += ')';
    return desc;
}

Query::Internal *
QueryWindowed::done()
{
    // If window size not specified, default it.
    if (window == 0)
	window = subqueries.size();
    return QueryAndLike::done();
}

void
QueryScaleWeight::gather_terms(void * void_terms) const
{
    subquery.internal->gather_terms(void_terms);
}

void QueryTerm::serialise(string & result) const
{
    size_t len = term.size();
    if (len == 0) {
	if (wqf == 1 && pos == 0) {
	    // Query::MatchAll
	    result += '\x0f';
	} else {
	    // Weird mutant versions of MatchAll
	    result += '\x0e';
	    result += encode_length(wqf);
	    result += encode_length(pos);
	}
    } else if (wqf == 1) {
	if (pos == 0) {
	    // Single occurrence free-text term without position set.
	    if (len >= 16) {
		result += static_cast<char>(0x40 | 0x10);
		result += encode_length(term.size() - 16);
	    } else {
		result += static_cast<char>(0x40 | 0x10 | len);
	    }
	    result += term;
	} else {
	    // Single occurrence free-text term with position set.
	    if (len >= 16) {
		result += static_cast<char>(0x40 | 0x20);
		result += encode_length(term.size() - 16);
	    } else {
		result += static_cast<char>(0x40 | 0x20 | len);
	    }
	    result += term;
	    result += encode_length(pos);
	}
    } else if (wqf > 1 || pos > 0) {
	// General case.
	if (len >= 16) {
	    result += static_cast<char>(0x40 | 0x30);
	    result += encode_length(term.size() - 16);
	} else if (len) {
	    result += static_cast<char>(0x40 | 0x30 | len);
	}
	result += term;
	result += encode_length(wqf);
	result += encode_length(pos);
    } else {
	// Typical boolean term.
	AssertEq(wqf, 0);
	AssertEq(pos, 0);
	if (len >= 16) {
	    result += static_cast<char>(0x40);
	    result += encode_length(term.size() - 16);
	} else {
	    result += static_cast<char>(0x40 | len);
	}
	result += term;
    }
}

void QueryPostingSource::serialise(string & result) const
{
    result += static_cast<char>(0x0c);

    const string & n = source->name();
    result += encode_length(n.size());
    result += n;

    const string & s = source->serialise();
    result += encode_length(s.size());
    result += s;
}

void QueryScaleWeight::serialise(string & result) const
{
    Assert(subquery.internal.get());
    result += '\x0d';
    result += serialise_double(scale_factor);
    subquery.internal->serialise(result);
}

void
QueryAndLike::add_subquery(const Xapian::Query & subquery)
{
    // If the AndLike is already MatchNothing, do nothing.
    if (subqueries.size() == 1 && subqueries[0].internal.get() == NULL)
	return;
    // If we're adding MatchNothing, discard any previous subqueries.
    if (subquery.internal.get() == NULL)
	subqueries.clear();
    subqueries.push_back(subquery);
}

Query::Internal *
QueryAndLike::done()
{
    // Empty AndLike gives MatchNothing.
    if (subqueries.empty())
	return NULL;
    // We handle any subquery being MatchNothing in add_subquery() by leaving
    // a single MatchNothing subquery, and so this check results in AndLike
    // giving MatchNothing.
    if (subqueries.size() == 1)
	return subqueries[0].internal.get();
    return this;
}

PostingIterator::Internal *
QueryAndLike::postlist(QueryOptimiser * qopt, double factor) const
{
    LOGCALL(QUERY, PostingIterator::Internal *, "QueryAndLike::postlist", qopt | factor);
    AndContext ctx(qopt, subqueries.size());
    postlist_sub_and_like(ctx, qopt, factor);
    RETURN(ctx.postlist());
}

void
QueryAndLike::postlist_sub_and_like(AndContext& ctx, QueryOptimiser * qopt, double factor) const
{
    QueryVector::const_iterator i;
    for (i = subqueries.begin(); i != subqueries.end(); ++i) {
	// MatchNothing subqueries should have been removed by done().
	Assert((*i).internal.get());
	(*i).internal->postlist_sub_and_like(ctx, qopt, factor);
    }
}

void
QueryOrLike::add_subquery(const Xapian::Query & subquery)
{
    // Drop any subqueries which are MatchNothing.
    if (subquery.internal.get() != NULL)
	subqueries.push_back(subquery);
}

Query::Internal *
QueryOrLike::done()
{
    // An empty OrLike gives MatchNothing.  Note that add_subquery() drops any
    // subqueries which are MatchNothing.
    if (subqueries.empty())
	return NULL;
    if (subqueries.size() == 1)
	return subqueries[0].internal.get();
    return this;
}

void
QueryAndNot::add_subquery(const Xapian::Query & subquery)
{
    if (!subqueries.empty()) {
	// We're adding the 2nd or subsequent subquery, so this subquery is
	// negated.
	if (subqueries[0].internal.get() == NULL) {
	    // The left side is already MatchNothing so drop any right side.
	    //
	    // MatchNothing AND_NOT X == MatchNothing
	    return;
	}
	if (subquery.internal.get() == NULL) {
	    // Drop MatchNothing on the right of AndNot.
	    //
	    // X AND_NOT MatchNothing == X
	    return;
	}
	if (subquery.get_type() == subquery.OP_SCALE_WEIGHT) {
	    // Strip OP_SCALE_WEIGHT wrapping from queries on the right of
	    // AndNot as no weight is taken from them.
	    subqueries.push_back(subquery.get_subquery(0));
	    // The Query constructor for OP_SCALE_WEIGHT constructor should
	    // eliminate OP_SCALE_WEIGHT applied to MatchNothing.
	    Assert(subquery.get_subquery(0).internal.get() != NULL);
	    return;
	}
    }
    subqueries.push_back(subquery);
}

Query::Internal *
QueryAndNot::done()
{
    // Any MatchNothing right subqueries get discarded by add_subquery() - if
    // that leaves just the left subquery, return that.
    //
    // If left subquery is MatchNothing, then add_subquery() discards all right
    // subqueries, so this check also gives MatchNothing for this case.
    if (subqueries.size() == 1)
	return subqueries[0].internal.get();
    return this;
}

void
QueryAndMaybe::add_subquery(const Xapian::Query & subquery)
{
    // If the left side of AndMaybe is already MatchNothing, do nothing.
    if (subqueries.size() == 1 && subqueries[0].internal.get() == NULL)
	return;
    // Drop any 2nd or subsequent subqueries which are MatchNothing.
    if (subquery.internal.get() != NULL || subqueries.empty())
	subqueries.push_back(subquery);
}

Query::Internal *
QueryAndMaybe::done()
{
    // Any MatchNothing right subqueries get discarded by add_subquery() - if
    // that leaves just the left subquery, return that.
    //
    // If left subquery is MatchNothing, then add_subquery() discards all right
    // subqueries, so this check also gives MatchNothing for this case.
    if (subqueries.size() == 1)
	return subqueries[0].internal.get();
    return this;
}

PostingIterator::Internal *
QueryOr::postlist(QueryOptimiser * qopt, double factor) const
{
    LOGCALL(QUERY, PostingIterator::Internal *, "QueryOr::postlist", qopt | factor);
    OrContext ctx(qopt, subqueries.size());
    do_or_like(ctx, qopt, factor);
    RETURN(ctx.postlist());
}

void
QueryOr::postlist_sub_or_like(OrContext& ctx, QueryOptimiser * qopt, double factor) const
{
    do_or_like(ctx, qopt, factor);
}

PostingIterator::Internal *
QueryAndNot::postlist(QueryOptimiser * qopt, double factor) const
{
    LOGCALL(QUERY, PostingIterator::Internal *, "QueryAndNot::postlist", qopt | factor);
    AutoPtr<PostList> l(subqueries[0].internal->postlist(qopt, factor));
    OrContext ctx(qopt, subqueries.size() - 1);
    do_or_like(ctx, qopt, 0.0, 0, 1);
    AutoPtr<PostList> r(ctx.postlist());
    RETURN(new AndNotPostList(l.release(), r.release(),
			      qopt->matcher, qopt->db_size));
}

void
QueryAndNot::postlist_sub_and_like(AndContext& ctx,
				   QueryOptimiser* qopt,
				   double factor) const
{
    subqueries[0].internal->postlist_sub_and_like(ctx, qopt, factor);
    do_or_like(ctx.get_not_ctx(subqueries.size() - 1), qopt, 0.0, 0, 1);
}

PostingIterator::Internal *
QueryXor::postlist(QueryOptimiser * qopt, double factor) const
{
    LOGCALL(QUERY, PostingIterator::Internal *, "QueryXor::postlist", qopt | factor);
    XorContext ctx(qopt, subqueries.size());
    postlist_sub_xor(ctx, qopt, factor);
    RETURN(ctx.postlist());
}

void
QueryXor::postlist_sub_xor(XorContext& ctx, QueryOptimiser * qopt, double factor) const
{
    QueryVector::const_iterator i;
    for (i = subqueries.begin(); i != subqueries.end(); ++i) {
	// MatchNothing subqueries should have been removed by done().
	Assert((*i).internal.get());
	(*i).internal->postlist_sub_xor(ctx, qopt, factor);
    }
}

PostingIterator::Internal *
QueryAndMaybe::postlist(QueryOptimiser * qopt, double factor) const
{
    LOGCALL(QUERY, PostingIterator::Internal *, "QueryAndMaybe::postlist", qopt | factor);
    // FIXME: Combine and-like side with and-like stuff above.
    AutoPtr<PostList> l(subqueries[0].internal->postlist(qopt, factor));
    if (factor == 0.0) {
	// An unweighted OP_AND_MAYBE can be replaced with its left branch.
	RETURN(l.release());
    }
    OrContext ctx(qopt, subqueries.size() - 1);
    do_or_like(ctx, qopt, factor, 0, 1);
    AutoPtr<PostList> r(ctx.postlist());
    RETURN(new AndMaybePostList(l.release(), r.release(),
				qopt->matcher, qopt->db_size));
}

void
QueryAndMaybe::postlist_sub_and_like(AndContext& ctx,
				     QueryOptimiser* qopt,
				     double factor) const
{
    subqueries[0].internal->postlist_sub_and_like(ctx, qopt, factor);
    do_or_like(ctx.get_maybe_ctx(subqueries.size() - 1), qopt, factor, 0, 1);
}

PostingIterator::Internal *
QueryFilter::postlist(QueryOptimiser * qopt, double factor) const
{
    LOGCALL(QUERY, PostingIterator::Internal *, "QueryFilter::postlist", qopt | factor);
    AndContext ctx(qopt, subqueries.size());
    QueryFilter::postlist_sub_and_like(ctx, qopt, factor);
    RETURN(ctx.postlist());
}

void
QueryFilter::postlist_sub_and_like(AndContext& ctx, QueryOptimiser * qopt, double factor) const
{
    QueryVector::const_iterator i;
    for (i = subqueries.begin(); i != subqueries.end(); ++i) {
	// MatchNothing subqueries should have been removed by done().
	Assert((*i).internal.get());
	(*i).internal->postlist_sub_and_like(ctx, qopt, factor);
	// Second and subsequent subqueries are unweighted.
	factor = 0.0;
    }
}

void
QueryWindowed::postlist_windowed(Query::op op, AndContext& ctx, QueryOptimiser * qopt, double factor) const
{
    if (!qopt->full_db_has_positions()) {
	// No positional data anywhere, so just handle as AND.
	QueryAndLike::postlist_sub_and_like(ctx, qopt, factor);
	return;
    }

    if (!qopt->db.has_positions()) {
	// No positions in this subdatabase so this matches nothing, which
	// means the whole andcontext matches nothing.
	//
	// Bailing out here means we don't recurse deeper and that means we
	// don't call QueryOptimiser::inc_total_subqs() for leaf postlists in
	// the phrase, but at least one shard will count them, and the matcher
	// takes the highest answer (since 1.4.6).
	ctx.shrink(0);
	return;
    }

    bool old_need_positions = qopt->need_positions;
    qopt->need_positions = true;

    QueryVector::const_iterator i;
    for (i = subqueries.begin(); i != subqueries.end(); ++i) {
	// MatchNothing subqueries should have been removed by done().
	Assert((*i).internal.get());
	bool is_term = ((*i).internal->get_type() == Query::LEAF_TERM);
	PostList* pl = (*i).internal->postlist(qopt, factor);
	if (!is_term)
	    pl = new OrPosPostList(pl);
	ctx.add_postlist(pl);
    }
    // Record the positional filter to apply higher up the tree.
    ctx.add_pos_filter(op, subqueries.size(), window);

    qopt->need_positions = old_need_positions;
}

void
QueryPhrase::postlist_sub_and_like(AndContext & ctx, QueryOptimiser * qopt, double factor) const
{
    QueryWindowed::postlist_windowed(Query::OP_PHRASE, ctx, qopt, factor);
}

void
QueryNear::postlist_sub_and_like(AndContext & ctx, QueryOptimiser * qopt, double factor) const
{
    QueryWindowed::postlist_windowed(Query::OP_NEAR, ctx, qopt, factor);
}

PostingIterator::Internal *
QueryEliteSet::postlist(QueryOptimiser * qopt, double factor) const
{
    LOGCALL(QUERY, PostingIterator::Internal *, "QueryEliteSet::postlist", qopt | factor);
    OrContext ctx(qopt, subqueries.size());
    do_or_like(ctx, qopt, factor, set_size);
    RETURN(ctx.postlist());
}

void
QueryEliteSet::postlist_sub_or_like(OrContext& ctx, QueryOptimiser * qopt, double factor) const
{
    do_or_like(ctx, qopt, factor, set_size);
}

PostingIterator::Internal *
QuerySynonym::postlist(QueryOptimiser * qopt, double factor) const
{
    LOGCALL(QUERY, PostingIterator::Internal *, "QuerySynonym::postlist", qopt | factor);
    // Save and restore total_subqs so we only add one for the whole
    // OP_SYNONYM subquery (or none if we're not weighted).
    Xapian::termcount save_total_subqs = qopt->get_total_subqs();
    if (factor != 0.0)
	++save_total_subqs;
    PostList * pl = do_synonym(qopt, factor);
    qopt->set_total_subqs(save_total_subqs);
    RETURN(pl);
}

Query::Internal *
QuerySynonym::done()
{
    // An empty Synonym gives MatchNothing.  Note that add_subquery() drops any
    // subqueries which are MatchNothing.
    if (subqueries.empty())
	return NULL;
    if (subqueries.size() == 1) {
	Query::op sub_type = subqueries[0].get_type();
	// Synonym of a single subquery should only be simplified if that
	// subquery is a term (or MatchAll), or if it's also OP_SYNONYM.  Note
	// that MatchNothing subqueries are dropped, so we'd never get here
	// with a single MatchNothing subquery.
	if (sub_type == Query::LEAF_TERM || sub_type == Query::LEAF_MATCH_ALL ||
	    sub_type == Query::OP_SYNONYM) {
	    return subqueries[0].internal.get();
	}
	if (sub_type == Query::OP_WILDCARD) {
	    auto q = static_cast<QueryWildcard*>(subqueries[0].internal.get());
	    // SYNONYM over WILDCARD X -> WILDCARD SYNONYM for any combiner X.
	    return q->change_combiner(Query::OP_SYNONYM);
	}
    }
    return this;
}

PostingIterator::Internal *
QueryMax::postlist(QueryOptimiser * qopt, double factor) const
{
    LOGCALL(QUERY, PostingIterator::Internal *, "QueryMax::postlist", qopt | factor);
    // Save and restore total_subqs so we only add one for the whole
    // OP_MAX subquery (or none if we're not weighted).
    Xapian::termcount save_total_subqs = qopt->get_total_subqs();
    if (factor != 0.0)
	++save_total_subqs;
    PostList * pl = do_max(qopt, factor);
    qopt->set_total_subqs(save_total_subqs);
    RETURN(pl);
}

Xapian::Query::op
QueryAnd::get_op() const
{
    return Xapian::Query::OP_AND;
}

Xapian::Query::op
QueryOr::get_op() const
{
    return Xapian::Query::OP_OR;
}

Xapian::Query::op
QueryAndNot::get_op() const
{
    return Xapian::Query::OP_AND_NOT;
}

Xapian::Query::op
QueryXor::get_op() const
{
    return Xapian::Query::OP_XOR;
}

Xapian::Query::op
QueryAndMaybe::get_op() const
{
    return Xapian::Query::OP_AND_MAYBE;
}

Xapian::Query::op
QueryFilter::get_op() const
{
    return Xapian::Query::OP_FILTER;
}

Xapian::Query::op
QueryNear::get_op() const
{
    return Xapian::Query::OP_NEAR;
}

Xapian::Query::op
QueryPhrase::get_op() const
{
    return Xapian::Query::OP_PHRASE;
}

Xapian::Query::op
QueryEliteSet::get_op() const
{
    return Xapian::Query::OP_ELITE_SET;
}

Xapian::Query::op
QuerySynonym::get_op() const
{
    return Xapian::Query::OP_SYNONYM;
}

Xapian::Query::op
QueryMax::get_op() const
{
    return Xapian::Query::OP_MAX;
}

Xapian::Query::op
QueryWildcard::get_op() const
{
    return Xapian::Query::OP_WILDCARD;
}

string
QueryAnd::get_description() const
{
    return get_description_helper(" AND ");
}

string
QueryOr::get_description() const
{
    return get_description_helper(" OR ");
}

string
QueryAndNot::get_description() const
{
    return get_description_helper(" AND_NOT ");
}

string
QueryXor::get_description() const
{
    return get_description_helper(" XOR ");
}

string
QueryAndMaybe::get_description() const
{
    return get_description_helper(" AND_MAYBE ");
}

string
QueryFilter::get_description() const
{
    return get_description_helper(" FILTER ");
}

string
QueryNear::get_description() const
{
    return get_description_helper(" NEAR ", window);
}

string
QueryPhrase::get_description() const
{
    return get_description_helper(" PHRASE ", window);
}

string
QueryEliteSet::get_description() const
{
    return get_description_helper(" ELITE_SET ", set_size);
}

string
QuerySynonym::get_description() const
{
    if (subqueries.size() == 1) {
	string d = "(SYNONYM ";
	d += subqueries[0].internal->get_description();
	d += ")";
	return d;
    }
    return get_description_helper(" SYNONYM ");
}

string
QueryMax::get_description() const
{
    return get_description_helper(" MAX ");
}

Xapian::Query::op
QueryInvalid::get_type() const XAPIAN_NOEXCEPT
{
    return Xapian::Query::OP_INVALID;
}

PostingIterator::Internal *
QueryInvalid::postlist(QueryOptimiser *, double) const
{
    throw Xapian::InvalidOperationError("Query is invalid");
}

void
QueryInvalid::serialise(std::string & result) const
{
    result += static_cast<char>(0x00);
}

string
QueryInvalid::get_description() const
{
    return "<INVALID>";
}

}
}
