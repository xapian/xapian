/** @file
 * @brief Xapian::Query internals
 */
/* Copyright (C) 2007-2024 Olly Betts
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
#include "xapian/unicode.h"

#include "api/editdistance.h"
#include "backends/postlist.h"
#include "heap.h"
#include "matcher/andmaybepostlist.h"
#include "matcher/andnotpostlist.h"
#include "matcher/andpostlist.h"
#include "matcher/boolorpostlist.h"
#include "matcher/exactphrasepostlist.h"
#include "matcher/externalpostlist.h"
#include "matcher/maxpostlist.h"
#include "matcher/nearpostlist.h"
#include "matcher/orpospostlist.h"
#include "matcher/orpostlist.h"
#include "matcher/phrasepostlist.h"
#include "matcher/queryoptimiser.h"
#include "matcher/valuerangepostlist.h"
#include "matcher/valuegepostlist.h"
#include "matcher/xorpostlist.h"
#include "pack.h"
#include "serialise-double.h"
#include "stringutils.h"
#include "termlist.h"

#include "debuglog.h"
#include "omassert.h"
#include "str.h"
#include "stringutils.h"
#include "unicode/description_append.h"

#include <algorithm>
#include <limits>
#include <list>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

using namespace std;

static constexpr unsigned MAX_UTF_8_CHARACTER_LENGTH = 4;

using Xapian::Internal::AndContext;
using Xapian::Internal::OrContext;
using Xapian::Internal::XorContext;

namespace Xapian {

namespace Internal {

/** Class providing an operator which sorts postlists to select max or terms.
 *  This returns true if a has a (strictly) greater termweight than b.
 *
 *  Using this comparator will tend to result in multiple calls to
 *  recalc_maxweight() for each of the subqueries (we use it with nth_element
 *  which should be O(n)) - perhaps it'd be better to call recalc_maxweight()
 *  once and then sort on that.
 */
struct CmpMaxOrTerms {
    /** Return true if and only if a has a strictly greater termweight than b. */
    bool operator()(PostList* a, PostList* b) {
#if (defined(__i386__) && !defined(__SSE_MATH__)) || \
    defined(__mc68000__) || defined(__mc68010__) || \
    defined(__mc68020__) || defined(__mc68030__)
	// On some architectures, most common of which is x86, floating point
	// values are calculated and stored in registers with excess precision.
	// If the two recalc_maxweight() calls below return identical values in a
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
	volatile double a_max_wt = a->recalc_maxweight();
	volatile double b_max_wt = b->recalc_maxweight();
	return a_max_wt > b_max_wt;
#else
	return a->recalc_maxweight() > b->recalc_maxweight();
#endif
    }
};

/// Comparison functor which orders by descending termfreq.
struct ComparePostListTermFreqAscending {
    /// Order PostList* by descending get_termfreq().
    bool operator()(const PostList* a,
		    const PostList* b) const {
	return a->get_termfreq() > b->get_termfreq();
    }
};

class Context {
  protected:
    QueryOptimiser* qopt;

    vector<PostList*> pls;

    vector<TermFreqs> termfreqs_list;

    /// Lower bound on matching docid range.
    Xapian::docid first = Xapian::docid(-1);

    /// Upper bound on matching docid range.
    Xapian::docid last = 0;

  public:
    Context(QueryOptimiser* qopt_, size_t reserve) : qopt(qopt_) {
	pls.reserve(reserve);
    }

    ~Context() {
	shrink(0);
    }

    Xapian::docid get_first() const { return first; }

    Xapian::docid get_last() const { return last; }

    void add_termfreqs(TermFreqs* termfreqs) {
	if (termfreqs) termfreqs_list.emplace_back(*termfreqs);
    }

    void add_postlist(PostList* pl, TermFreqs* termfreqs) {
	add_termfreqs(termfreqs);
	if (pl) {
	    pls.emplace_back(pl);
	    Xapian::docid f = 1;
	    Xapian::docid l = Xapian::docid(-1);
	    pl->get_docid_range(f, l);
	    first = std::min(first, f);
	    last = std::max(last, l);
	}
    }

    bool empty() const {
	return pls.empty();
    }

    Xapian::termcount size() const {
	return Xapian::termcount(pls.size());
    }

    void shrink(size_t new_size) {
	AssertRel(new_size, <=, pls.size());
	if (new_size >= pls.size())
	    return;

	for (auto&& i = pls.begin() + new_size; i != pls.end(); ++i) {
	    qopt->destroy_postlist(*i);
	}
	pls.resize(new_size);
    }

    /** Expand a wildcard query. */
    void expand_wildcard(const QueryWildcard* query,
			 double factor,
			 TermFreqs* termfreqs);

    /** Expand an edit distance query. */
    void expand_edit_distance(const QueryEditDistance* query,
			      double factor,
			      TermFreqs* termfreqs);
};

inline void
Context::expand_wildcard(const QueryWildcard* query,
			 double factor,
			 TermFreqs* termfreqs)
{
    unique_ptr<TermList> t(qopt->db.open_allterms(query->get_fixed_prefix()));
    bool skip_ucase = query->get_fixed_prefix().empty();
    auto max_type = query->get_max_type();
    Xapian::termcount expansions_left = query->get_max_expansion();
    // If there's no expansion limit, set expansions_left to the maximum
    // value it can hold.
    if (expansions_left == 0)
	expansions_left = numeric_limits<decltype(expansions_left)>::max();
    while (true) {
	TermList* ret = t->next();
done_skip_to:
	if (ret) {
	    // Pruning shouldn't be possible, as this is iterating allterms for
	    // a single shard.
	    Assert(ret == t.get());
	    // End of entries.
	    break;
	}

	const string & term = t->get_termname();
	if (skip_ucase && term[0] >= 'A') {
	    // If there's a leading wildcard then skip terms that start
	    // with A-Z, as we don't want the expansion to include prefixed
	    // terms.
	    //
	    // This assumes things about the structure of terms which the
	    // Query class otherwise doesn't need to care about, but it
	    // seems hard to avoid here.
	    skip_ucase = false;
	    if (term[0] <= 'Z') {
		static_assert('Z' + 1 == '[', "'Z' + 1 == '['");
		ret = t->skip_to("[");
		goto done_skip_to;
	    }
	}

	if (!query->test_prefix_known(term)) continue;

	if (max_type < Xapian::Query::WILDCARD_LIMIT_MOST_FREQUENT) {
	    if (expansions_left == 0) {
		if (max_type == Xapian::Query::WILDCARD_LIMIT_FIRST)
		    break;
		string msg("Wildcard ");
		msg += query->get_pattern();
		if (query->get_just_flags() == 0)
		    msg += '*';
		msg += " expands to more than ";
		msg += str(query->get_max_expansion());
		msg += " terms";
		throw Xapian::WildcardError(msg);
	    }
	    --expansions_left;
	}

	add_postlist(qopt->open_lazy_post_list(term, 1, factor), NULL);
    }

    if (max_type == Xapian::Query::WILDCARD_LIMIT_MOST_FREQUENT) {
	auto set_size = query->get_max_expansion();
	if (size() > set_size) {
	    auto begin = pls.begin();
	    nth_element(begin, begin + set_size - 1, pls.end(),
			ComparePostListTermFreqAscending());
	    shrink(set_size);
	}
    }

    // Now register the postlists we're actually using for stats.
    for (auto pl : pls) {
	// FIXME: Be more typesafe?
	qopt->register_lazy_postlist_for_stats(static_cast<LeafPostList*>(pl),
					       termfreqs);
	add_termfreqs(termfreqs);
    }
}

inline void
Context::expand_edit_distance(const QueryEditDistance* query,
			      double factor,
			      TermFreqs* termfreqs)
{
    string pfx(query->get_pattern(), 0, query->get_fixed_prefix_len());
    unique_ptr<TermList> t(qopt->db.open_allterms(pfx));
    bool skip_ucase = pfx.empty();
    auto max_type = query->get_max_type();
    Xapian::termcount expansions_left = query->get_max_expansion();
    // If there's no expansion limit, set expansions_left to the maximum
    // value it can hold.
    if (expansions_left == 0)
	expansions_left = numeric_limits<decltype(expansions_left)>::max();
    while (true) {
	TermList* res = t->next();
done_skip_to:
	if (res) {
	    // Pruning shouldn't be possible, as this is iterating allterms for
	    // a single shard.
	    Assert(res == t.get());
	    // Out of entries.
	    break;
	}

	const string& term = t->get_termname();
	if (!startswith(term, pfx))
	    break;
	if (skip_ucase && term[0] >= 'A') {
	    // Skip terms that start with A-Z, as we don't want the expansion
	    // to include prefixed terms.
	    //
	    // This assumes things about the structure of terms which the
	    // Query class otherwise doesn't need to care about, but it
	    // seems hard to avoid here.
	    skip_ucase = false;
	    if (term[0] <= 'Z') {
		static_assert('Z' + 1 == '[', "'Z' + 1 == '['");
		res = t->skip_to("[");
		goto done_skip_to;
	    }
	}

	if (!query->test(term)) continue;

	if (max_type < Xapian::Query::WILDCARD_LIMIT_MOST_FREQUENT) {
	    if (expansions_left == 0) {
		if (max_type == Xapian::Query::WILDCARD_LIMIT_FIRST)
		    break;
		string msg("Edit distance ");
		msg += query->get_pattern();
		msg += '~';
		msg += str(query->get_threshold());
		msg += " expands to more than ";
		msg += str(query->get_max_expansion());
		msg += " terms";
		throw Xapian::WildcardError(msg);
	    }
	    --expansions_left;
	}

	add_postlist(qopt->open_lazy_post_list(term, 1, factor), NULL);
    }

    if (max_type == Xapian::Query::WILDCARD_LIMIT_MOST_FREQUENT) {
	auto set_size = query->get_max_expansion();
	if (size() > set_size) {
	    auto begin = pls.begin();
	    nth_element(begin, begin + set_size - 1, pls.end(),
			ComparePostListTermFreqAscending());
	    shrink(set_size);
	}
    }

    // Now register the postlists we're actually using for stats.
    for (auto pl : pls) {
	// FIXME: Be more typesafe?
	qopt->register_lazy_postlist_for_stats(static_cast<LeafPostList*>(pl),
					       termfreqs);
	add_termfreqs(termfreqs);
    }
}

class OrContext : public Context {
  public:
    OrContext(QueryOptimiser* qopt_, size_t reserve)
	: Context(qopt_, reserve) { }

    void estimate_termfreqs(TermFreqs* termfreqs);

    /// Select the best set_size postlists from the last out_of added.
    void select_elite_set(size_t set_size, size_t out_of);

    PostList* postlist(TermFreqs* termfreqs, bool bool_or = false);

    PostList* postlist_max();
};

void
OrContext::estimate_termfreqs(TermFreqs* termfreqs)
{
    Assert(termfreqs);

    Assert(!termfreqs_list.empty());

    // We calculate the estimate assuming independence.  The simplest
    // way to calculate this seems to be a series of (n - 1) pairwise
    // calculations, which gives the same answer regardless of the order.
    const TermFreqs& freqs = termfreqs_list[0];
    auto& stats = *qopt->get_stats();

    // Our caller should have ensured this.
    Assert(stats.collection_size);
    double scale = 1.0 / stats.collection_size;
    double P_est = freqs.termfreq * scale;
    double rtf_scale = 0.0;
    if (stats.rset_size != 0) {
	rtf_scale = 1.0 / stats.rset_size;
    }
    double Pr_est = freqs.reltermfreq * rtf_scale;
    // If total_length is 0, cf must always be 0 so cf_scale is irrelevant.
    double cf_scale = 0.0;
    if (usual(stats.total_length != 0)) {
	cf_scale = 1.0 / stats.total_length;
    }
    double Pc_est = freqs.collfreq * cf_scale;

    for (size_t i = 1; i < termfreqs_list.size(); ++i) {
	const TermFreqs& f = termfreqs_list[i];
	double P_i = f.termfreq * scale;
	P_est += P_i - P_est * P_i;
	double Pc_i = f.collfreq * cf_scale;
	Pc_est += Pc_i - Pc_est * Pc_i;
	// If the rset is empty, Pr_est should be 0 already, so leave
	// it alone.
	if (stats.rset_size != 0) {
	    double Pr_i = f.reltermfreq * rtf_scale;
	    Pr_est += Pr_i - Pr_est * Pr_i;
	}
    }

    *termfreqs =
	TermFreqs(Xapian::doccount(P_est * stats.collection_size + 0.5),
		  Xapian::doccount(Pr_est * stats.rset_size + 0.5),
		  Xapian::termcount(Pc_est * stats.total_length + 0.5));
}

void
OrContext::select_elite_set(size_t set_size, size_t out_of)
{
    auto begin = pls.begin() + pls.size() - out_of;
    nth_element(begin, begin + set_size - 1, pls.end(), CmpMaxOrTerms());
    shrink(pls.size() - out_of + set_size);
}

PostList*
OrContext::postlist(TermFreqs* termfreqs, bool bool_or)
{
    if (!termfreqs_list.empty()) estimate_termfreqs(termfreqs);

    switch (pls.size()) {
	case 0:
	    return NULL;
	case 1: {
	    PostList* pl = pls[0];
	    pls.clear();
	    return pl;
	}
    }

    qopt->add_op(EstimateOp::OR, pls.size(), first, last);

    if (bool_or) {
	auto pl = new BoolOrPostList(pls.begin(), pls.end(), qopt->db_size);
	// Empty pls so our destructor doesn't delete them all!
	pls.clear();
	return pl;
    }

    // Make postlists into a heap so that the postlist with the greatest term
    // frequency is at the top of the heap.
    Heap::make(pls.begin(), pls.end(), ComparePostListTermFreqAscending());

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
	//   l.get_termfreq() >= r.get_termfreq()
	//
	// We do this so that the OrPostList class can be optimised assuming
	// that this is the case.
	PostList* r = pls.front();
	Heap::pop(pls.begin(), pls.end(), ComparePostListTermFreqAscending());
	pls.pop_back();
	auto pl = new OrPostList(pls.front(), r, qopt->matcher);

	if (pls.size() == 1) {
	    pls.clear();
	    return pl;
	}

	pls[0] = pl;
	Heap::replace(pls.begin(), pls.end(),
		      ComparePostListTermFreqAscending());
    }
}

PostList *
OrContext::postlist_max()
{
    switch (pls.size()) {
	case 0:
	    return NULL;
	case 1: {
	    PostList* pl = pls[0];
	    pls.clear();
	    return pl;
	}
    }

    // Sort the postlists so that the postlist with the greatest term frequency
    // is first.
    sort(pls.begin(), pls.end(), ComparePostListTermFreqAscending());

    PostList * pl;
    pl = new MaxPostList(pls.begin(), pls.end(), qopt->matcher, qopt->db_size);
    // Same as OR for number of matches.
    qopt->add_op(EstimateOp::OR, pls.size(), first, last);

    pls.clear();
    return pl;
}

class XorContext : public Context {
  public:
    XorContext(QueryOptimiser* qopt_, size_t reserve)
	: Context(qopt_, reserve) { }

    PostList* postlist(TermFreqs* termfreqs);
};

PostList *
XorContext::postlist(TermFreqs* termfreqs)
{
    if (pls.empty())
	return NULL;

    if (termfreqs) {
	Assert(!termfreqs_list.empty());

	// We calculate the estimate assuming independence.  The simplest
	// way to calculate this seems to be a series of (n - 1) pairwise
	// calculations, which gives the same answer regardless of the order.
	auto& stats = *qopt->get_stats();
	const TermFreqs& freqs = termfreqs_list[0];

	// Our caller should have ensured this.
	Assert(stats.collection_size);
	double scale = 1.0 / stats.collection_size;
	double P_est = freqs.termfreq * scale;
	double rtf_scale = 0.0;
	if (stats.rset_size != 0) {
	    rtf_scale = 1.0 / stats.rset_size;
	}
	double Pr_est = freqs.reltermfreq * rtf_scale;
	// If total_length is 0, cf must always be 0 so cf_scale is irrelevant.
	double cf_scale = 0.0;
	if (usual(stats.total_length != 0)) {
	    cf_scale = 1.0 / stats.total_length;
	}
	double Pc_est = freqs.collfreq * cf_scale;

	for (size_t i = 1; i < termfreqs_list.size(); ++i) {
	    const TermFreqs& f = termfreqs_list[i];
	    double P_i = f.termfreq * scale;
	    P_est += P_i - 2.0 * P_est * P_i;
	    double Pc_i = f.collfreq * cf_scale;
	    Pc_est += Pc_i - 2.0 * Pc_est * Pc_i;
	    // If the rset is empty, Pr_est should be 0 already, so leave
	    // it alone.
	    if (stats.rset_size != 0) {
		double Pr_i = f.reltermfreq * rtf_scale;
		Pr_est += Pr_i - 2.0 * Pr_est * Pr_i;
	    }
	}

	*termfreqs =
	    TermFreqs(Xapian::doccount(P_est * stats.collection_size + 0.5),
		      Xapian::doccount(Pr_est * stats.rset_size + 0.5),
		      Xapian::termcount(Pc_est * stats.total_length + 0.5));
    }

    Xapian::doccount db_size = qopt->db_size;
    auto pl = new XorPostList(pls.begin(), pls.end(), qopt->matcher, db_size);
    qopt->add_op(EstimateOp::XOR, pls.size(), first, last);

    // Empty pls so our destructor doesn't delete them all!
    pls.clear();
    return pl;
}

class PosFilter {
    Xapian::Query::op op_;

    /// Start and end indices for the PostLists this positional filter uses.
    size_t begin, end;

    Xapian::termcount window;

  public:
    PosFilter(Xapian::Query::op op__, size_t begin_, size_t end_,
	      Xapian::termcount window_)
	: op_(op__), begin(begin_), end(end_), window(window_) { }

    PostList* postlist(PostList* pl,
		       const vector<PostList*>& pls,
		       PostListTree* pltree,
		       QueryOptimiser* qopt,
		       TermFreqs* termfreqs) const
    try {
	auto terms_begin = pls.begin() + begin;
	auto terms_end = pls.begin() + end;

	if (op_ == Xapian::Query::OP_NEAR) {
	    auto estimate_op = qopt->add_op(EstimateOp::NEAR);
	    if (termfreqs) *termfreqs /= 2;
	    pl = new NearPostList(pl, estimate_op,
				  window, terms_begin, terms_end, pltree);
	} else if (window != end - begin) {
	    AssertEq(op_, Xapian::Query::OP_PHRASE);
	    auto estimate_op = qopt->add_op(EstimateOp::PHRASE);
	    if (termfreqs) *termfreqs /= 3;
	    pl = new PhrasePostList(pl, estimate_op,
				    window, terms_begin, terms_end, pltree);
	} else {
	    AssertEq(op_, Xapian::Query::OP_PHRASE);
	    auto estimate_op = qopt->add_op(EstimateOp::EXACT_PHRASE);
	    if (termfreqs) *termfreqs /= 4;
	    pl = new ExactPhrasePostList(pl, estimate_op,
					 terms_begin, terms_end, pltree);
	}
	return pl;
    } catch (...) {
	delete pl;
	throw;
    }
};

class AndContext : public Context {
    list<PosFilter> pos_filters;

    unique_ptr<OrContext> not_ctx;

    unique_ptr<OrContext> maybe_ctx;

    /** True if this AndContext has seen a no-op MatchAll.
     *
     *  If it has and it ends up empty then the resulting postlist should be
     *  MatchAll not MatchNothing.
     */
    bool match_all = false;

  public:
    AndContext(QueryOptimiser* qopt_, size_t reserve)
	: Context(qopt_, reserve) {
	first = 1;
	last = Xapian::docid(-1);
    }

    bool add_postlist(PostList* pl, TermFreqs* termfreqs) {
	add_termfreqs(termfreqs);
	if (pl) {
	    if (pls.empty() && termfreqs_list.size() > 1) {
		qopt->destroy_postlist(pl);
		return true;
	    }
	    pls.emplace_back(pl);
	    Xapian::docid pl_first = first, pl_last = last;
	    pl->get_docid_range(pl_first, pl_last);
	    first = std::max(first, pl_first);
	    last = std::min(last, pl_last);
	    if (first <= last) {
		return true;
	    }
	}
	shrink(0);
	match_all = false;
	return termfreqs != NULL;
    }

    void set_match_all() { match_all = true; }

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

    PostList* postlist(TermFreqs* termfreqs);
};

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

template<typename T, typename U>
inline static T
estimate_and_not(T l, T r, U n)
{
    // We calculate the estimates assuming independence.  With this assumption,
    // the estimate is the product of the estimates for the sub-postlists
    // (with the right side this is inverted by subtracting from the total
    // size), divided by the total size.
    return static_cast<T>((l * double(n - r)) / n + 0.5);
}

PostList *
AndContext::postlist(TermFreqs* termfreqs)
{
    auto matcher = qopt->matcher;
    auto db_size = qopt->db_size;

    if (termfreqs) {
	Assert(!termfreqs_list.empty());

	// We calculate the estimate assuming independence.  With this
	// assumption, the estimate is the product of the estimates for the
	// sub-postlists divided by db_size (n - 1) times.
	const TermFreqs& freqs = termfreqs_list[0];

	double freqest = double(freqs.termfreq);
	double relfreqest = double(freqs.reltermfreq);
	double collfreqest = double(freqs.collfreq);

	auto& stats = *qopt->get_stats();

	// Our caller should have ensured this.
	Assert(stats.collection_size);

	for (size_t i = 1; i < termfreqs_list.size(); ++i) {
	    const TermFreqs& f = termfreqs_list[i];

	    // If the collection is empty, freqest should be 0 already, so
	    // leave it alone.
	    freqest *= f.termfreq / stats.collection_size;
	    if (usual(stats.total_length != 0)) {
		collfreqest *= f.collfreq / stats.total_length;
	    }

	    // If the rset is empty, relfreqest should be 0 already, so leave
	    // it alone.
	    if (stats.rset_size != 0)
		relfreqest *= f.reltermfreq / stats.rset_size;
	}

	*termfreqs =
	    TermFreqs(static_cast<Xapian::doccount>(freqest + 0.5),
		      static_cast<Xapian::doccount>(relfreqest + 0.5),
		      static_cast<Xapian::termcount>(collfreqest + 0.5));
    }

    unique_ptr<PostList> pl;
    switch (pls.size()) {
      case 0:
	if (!match_all) {
	    // The "and" part doesn't match anything, so any "not" part or
	    // positional filters are irrelevant.
	    return NULL;
	}
	pl.reset(qopt->open_post_list({}, 0, 0.0, nullptr));
	break;
      case 1:
	pl.reset(pls[0]);
	break;
      default:
	pl.reset(new AndPostList(pls.begin(), pls.end(), matcher));
	qopt->add_op(EstimateOp::AND, pls.size(), first, last);
	break;
    }

    if (not_ctx && !not_ctx->empty()) {
	if (not_ctx->get_last() < first || not_ctx->get_first() > last) {
	    // The ranges don't overlap so the right side has no effect.
	    // The call to not_ctx.reset() below will clean up the estimate
	    // stack.
	} else {
	    TermFreqs r_freqs;
	    PostList* rhs = not_ctx->postlist(termfreqs ? &r_freqs : NULL,
					      true);
	    if (termfreqs) {
		TermFreqs& freqs = *termfreqs;
		auto& stats = *qopt->get_stats();

		// Our caller should have ensured this.
		Assert(stats.collection_size);
		freqs.termfreq = estimate_and_not(freqs.termfreq,
						  r_freqs.termfreq,
						  stats.collection_size);

		// If total_length is 0 then collfreq should always be 0 (since
		// total_length is the sum of all collfreq values) so nothing
		// to do.
		if (stats.total_length != 0) {
		    freqs.collfreq = estimate_and_not(freqs.collfreq,
						      r_freqs.collfreq,
						      stats.total_length);
		}

		// If the rset is empty then relfreqest should always be 0 so
		// nothing to do.
		if (stats.rset_size != 0) {
		    freqs.reltermfreq = estimate_and_not(freqs.reltermfreq,
							 r_freqs.reltermfreq,
							 stats.rset_size);
		}
	    }

	    pl.reset(new AndNotPostList(pl.release(), rhs, db_size));
	    // The bounds are the same as those for the left side.
	    qopt->add_op(EstimateOp::AND_NOT, 2, first, last);
	}
	not_ctx.reset();
    }

    // Sort the positional filters to try to apply them in an efficient order.
    // FIXME: We need to figure out what that is!  Try applying lowest cf/tf
    // first?

    // Apply any positional filters.
    for (const PosFilter& filter : pos_filters) {
	pl.reset(filter.postlist(pl.release(), pls, matcher, qopt, termfreqs));
    }

    // Empty pls so our destructor doesn't delete them all!
    pls.clear();

    if (maybe_ctx && !maybe_ctx->empty()) {
	if (maybe_ctx->get_last() < first || maybe_ctx->get_first() > last) {
	    // The ranges don't overlap so the right side has no effect.
	    // The call to maybe_ctx.reset() below will clean up the estimate
	    // stack.
	} else {
	    PostList* rhs = maybe_ctx->postlist(termfreqs);
	    // A NULL return from OrContext::postlist() can only mean that
	    // maybe_ctx is empty, but in that case get_last() returns zero
	    // which means we would have taken the branch above.
	    Assert(rhs);
	    // For OP_AND_MAYBE only the LHS determines which documents match
	    // (the RHS only adds weight) so for the estimates we can just
	    // ignore the RHS and the operator itself).
	    qopt->pop_op();
	    pl.reset(new AndMaybePostList(pl.release(), rhs, matcher));
	}
	maybe_ctx.reset();
    }

    return pl.release();
}

}

Query::Internal::~Internal() { }

size_t
Query::Internal::get_num_subqueries() const noexcept
{
    return 0;
}

const Query
Query::Internal::get_subquery(size_t) const
{
    throw Xapian::InvalidArgumentError("get_subquery() not meaningful for this Query object");
}

Xapian::termcount
Query::Internal::get_wqf() const
{
    throw Xapian::InvalidArgumentError("get_wqf() not meaningful for this Query object");
}

Xapian::termpos
Query::Internal::get_pos() const
{
    throw Xapian::InvalidArgumentError("get_pos() not meaningful for this Query object");
}

void
Query::Internal::gather_terms(void *) const
{
}

Xapian::termcount
Query::Internal::get_length() const noexcept
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
		if (!unpack_uint(p, end, &n_subqs)) {
		    unpack_throw_serialisation_error(*p);
		}
		n_subqs += 8;
	    }
	    unsigned char code = (ch >> 3) & 0x0f;
	    Xapian::termcount parameter = 0;
	    if (code >= 13) {
		if (!unpack_uint(p, end, &parameter)) {
		    unpack_throw_serialisation_error(*p);
		}
	    }
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
		if (!unpack_uint(p, end, &len)) {
		    unpack_throw_serialisation_error(*p);
		}
		len += 16;
	    }
	    if (size_t(end - *p) < len)
		throw SerialisationError("Not enough data");
	    string term(*p, len);
	    *p += len;

	    int code = ((ch >> 4) & 0x03);

	    Xapian::termcount wqf = static_cast<Xapian::termcount>(code > 0);
	    if (code == 3) {
		if (!unpack_uint(p, end, &wqf)) {
		    unpack_throw_serialisation_error(*p);
		}
	    }

	    Xapian::termpos pos = 0;
	    if (code >= 2) {
		if (!unpack_uint(p, end, &pos)) {
		    unpack_throw_serialisation_error(*p);
		}
	    }

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
		if (!unpack_uint(p, end, &slot)) {
		    unpack_throw_serialisation_error(*p);
		}
		slot += 15;
	    }
	    string begin;
	    if (!unpack_string(p, end, begin)) {
		unpack_throw_serialisation_error(*p);
	    }
	    if (ch & 0x10) {
		// OP_VALUE_GE
		return new Xapian::Internal::QueryValueGE(slot, begin);
	    }

	    // OP_VALUE_RANGE
	    string end_;
	    if (!unpack_string(p, end, end_)) {
		unpack_throw_serialisation_error(*p);
	    }
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
		case 0x0a: { // Edit distance
		    Xapian::termcount max_expansion;
		    if (!unpack_uint(p, end, &max_expansion) || end - *p < 2) {
			throw SerialisationError("not enough data");
		    }
		    int flags = static_cast<unsigned char>(*(*p)++);
		    op combiner = static_cast<op>(*(*p)++);
		    unsigned edit_distance;
		    size_t fixed_prefix_len;
		    string pattern;
		    if (!unpack_uint(p, end, &edit_distance) ||
			!unpack_uint(p, end, &fixed_prefix_len) ||
			!unpack_string(p, end, pattern)) {
			throw SerialisationError("not enough data");
		    }
		    using Xapian::Internal::QueryEditDistance;
		    return new QueryEditDistance(pattern,
						 max_expansion,
						 flags,
						 combiner,
						 edit_distance,
						 fixed_prefix_len);
		}
		case 0x0b: { // Wildcard
		    Xapian::termcount max_expansion;
		    if (!unpack_uint(p, end, &max_expansion) || end - *p < 2) {
			throw SerialisationError("not enough data");
		    }
		    int flags = static_cast<unsigned char>(*(*p)++);
		    op combiner = static_cast<op>(*(*p)++);
		    string pattern;
		    if (!unpack_string(p, end, pattern)) {
			throw SerialisationError("not enough data");
		    }
		    return new Xapian::Internal::QueryWildcard(pattern,
							       max_expansion,
							       flags,
							       combiner);
		}
		case 0x0c: { // PostingSource
		    string name;
		    if (!unpack_string(p, end, name)) {
			throw SerialisationError("not enough data");
		    }

		    const PostingSource * reg_source = reg.get_posting_source(name);
		    if (!reg_source) {
			string m = "PostingSource ";
			m += name;
			m += " not registered";
			throw SerialisationError(m);
		    }

		    string serialised_source;
		    if (!unpack_string(p, end, serialised_source)) {
			throw SerialisationError("not enough data");
		    }
		    PostingSource* source =
			reg_source->unserialise_with_registry(serialised_source,
							      reg);
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
		    if (!unpack_uint(p, end, &wqf) ||
			!unpack_uint(p, end, &pos)) {
			throw SerialisationError("not enough data");
		    }
		    return new Xapian::Internal::QueryTerm({}, wqf, pos);
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

bool
Query::Internal::postlist_sub_and_like(AndContext& ctx,
				       QueryOptimiser * qopt,
				       double factor,
				       TermFreqs* termfreqs) const
{
    return ctx.add_postlist(postlist(qopt, factor, termfreqs), termfreqs);
}

void
Query::Internal::postlist_sub_or_like(OrContext& ctx,
				      QueryOptimiser* qopt,
				      double factor,
				      TermFreqs* termfreqs,
				      bool keep_zero_weight) const
{
    Xapian::termcount save_total_subqs = qopt->get_total_subqs();
    unique_ptr<PostList> pl(postlist(qopt, factor, termfreqs));
    if (!keep_zero_weight && pl && pl->recalc_maxweight() == 0.0) {
	// This subquery can't contribute any weight, so can be discarded.
	//
	// Restore the value of total_subqs so that percentages don't get
	// messed up if we increased total_subqs in the call to postlist()
	// above.
	qopt->set_total_subqs(save_total_subqs);
	qopt->destroy_postlist(pl.release());
	return;
    }
    ctx.add_postlist(pl.release(), termfreqs);
}

void
Query::Internal::postlist_sub_bool_or_like(OrContext& ctx,
					   QueryOptimiser* qopt,
					   TermFreqs* termfreqs) const
{
    ctx.add_postlist(postlist(qopt, 0.0, termfreqs), termfreqs);
}

void
Query::Internal::postlist_sub_xor(XorContext& ctx,
				  QueryOptimiser* qopt,
				  double factor,
				  TermFreqs* termfreqs) const
{
    ctx.add_postlist(postlist(qopt, factor, termfreqs), termfreqs);
}

namespace Internal {

Query::op
QueryTerm::get_type() const noexcept
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
QueryPostingSource::get_type() const noexcept
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
QueryScaleWeight::get_type() const noexcept
{
    return Query::OP_SCALE_WEIGHT;
}

size_t
QueryScaleWeight::get_num_subqueries() const noexcept
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
    Assert(subquery.internal);
    string desc = str(scale_factor);
    desc += " * ";
    desc += subquery.internal->get_description();
    return desc;
}

PostList*
QueryTerm::postlist(QueryOptimiser* qopt, double factor,
		    TermFreqs* termfreqs) const
{
    LOGCALL(QUERY, PostList*, "QueryTerm::postlist", qopt | factor | termfreqs);
    if (factor != 0.0)
	qopt->inc_total_subqs();
    RETURN(qopt->open_post_list(term, wqf, factor, termfreqs));
}

bool
QueryTerm::postlist_sub_and_like(AndContext& ctx,
				 QueryOptimiser* qopt,
				 double factor,
				 TermFreqs* termfreqs) const
{
    if (term.empty() && !qopt->need_positions && factor == 0.0 && !termfreqs) {
	// No-op MatchAll.
	ctx.set_match_all();
	return true;
    }
    return ctx.add_postlist(postlist(qopt, factor, termfreqs), termfreqs);
}

PostList*
QueryPostingSource::postlist(QueryOptimiser* qopt, double factor,
			     TermFreqs* termfreqs) const
{
    LOGCALL(QUERY, PostList*, "QueryPostingSource::postlist", qopt | factor | termfreqs);
    Assert(source);
    if (factor != 0.0)
	qopt->inc_total_subqs();
    auto estimate_op = qopt->add_op();
    // Casting away const on the Database::Internal here is OK, as we wrap
    // them in a const Xapian::Database so non-const methods can't actually
    // be called on the Database::Internal object.
    const Xapian::Database wrappeddb(
	    const_cast<Xapian::Database::Internal*>(&(qopt->db)));
    auto pl =
	new ExternalPostList(wrappeddb, source.get(), estimate_op, factor,
			     qopt->matcher->get_max_weight_cached_flag_ptr(),
			     qopt->shard_index);
    if (termfreqs) {
	auto& stats = *qopt->get_stats();
	auto db_size = qopt->db_size;
	// Scale proportionately from this shard to the whole collection.  Not
	// as good as summing over all shards, but that's harder to do.
	double termfreq = pl->get_termfreq();
	auto tf = termfreq * stats.collection_size / db_size;
	auto rtf = termfreq * stats.rset_size / db_size;
	auto cf = termfreq * stats.total_length / db_size;
	*termfreqs = TermFreqs(static_cast<Xapian::doccount>(tf + 0.5),
			       static_cast<Xapian::doccount>(rtf + 0.5),
			       static_cast<Xapian::termcount>(cf + 0.5));
    }
    RETURN(pl);
}

PostList*
QueryScaleWeight::postlist(QueryOptimiser* qopt, double factor,
			   TermFreqs* termfreqs) const
{
    LOGCALL(QUERY, PostList*, "QueryScaleWeight::postlist", qopt | factor | termfreqs);
    RETURN(subquery.internal->postlist(qopt, factor * scale_factor, termfreqs));
}

bool
QueryScaleWeight::postlist_sub_and_like(AndContext& ctx,
					QueryOptimiser* qopt,
					double factor,
					TermFreqs* termfreqs) const
{
    return subquery.internal->postlist_sub_and_like(ctx, qopt,
						    factor * scale_factor,
						    termfreqs);
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

static double
string_frac(const string& s, size_t prefix)
{
    double r = 0;
    double f = 1.0;
    for (size_t i = prefix; i != s.size(); ++i) {
	f /= 256.0;
	r += static_cast<unsigned char>(s[i]) * f;
    }

    return r;
}

static Xapian::doccount
estimate_range_freq(const string& lo, const string& hi,
		    const string& begin, const string* end,
		    Xapian::doccount value_freq)
{
    // Assume the values are evenly spread out between lo and hi.
    // FIXME: Perhaps we should store some sort of binned distribution?
    AssertRel(lo, <=, hi);

    size_t common_prefix_len = size_t(-1);
    do {
	UNSIGNED_OVERFLOW_OK(++common_prefix_len);
	// lo <= hi so while we're in the common prefix hi can't run out
	// before lo.
	if (common_prefix_len == lo.size()) {
	    if (common_prefix_len != hi.size())
		break;
	    // All values in the slot are the same.  We should have optimised
	    // to NULL if that singular value is outside the range, and if it's
	    // inside the range then we know that the frequency is exactly the
	    // value frequency.
	    Assert(begin <= lo && (!end || hi <= *end));
	    return value_freq;
	}
	AssertRel(common_prefix_len, !=, hi.size());
    } while (lo[common_prefix_len] == hi[common_prefix_len]);

    double l = string_frac(lo, common_prefix_len);
    double h = string_frac(hi, common_prefix_len);
    double denom = h - l;
    if (rare(denom == 0.0)) {
	// Weird corner case - hi != lo (because that's handled inside the loop
	// above) but they give the same string_frac value.  Because we only
	// calculate the fraction starting from the first difference, this
	// should only happen if hi is lo + one or more trailing zero bytes.

	// The case where all set values lie within the range should be handled
	// at a higher level and we shouldn't get called.
	Assert(!(begin <= lo && (!end || hi <= *end)));

	// There must be partial overlap as the cases where the range
	// dominates the bounds and where the range is entirely outside the
	// bounds are both handled at a higher level.
	return value_freq / 2;
    }

    double b = l;
    if (begin > lo) {
	b = string_frac(begin, common_prefix_len);
    }
    double e = h;
    if (end && *end < hi) {
	// end is NULL for a ValueGePostList
	e = string_frac(*end, common_prefix_len);
    }

    double est = (e - b) / denom * value_freq;
    return Xapian::doccount(est + 0.5);
}

PostList*
QueryValueRange::postlist(QueryOptimiser* qopt, double factor,
			  TermFreqs* termfreqs) const
{
    LOGCALL(QUERY, PostList*, "QueryValueRange::postlist", qopt | factor | termfreqs);
    if (factor != 0.0)
	qopt->inc_total_subqs();
    const Xapian::Database::Internal & db = qopt->db;
    const auto db_size = qopt->db_size;
    const string & lb = db.get_value_lower_bound(slot);
    if (lb.empty()) {
	// This should only happen if there are no values in this slot (which
	// could be because the backend just doesn't support values at all).
	// If there were values in the slot, the backend should have a
	// non-empty lower bound, even if it isn't a tight one.
	AssertEq(db.get_value_freq(slot), 0);
	if (termfreqs) *termfreqs = TermFreqs();
	RETURN(NULL);
    }
    if (end < lb) {
	if (termfreqs) *termfreqs = TermFreqs();
	RETURN(NULL);
    }
    const string & ub = db.get_value_upper_bound(slot);
    if (begin > ub) {
	if (termfreqs) *termfreqs = TermFreqs();
	RETURN(NULL);
    }

    if (termfreqs) {
	// We scale these below.
	auto& stats = *qopt->get_stats();
	*termfreqs = TermFreqs(stats.collection_size,
			       stats.rset_size,
			       stats.total_length);
    }

    auto value_freq = db.get_value_freq(slot);
    if (end >= ub) {
	if (begin <= lb) {
	    // The known bounds for the slot both fall within the range so we
	    // know the range matches whenever the value is set, which is
	    // exactly value_freq times.
	    auto estimate_op = qopt->add_op(value_freq);
	    if (value_freq == db_size) {
		// This value is set for all documents in the current shard, so
		// we can replace it with a MatchAll postlist, which is
		// especially efficient if there are no gaps in the docids.
		RETURN(db.open_post_list({}));
	    }
	    // We need to check which documents have a value set in this slot
	    // but don't need to worry about the range bounds so we can use
	    // ValueGePostList with an empty string as the lower bound which
	    // means the range test just becomes a cheap `>= string()` test.
	    if (termfreqs) *termfreqs *= double(value_freq) / db_size;
	    RETURN(new ValueGePostList(&db, estimate_op,
				       value_freq, slot, string()));
	}
	auto est = estimate_range_freq(lb, ub, begin, NULL, value_freq);
	auto estimate_op = qopt->add_op(Estimates(0, est, value_freq));
	if (termfreqs) *termfreqs *= double(est) / db_size;
	RETURN(new ValueGePostList(&db, estimate_op, est, slot, begin));
    }
    auto est = estimate_range_freq(lb, ub, begin, &end, value_freq);
    auto estimate_op = qopt->add_op(Estimates(0, est, value_freq));
    if (termfreqs) *termfreqs *= double(est) / db_size;
    RETURN(new ValueRangePostList(&db, estimate_op, est, slot, begin, end));
}

void
QueryValueRange::serialise(string & result) const
{
    if (slot < 15) {
	result += static_cast<char>(0x20 | slot);
    } else {
	result += static_cast<char>(0x20 | 15);
	pack_uint(result, slot - 15);
    }
    pack_string(result, begin);
    pack_string(result, end);
}

Query::op
QueryValueRange::get_type() const noexcept
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

PostList*
QueryValueLE::postlist(QueryOptimiser* qopt, double factor,
		       TermFreqs* termfreqs) const
{
    LOGCALL(QUERY, PostList*, "QueryValueLE::postlist", qopt | factor | termfreqs);
    if (factor != 0.0)
	qopt->inc_total_subqs();
    const Xapian::Database::Internal & db = qopt->db;
    const auto db_size = qopt->db_size;
    const string & lb = db.get_value_lower_bound(slot);
    if (lb.empty()) {
	// This should only happen if there are no values in this slot (which
	// could be because the backend just doesn't support values at all).
	// If there were values in the slot, the backend should have a
	// non-empty lower bound, even if it isn't a tight one.
	AssertEq(db.get_value_freq(slot), 0);
	if (termfreqs) *termfreqs = TermFreqs();
	RETURN(NULL);
    }
    if (limit < lb) {
	if (termfreqs) *termfreqs = TermFreqs();
	RETURN(NULL);
    }

    if (termfreqs) {
	// We scale these below.
	auto& stats = *qopt->get_stats();
	*termfreqs = TermFreqs(stats.collection_size,
			       stats.rset_size,
			       stats.total_length);
    }

    auto value_freq = db.get_value_freq(slot);
    const string& ub = db.get_value_upper_bound(slot);
    if (limit >= ub) {
	// The known bounds for the slot both fall within the range so we
	// know the range matches whenever the value is set, which is
	// exactly value_freq times.
	auto estimate_op = qopt->add_op(value_freq);
	if (value_freq == db_size) {
	    // This value is set for all documents in the current shard, so
	    // we can replace it with a MatchAll postlist, which is
	    // especially efficient if there are no gaps in the docids.
	    RETURN(db.open_post_list({}));
	}
	// We need to check which documents have a value set in this slot
	// but don't need to worry about the range bounds so we can use
	// ValueGePostList with an empty string as the lower bound which
	// means the range test just becomes a cheap `>= string()` test.
	if (termfreqs) *termfreqs *= double(value_freq) / db_size;
	RETURN(new ValueGePostList(&db, estimate_op,
				   value_freq, slot, string()));
    }
    auto est = estimate_range_freq(lb, ub, string(), &limit, value_freq);
    auto estimate_op = qopt->add_op(0, est, value_freq);
    if (termfreqs) *termfreqs *= double(est) / db_size;
    RETURN(new ValueRangePostList(&db, estimate_op,
				  est, slot, string(), limit));
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
	pack_uint(result, slot - 15);
    }
    pack_string_empty(result);
    pack_string(result, limit);
}

Query::op
QueryValueLE::get_type() const noexcept
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

PostList*
QueryValueGE::postlist(QueryOptimiser* qopt, double factor,
		       TermFreqs* termfreqs) const
{
    LOGCALL(QUERY, PostList*, "QueryValueGE::postlist", qopt | factor | termfreqs);
    if (factor != 0.0)
	qopt->inc_total_subqs();
    const Xapian::Database::Internal & db = qopt->db;
    const auto db_size = qopt->db_size;
    const string & lb = db.get_value_lower_bound(slot);
    if (lb.empty()) {
	// This should only happen if there are no values in this slot (which
	// could be because the backend just doesn't support values at all).
	// If there were values in the slot, the backend should have a
	// non-empty lower bound, even if it isn't a tight one.
	AssertEq(db.get_value_freq(slot), 0);
	if (termfreqs) *termfreqs = TermFreqs();
	RETURN(NULL);
    }
    const string& ub = db.get_value_upper_bound(slot);
    if (limit > ub) {
	if (termfreqs) *termfreqs = TermFreqs();
	RETURN(NULL);
    }

    if (termfreqs) {
	// We scale these below.
	auto& stats = *qopt->get_stats();
	*termfreqs = TermFreqs(stats.collection_size,
			       stats.rset_size,
			       stats.total_length);
    }

    auto value_freq = db.get_value_freq(slot);
    if (limit <= lb) {
	// The known bounds for the slot both fall within the range so we
	// know the range matches whenever the value is set, which is
	// exactly value_freq times.
	auto estimate_op = qopt->add_op(value_freq);
	if (value_freq == db_size) {
	    // This value is set for all documents in the current shard, so
	    // we can replace it with a MatchAll postlist, which is
	    // especially efficient if there are no gaps in the docids.
	    RETURN(db.open_post_list({}));
	}
	// We need to check which documents have a value set in this slot
	// but don't need to worry about the range bounds so we can use
	// ValueGePostList with an empty string as the lower bound which
	// means the range test just becomes a cheap `>= string()` test.
	RETURN(new ValueGePostList(&db, estimate_op,
				   value_freq, slot, string()));
    }
    auto est = estimate_range_freq(lb, ub, limit, NULL, value_freq);
    auto estimate_op = qopt->add_op(0, est, value_freq);
    if (termfreqs) *termfreqs *= double(est) / db_size;
    RETURN(new ValueGePostList(&db, estimate_op, est, slot, limit));
}

void
QueryValueGE::serialise(string & result) const
{
    if (slot < 15) {
	result += static_cast<char>(0x20 | 0x10 | slot);
    } else {
	result += static_cast<char>(0x20 | 0x10 | 15);
	pack_uint(result, slot - 15);
    }
    pack_string(result, limit);
}

Query::op
QueryValueGE::get_type() const noexcept
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

QueryWildcard::QueryWildcard(std::string_view pattern_,
			     Xapian::termcount max_expansion_,
			     int flags_,
			     Query::op combiner_)
    : pattern(pattern_),
      max_expansion(max_expansion_),
      flags(flags_),
      combiner(combiner_)
{
    if ((flags & ~Query::WILDCARD_LIMIT_MASK_) == 0) {
	head = min_len = pattern.size();
	max_len = numeric_limits<decltype(max_len)>::max();
	prefix = pattern;
	return;
    }

    size_t i = 0;
    while (i != pattern.size()) {
	// Check for characters with special meaning.
	switch (pattern[i]) {
	    case '*':
		if (flags & Query::WILDCARD_PATTERN_MULTI)
		    goto found_special;
		break;
	    case '?':
		if (flags & Query::WILDCARD_PATTERN_SINGLE)
		    goto found_special;
		break;
	}
	prefix += pattern[i];
	++i;
	head = i;
    }
found_special:

    min_len = max_len = prefix.size();

    tail = i;
    bool had_qm = false, had_star = false;
    while (i != pattern.size()) {
	switch (pattern[i]) {
	    default:
default_case:
		suffix += pattern[i];
		++min_len;
		++max_len;
		break;

	    case '*':
		if (!(flags & Query::WILDCARD_PATTERN_MULTI))
		    goto default_case;
		// Matches zero or more characters.
		had_star = true;
		tail = i + 1;
		if (!suffix.empty()) {
		    check_pattern = true;
		    suffix.clear();
		}
		break;

	    case '?':
		if (!(flags & Query::WILDCARD_PATTERN_SINGLE))
		    goto default_case;
		// Matches exactly one character.
		tail = i + 1;
		if (!suffix.empty()) {
		    check_pattern = true;
		    suffix.clear();
		}
		// `?` matches one Unicode character, which is 1-4 bytes in
		// UTF-8, so we have to actually check the pattern if there's
		// more than one `?` in it.
		if (had_qm) {
		    check_pattern = true;
		}
		had_qm = true;
		++min_len;
		max_len += MAX_UTF_8_CHARACTER_LENGTH;
		break;
	}

	++i;
    }

    if (had_star) {
	max_len = numeric_limits<decltype(max_len)>::max();
    } else {
	// If the pattern only contains `?` wildcards we'll need to check it
	// since `?` matches one Unicode character, which is 1-4 bytes in
	// UTF-8.  FIXME: We can avoid this if there's only one `?` wildcard
	// and the candidate is min_len bytes long.
	check_pattern = true;
    }
}

bool
QueryWildcard::test_wildcard_(const string& candidate, size_t o, size_t p,
			      size_t i) const
{
    // FIXME: Optimisation potential here.  We could compile the pattern to a
    // regex, or other tricks like calculating the min length needed after each
    // position that we test with this method - e.g. for foo*bar*x*baz there
    // must be at least 7 bytes after a position or there's no point testing if
    // "bar" matches there.
    for ( ; i != tail; ++i) {
	if ((flags & Query::WILDCARD_PATTERN_MULTI) && pattern[i] == '*') {
	    if (++i == tail) {
		// '*' at end of variable part is easy!
		return true;
	    }
	    for (size_t test_o = o; test_o <= p; ++test_o) {
		if (test_wildcard_(candidate, test_o, p, i))
		    return true;
	    }
	    return false;
	}
	if (o == p) return false;
	if ((flags & Query::WILDCARD_PATTERN_SINGLE) && pattern[i] == '?') {
	    unsigned char b = candidate[o];
	    if (b < 0xc0) {
		++o;
		continue;
	    }
	    unsigned seqlen;
	    if (b < 0xe0) {
		seqlen = 2;
	    } else if (b < 0xf0) {
		seqlen = 3;
	    } else {
		seqlen = 4;
	    }
	    if (rare(p - o < seqlen)) return false;
	    o += seqlen;
	    continue;
	}

	if (pattern[i] != candidate[o]) return false;
	++o;
    }
    return (o == p);
}

bool
QueryWildcard::test_prefix_known(const string& candidate) const
{
    if (candidate.size() < min_len) return false;
    if (candidate.size() > max_len) return false;
    if (!endswith(candidate, suffix)) return false;

    if (!check_pattern) return true;

    return test_wildcard_(candidate, prefix.size(),
			  candidate.size() - suffix.size(),
			  head);
}

PostList*
QueryWildcard::postlist(QueryOptimiser* qopt, double factor,
			TermFreqs* termfreqs) const
{
    LOGCALL(QUERY, PostList*, "QueryWildcard::postlist", qopt | factor | termfreqs);
    OrContext ctx(qopt, 0);
    Query::op op = combiner;
    if (factor == 0.0 || op == Query::OP_SYNONYM) {
	if (factor == 0.0) {
	    // If we have a factor of 0, we don't care about the weights, so
	    // we're just like a normal OR query.
	    op = Query::OP_OR;
	}

	bool old_compound_weight = qopt->compound_weight;
	if (!old_compound_weight) {
	    qopt->compound_weight = (op == Query::OP_SYNONYM);
	}

	TermFreqs synonym_freqs;
	if (op == Query::OP_SYNONYM) {
	    qopt->inc_total_subqs();
	    ctx.expand_wildcard(this, 0.0, &synonym_freqs);
	} else {
	    ctx.expand_wildcard(this, 0.0, termfreqs);
	}

	qopt->compound_weight = old_compound_weight;

	if (ctx.empty())
	    RETURN(NULL);

	if (op != Query::OP_SYNONYM)
	    RETURN(ctx.postlist(termfreqs, true));

	// We build an OP_OR tree for OP_SYNONYM and then wrap it in a
	// SynonymPostList, which supplies the weights.
	//
	// We know the subqueries from a wildcard expansion are wdf-disjoint
	// (i.e. each wdf from the document contributes at most itself to the
	// wdf of the subquery).
	RETURN(qopt->make_synonym_postlist(ctx.postlist(&synonym_freqs, true),
					   factor, true, synonym_freqs));
    }

    ctx.expand_wildcard(this, factor, termfreqs);

    qopt->set_total_subqs(qopt->get_total_subqs() + ctx.size());

    if (ctx.empty())
	RETURN(NULL);

    if (op == Query::OP_MAX)
	RETURN(ctx.postlist_max());

    RETURN(ctx.postlist(termfreqs));
}

termcount
QueryWildcard::get_length() const noexcept
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
    pack_uint(result, max_expansion);
    result += static_cast<unsigned char>(flags);
    result += static_cast<unsigned char>(combiner);
    pack_string(result, pattern);
}

Query::op
QueryWildcard::get_type() const noexcept
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

int
QueryEditDistance::test(const string& candidate) const
{
    int threshold = get_threshold();
    int edist = edcalc(candidate, threshold);
    return edist <= threshold ? edist + 1 : 0;
}

PostList*
QueryEditDistance::postlist(QueryOptimiser* qopt, double factor,
			    TermFreqs* termfreqs) const
{
    LOGCALL(QUERY, PostList*, "QueryEditDistance::postlist", qopt | factor | termfreqs);
    OrContext ctx(qopt, 0);
    Query::op op = combiner;
    if (factor == 0.0 || op == Query::OP_SYNONYM) {
	if (factor == 0.0) {
	    // If we have a factor of 0, we don't care about the weights, so
	    // we're just like a normal OR query.
	    op = Query::OP_OR;
	}

	bool old_compound_weight = qopt->compound_weight;
	if (!old_compound_weight) {
	    qopt->compound_weight = (op == Query::OP_SYNONYM);
	}

	TermFreqs synonym_freqs;
	if (op == Query::OP_SYNONYM) {
	    qopt->inc_total_subqs();
	    ctx.expand_edit_distance(this, 0.0, &synonym_freqs);
	} else {
	    ctx.expand_edit_distance(this, 0.0, termfreqs);
	}

	qopt->compound_weight = old_compound_weight;

	if (ctx.empty())
	    RETURN(NULL);

	if (op != Query::OP_SYNONYM)
	    RETURN(ctx.postlist(termfreqs, true));

	// We build an OP_OR tree for OP_SYNONYM and then wrap it in a
	// SynonymPostList, which supplies the weights.
	//
	// We know the subqueries from an edit distance expansion are
	// wdf-disjoint (i.e. each wdf from the document contributes at most
	// itself to the wdf of the subquery).
	RETURN(qopt->make_synonym_postlist(ctx.postlist(&synonym_freqs, true),
					   factor, true, synonym_freqs));
    }

    ctx.expand_edit_distance(this, factor, termfreqs);

    qopt->set_total_subqs(qopt->get_total_subqs() + ctx.size());

    if (ctx.empty())
	RETURN(NULL);

    if (op == Query::OP_MAX)
	RETURN(ctx.postlist_max());

    RETURN(ctx.postlist(termfreqs));
}

termcount
QueryEditDistance::get_length() const noexcept
{
    // We currently assume wqf is 1 for calculating the synonym's weight
    // since conceptually the synonym is one "virtual" term.  If we were
    // to combine multiple occurrences of the same synonym expansion into
    // a single instance with wqf set, we would want to track the wqf.
    return 1;
}

void
QueryEditDistance::serialise(string & result) const
{
    result += static_cast<char>(0x0a);
    pack_uint(result, max_expansion);
    result += static_cast<unsigned char>(flags);
    result += static_cast<unsigned char>(combiner);
    pack_uint(result, edit_distance);
    pack_uint(result, fixed_prefix_len);
    pack_string(result, pattern);
}

Query::op
QueryEditDistance::get_type() const noexcept
{
    return Query::OP_EDIT_DISTANCE;
}

string
QueryEditDistance::get_description() const
{
    string desc = "EDIT_DISTANCE ";
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
    desc += '~';
    desc += str(edit_distance);
    if (fixed_prefix_len) {
	desc += " fixed_prefix_len=";
	desc += str(fixed_prefix_len);
    }
    return desc;
}

Xapian::termcount
QueryBranch::get_length() const noexcept
{
    // Sum results from all subqueries.
    Xapian::termcount result = 0;
    QueryVector::const_iterator i;
    for (i = subqueries.begin(); i != subqueries.end(); ++i) {
	// MatchNothing subqueries should have been removed by done(), but we
	// can't use Assert in a noexcept function.  But we'll get a
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
	    pack_uint(result, subqueries.size() - 8);
	if (ch >= MULTIWAY(13))
	    pack_uint(result, parameter);
    } else {
	result += ch;
    }

    QueryVector::const_iterator i;
    for (i = subqueries.begin(); i != subqueries.end(); ++i) {
	// MatchNothing subqueries should have been removed by done().
	Assert((*i).internal);
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
	Assert((*i).internal);
	(*i).internal->gather_terms(void_terms);
    }
}

void
QueryBranch::do_bool_or_like(OrContext& ctx,
			     QueryOptimiser* qopt,
			     TermFreqs* termfreqs,
			     size_t first) const
{
    LOGCALL_VOID(MATCH, "QueryBranch::do_bool_or_like", ctx | qopt | termfreqs | first);

    // FIXME: we could optimise by merging OP_ELITE_SET and OP_OR like we do
    // for AND-like operations.

    // OP_SYNONYM with a single subquery is only simplified by
    // QuerySynonym::done() if the single subquery is a term or MatchAll.
    Assert(subqueries.size() >= 2 || get_op() == Query::OP_SYNONYM);

    QueryVector::const_iterator q;
    for (q = subqueries.begin() + first; q != subqueries.end(); ++q) {
	// MatchNothing subqueries should have been removed by done().
	Assert((*q).internal);
	(*q).internal->postlist_sub_bool_or_like(ctx, qopt, termfreqs);
    }
}

void
QueryBranch::do_or_like(OrContext& ctx, QueryOptimiser * qopt, double factor,
			TermFreqs* termfreqs,
			Xapian::termcount elite_set_size, size_t first,
			bool keep_zero_weight) const
{
    LOGCALL_VOID(MATCH, "QueryBranch::do_or_like", ctx | qopt | factor | termfreqs | elite_set_size | first | keep_zero_weight);

    // FIXME: we could optimise by merging OP_ELITE_SET and OP_OR like we do
    // for AND-like operations.

    // OP_SYNONYM with a single subquery is only simplified by
    // QuerySynonym::done() if the single subquery is a term or MatchAll.
    Assert(subqueries.size() >= 2 || get_op() == Query::OP_SYNONYM);

    size_t size_before = ctx.size();
    QueryVector::const_iterator q;
    for (q = subqueries.begin() + first; q != subqueries.end(); ++q) {
	// MatchNothing subqueries should have been removed by done().
	Assert((*q).internal);
	(*q).internal->postlist_sub_or_like(ctx, qopt, factor,
					    termfreqs,
					    keep_zero_weight);
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
QueryBranch::do_synonym(QueryOptimiser* qopt,
			double factor,
			TermFreqs* termfreqs) const
{
    LOGCALL(MATCH, PostList*, "QueryBranch::do_synonym", qopt | factor | termfreqs);
    OrContext ctx(qopt, subqueries.size());
    if (factor == 0.0) {
	// If we have a factor of 0, we don't care about the weights, so
	// we're just like a normal OR query.  An OP_SYNONYM sets factor=0
	// for its subqueries so this handles an OP_SYNONYM below an
	// OP_SYNONYM.
	do_bool_or_like(ctx, qopt, termfreqs);
	return ctx.postlist(termfreqs, true);
    }

    bool old_compound_weight = qopt->compound_weight;
    Assert(!old_compound_weight);
    qopt->compound_weight = true;
    TermFreqs synonym_freqs;
    do_bool_or_like(ctx, qopt, &synonym_freqs);
    PostList* pl = ctx.postlist(&synonym_freqs, true);
    qopt->compound_weight = old_compound_weight;
    if (!pl) return NULL;

    bool wdf_disjoint = false;
    Assert(!subqueries.empty());
    auto type = subqueries.front().get_type();
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
	    prefixes.push_back(qw->get_fixed_prefix());
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
    RETURN(qopt->make_synonym_postlist(pl, factor, wdf_disjoint,
				       synonym_freqs));
}

PostList *
QueryBranch::do_max(QueryOptimiser* qopt,
		    double factor,
		    TermFreqs* termfreqs) const
{
    LOGCALL(MATCH, PostList*, "QueryBranch::do_max", qopt | factor | termfreqs);
    OrContext ctx(qopt, subqueries.size());
    if (factor == 0.0) {
	// Without the weights we're just like a normal OR query.
	do_bool_or_like(ctx, qopt, termfreqs);
	RETURN(ctx.postlist(termfreqs, true));
    }

    // If termfreqs is set that means we're below a synonym, but in that case
    // factor should be 0.0 which is handled above.
    Assert(!termfreqs);

    do_or_like(ctx, qopt, factor, termfreqs);
    // We currently assume wqf is 1 for calculating the OP_MAX's weight
    // since conceptually the OP_MAX is one "virtual" term.  If we were
    // to combine multiple occurrences of the same OP_MAX expansion into
    // a single instance with wqf set, we would want to track the wqf.
    RETURN(ctx.postlist_max());
}

Xapian::Query::op
QueryBranch::get_type() const noexcept
{
    return get_op();
}

size_t
QueryBranch::get_num_subqueries() const noexcept
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
	Assert((*i).internal);
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
	    pack_uint(result, wqf);
	    pack_uint(result, pos);
	}
    } else if (wqf == 1) {
	if (pos == 0) {
	    // Single occurrence free-text term without position set.
	    if (len >= 16) {
		result += static_cast<char>(0x40 | 0x10);
		pack_uint(result, term.size() - 16);
	    } else {
		result += static_cast<char>(0x40 | 0x10 | len);
	    }
	    result += term;
	} else {
	    // Single occurrence free-text term with position set.
	    if (len >= 16) {
		result += static_cast<char>(0x40 | 0x20);
		pack_uint(result, term.size() - 16);
	    } else {
		result += static_cast<char>(0x40 | 0x20 | len);
	    }
	    result += term;
	    pack_uint(result, pos);
	}
    } else if (wqf > 1 || pos > 0) {
	// General case.
	if (len >= 16) {
	    result += static_cast<char>(0x40 | 0x30);
	    pack_uint(result, term.size() - 16);
	} else if (len) {
	    result += static_cast<char>(0x40 | 0x30 | len);
	}
	result += term;
	pack_uint(result, wqf);
	pack_uint(result, pos);
    } else {
	// Typical boolean term.
	AssertEq(wqf, 0);
	AssertEq(pos, 0);
	if (len >= 16) {
	    result += static_cast<char>(0x40);
	    pack_uint(result, term.size() - 16);
	} else {
	    result += static_cast<char>(0x40 | len);
	}
	result += term;
    }
}

void QueryPostingSource::serialise(string & result) const
{
    result += static_cast<char>(0x0c);
    pack_string(result, source->name());
    pack_string(result, source->serialise());
}

void QueryScaleWeight::serialise(string & result) const
{
    Assert(subquery.internal);
    result += '\x0d';
    result += serialise_double(scale_factor);
    subquery.internal->serialise(result);
}

void
QueryAndLike::add_subquery(const Xapian::Query & subquery)
{
    // If the AndLike is already MatchNothing, do nothing.
    if (subqueries.size() == 1 && !subqueries[0].internal)
	return;
    // If we're adding MatchNothing, discard any previous subqueries.
    if (!subquery.internal)
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

PostList*
QueryAndLike::postlist(QueryOptimiser* qopt, double factor,
		       TermFreqs* termfreqs) const
{
    LOGCALL(QUERY, PostList*, "QueryAndLike::postlist", qopt | factor | termfreqs);
    AndContext ctx(qopt, subqueries.size());
    if (!postlist_sub_and_like(ctx, qopt, factor, termfreqs)) {
	RETURN(nullptr);
    }
    RETURN(ctx.postlist(termfreqs));
}

bool
QueryAndLike::postlist_sub_and_like(AndContext& ctx,
				    QueryOptimiser* qopt,
				    double factor,
				    TermFreqs* termfreqs) const
{
    QueryVector::const_iterator i;
    for (i = subqueries.begin(); i != subqueries.end(); ++i) {
	// MatchNothing subqueries should have been removed by done().
	Assert((*i).internal);
	if (!(*i).internal->postlist_sub_and_like(ctx, qopt, factor, termfreqs))
	    return false;
    }
    return true;
}

void
QueryOrLike::add_subquery(const Xapian::Query & subquery)
{
    // Drop any subqueries which are MatchNothing.
    if (subquery.internal)
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
	if (!subqueries[0].internal) {
	    // The left side is already MatchNothing so drop any right side.
	    //
	    // MatchNothing AND_NOT X == MatchNothing
	    return;
	}
	if (!subquery.internal) {
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
	    Assert(subquery.get_subquery(0).internal);
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
    if (subqueries.size() == 1 && !subqueries[0].internal)
	return;
    // Drop any 2nd or subsequent subqueries which are MatchNothing.
    if (subquery.internal || subqueries.empty())
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

PostList*
QueryOr::postlist(QueryOptimiser* qopt, double factor,
		  TermFreqs* termfreqs) const
{
    LOGCALL(QUERY, PostList*, "QueryOr::postlist", qopt | factor | termfreqs);
    OrContext ctx(qopt, subqueries.size());
    if (factor == 0.0) {
	do_bool_or_like(ctx, qopt, termfreqs);
	RETURN(ctx.postlist(termfreqs, true));
    }
    do_or_like(ctx, qopt, factor, termfreqs);
    RETURN(ctx.postlist(termfreqs));
}

void
QueryOr::postlist_sub_or_like(OrContext& ctx, QueryOptimiser* qopt,
			      double factor,
			      TermFreqs* termfreqs,
			      bool keep_zero_weight) const
{
    do_or_like(ctx, qopt, factor, termfreqs, 0, 0, keep_zero_weight);
}

void
QueryOr::postlist_sub_bool_or_like(OrContext& ctx,
				   QueryOptimiser* qopt,
				   TermFreqs* termfreqs) const
{
    do_bool_or_like(ctx, qopt, termfreqs);
}

PostList*
QueryAndNot::postlist(QueryOptimiser* qopt, double factor,
		      TermFreqs* termfreqs) const
{
    LOGCALL(QUERY, PostList*, "QueryAndNot::postlist", qopt | factor | termfreqs);
    AndContext ctx(qopt, 1);
    if (!QueryAndNot::postlist_sub_and_like(ctx, qopt, factor, termfreqs)) {
	RETURN(NULL);
    }
    RETURN(ctx.postlist(termfreqs));
}

bool
QueryAndNot::postlist_sub_and_like(AndContext& ctx,
				   QueryOptimiser* qopt,
				   double factor,
				   TermFreqs* termfreqs) const
{
    // This invariant should be established by QueryAndNot::done() with
    // assistance from QueryAndNot::add_subquery().
    Assert(subqueries[0].internal);
    if (!subqueries[0].internal->postlist_sub_and_like(ctx, qopt, factor,
						       termfreqs)) {
	return false;
    }
    do_bool_or_like(ctx.get_not_ctx(subqueries.size() - 1), qopt, termfreqs, 1);
    return true;
}

PostList*
QueryXor::postlist(QueryOptimiser* qopt, double factor,
		   TermFreqs* termfreqs) const
{
    LOGCALL(QUERY, PostList*, "QueryXor::postlist", qopt | factor | termfreqs);
    XorContext ctx(qopt, subqueries.size());
    postlist_sub_xor(ctx, qopt, factor, termfreqs);
    RETURN(ctx.postlist(termfreqs));
}

void
QueryXor::postlist_sub_xor(XorContext& ctx,
			   QueryOptimiser* qopt,
			   double factor,
			   TermFreqs* termfreqs) const
{
    QueryVector::const_iterator i;
    for (i = subqueries.begin(); i != subqueries.end(); ++i) {
	// MatchNothing subqueries should have been removed by done().
	Assert((*i).internal);
	(*i).internal->postlist_sub_xor(ctx, qopt, factor, termfreqs);
    }
}

PostList*
QueryAndMaybe::postlist(QueryOptimiser* qopt, double factor,
			TermFreqs* termfreqs) const
{
    LOGCALL(QUERY, PostList*, "QueryAndMaybe::postlist", qopt | factor | termfreqs);
    AndContext ctx(qopt, 1);
    if (!QueryAndMaybe::postlist_sub_and_like(ctx, qopt, factor, termfreqs)) {
	RETURN(NULL);
    }
    RETURN(ctx.postlist(termfreqs));
}

bool
QueryAndMaybe::postlist_sub_and_like(AndContext& ctx,
				     QueryOptimiser* qopt,
				     double factor,
				     TermFreqs* termfreqs) const
{
    // This invariant should be established by QueryAndMaybe::done() with
    // assistance from QueryAndMaybe::add_subquery().
    Assert(subqueries[0].internal);
    if (!subqueries[0].internal->postlist_sub_and_like(ctx, qopt, factor,
						       termfreqs)) {
	return false;
    }
    // We only need to consider the right branch or branches if we're weighted
    // - an unweighted OP_AND_MAYBE can be replaced with its left branch.
    if (factor != 0.0) {
	// Only keep zero-weight subqueries if we need their wdf because
	// they're underneath a compound weight.
	OrContext& maybe_ctx = ctx.get_maybe_ctx(subqueries.size() - 1);
	bool need_wdf = qopt->need_wdf_for_compound_weight();
	do_or_like(maybe_ctx, qopt, factor, termfreqs, 0, 1, need_wdf);
    }
    return true;
}

PostList*
QueryFilter::postlist(QueryOptimiser* qopt, double factor,
		      TermFreqs* termfreqs) const
{
    LOGCALL(QUERY, PostList*, "QueryFilter::postlist", qopt | factor | termfreqs);
    AndContext ctx(qopt, subqueries.size());
    if (!QueryFilter::postlist_sub_and_like(ctx, qopt, factor, termfreqs)) {
	RETURN(nullptr);
    }
    RETURN(ctx.postlist(termfreqs));
}

bool
QueryFilter::postlist_sub_and_like(AndContext& ctx,
				   QueryOptimiser* qopt,
				   double factor,
				   TermFreqs* termfreqs) const
{
    QueryVector::const_iterator i;
    for (i = subqueries.begin(); i != subqueries.end(); ++i) {
	// MatchNothing subqueries should have been removed by done().
	Assert((*i).internal);
	if (!(*i).internal->postlist_sub_and_like(ctx, qopt, factor, termfreqs))
	    return false;
	// Second and subsequent subqueries are unweighted.
	factor = 0.0;
    }
    return true;
}

bool
QueryWindowed::postlist_windowed(Query::op op,
				 AndContext& ctx,
				 QueryOptimiser* qopt,
				 double factor,
				 TermFreqs* termfreqs) const
{
    if (!qopt->db.has_positions()) {
	// No positions in this subdatabase so this matches nothing, which
	// means the whole andcontext matches nothing.
	//
	// Bailing out here means we don't recurse deeper and that means we
	// don't call QueryOptimiser::inc_total_subqs() for leaf postlists in
	// the phrase, but at least one shard will count them, and the matcher
	// takes the highest answer (since 1.4.6).
	ctx.shrink(0);
	return false;
    }

    bool old_need_positions = qopt->need_positions;
    qopt->need_positions = true;

    bool result = true;
    QueryVector::const_iterator i;
    for (i = subqueries.begin(); i != subqueries.end(); ++i) {
	// MatchNothing subqueries should have been removed by done().
	Assert((*i).internal);
	PostList* pl = (*i).internal->postlist(qopt, factor, NULL);
	if (pl && (*i).internal->get_type() != Query::LEAF_TERM) {
	    pl = new OrPosPostList(pl);
	}
	result = ctx.add_postlist(pl, termfreqs);
	if (!result) {
	    if (factor == 0.0) break;
	    // If we don't complete the iteration, the subquery count may be
	    // wrong, and weighting information may not be filled in.
	    while (i != subqueries.end()) {
		// MatchNothing subqueries should have been removed by done().
		// FIXME: Can we handle this more gracefully?
		Assert((*i).internal);
		qopt->destroy_postlist((*i).internal->postlist(qopt, factor,
							       NULL));
		++i;
	    }
	    break;
	}
    }
    if (result) {
	// Record the positional filter to apply higher up the tree.
	ctx.add_pos_filter(op, subqueries.size(), window);
    }

    qopt->need_positions = old_need_positions;
    return result;
}

bool
QueryPhrase::postlist_sub_and_like(AndContext& ctx,
				   QueryOptimiser* qopt,
				   double factor,
				   TermFreqs* termfreqs) const
{
    constexpr auto OP_PHRASE = Query::OP_PHRASE;
    return QueryWindowed::postlist_windowed(OP_PHRASE, ctx, qopt, factor,
					    termfreqs);
}

bool
QueryNear::postlist_sub_and_like(AndContext& ctx,
				 QueryOptimiser* qopt,
				 double factor,
				 TermFreqs* termfreqs) const
{
    constexpr auto OP_NEAR = Query::OP_NEAR;
    return QueryWindowed::postlist_windowed(OP_NEAR, ctx, qopt, factor,
					    termfreqs);
}

PostList*
QueryEliteSet::postlist(QueryOptimiser* qopt, double factor,
			TermFreqs* termfreqs) const
{
    LOGCALL(QUERY, PostList*, "QueryEliteSet::postlist", qopt | factor | termfreqs);
    OrContext ctx(qopt, subqueries.size());
    do_or_like(ctx, qopt, factor, termfreqs, set_size);
    RETURN(ctx.postlist(termfreqs));
}

void
QueryEliteSet::postlist_sub_or_like(OrContext& ctx, QueryOptimiser* qopt,
				    double factor,
				    TermFreqs* termfreqs,
				    bool keep_zero_weight) const
{
    do_or_like(ctx, qopt, factor, termfreqs, set_size, 0, keep_zero_weight);
}

PostList*
QuerySynonym::postlist(QueryOptimiser* qopt, double factor,
		       TermFreqs* termfreqs) const
{
    LOGCALL(QUERY, PostList*, "QuerySynonym::postlist", qopt | factor | termfreqs);
    // Save and restore total_subqs so we only add one for the whole
    // OP_SYNONYM subquery (or none if we're not weighted).
    Xapian::termcount save_total_subqs = qopt->get_total_subqs();
    if (factor != 0.0)
	++save_total_subqs;
    PostList* pl = do_synonym(qopt, factor, termfreqs);
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
	if (sub_type == Query::OP_EDIT_DISTANCE) {
	    auto q =
		static_cast<QueryEditDistance*>(subqueries[0].internal.get());
	    // SYNONYM over EDIT_DISTANCE X -> EDIT_DISTANCE SYNONYM for any
	    // combiner X.
	    return q->change_combiner(Query::OP_SYNONYM);
	}
    }
    return this;
}

PostList*
QueryMax::postlist(QueryOptimiser* qopt, double factor,
		   TermFreqs* termfreqs) const
{
    LOGCALL(QUERY, PostList*, "QueryMax::postlist", qopt | factor | termfreqs);
    // Save and restore total_subqs so we only add one for the whole
    // OP_MAX subquery (or none if we're not weighted).
    Xapian::termcount save_total_subqs = qopt->get_total_subqs();
    if (factor != 0.0)
	++save_total_subqs;
    PostList* pl = do_max(qopt, factor, termfreqs);
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
QueryInvalid::get_type() const noexcept
{
    return Xapian::Query::OP_INVALID;
}

PostList*
QueryInvalid::postlist(QueryOptimiser*, double, TermFreqs*) const
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
