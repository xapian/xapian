/** @file queryinternal.cc
 * @brief Xapian::Query internals
 */
/* Copyright (C) 2007,2008,2009,2010,2011,2012,2013,2014 Olly Betts
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

#include "xapian/postingsource.h"
#include "xapian/query.h"

#include "matcher/const_database_wrapper.h"
#include "leafpostlist.h"
#include "matcher/andmaybepostlist.h"
#include "matcher/andnotpostlist.h"
#include "emptypostlist.h"
#include "matcher/exactphrasepostlist.h"
#include "matcher/externalpostlist.h"
#include "matcher/maxpostlist.h"
#include "matcher/multiandpostlist.h"
#include "matcher/multixorpostlist.h"
#include "matcher/orpostlist.h"
#include "matcher/phrasepostlist.h"
#include "matcher/queryoptimiser.h"
#include "matcher/valuerangepostlist.h"
#include "matcher/valuegepostlist.h"
#include "net/length.h"
#include "serialise-double.h"

#include "autoptr.h"
#include "debuglog.h"
#include "omassert.h"
#include "str.h"
#include "unicode/description_append.h"

#include <algorithm>
#include <functional>
#include <list>
#include <string>
#include <vector>

using namespace std;

template<class CLASS> struct delete_ptr {
    void operator()(CLASS *p) { delete p; }
};

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
    /** Return true if and only if a has a strictly greater termweight than b;
     *  with the proviso that if the termfrequency of a or b is 0, then the
     *  termweight is considered to be 0.
     *
     *  We use termfreq_max() because we really don't want to exclude a
     *  postlist which has a low but non-zero termfrequency: the estimate
     *  is quite likely to be zero in this case.
     */
    bool operator()(const PostList *a, const PostList *b) {
	if (a->get_termfreq_max() == 0) return false;
	if (b->get_termfreq_max() == 0) return true;

#if (defined(__i386__) && !defined(__SSE2_MATH__)) || defined(__mc68000__) || defined(__mc68010__) || defined(__mc68020__) || defined(__mc68030__)
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
	// http://gcc.gnu.org/ml/gcc-patches/2008-11/msg00105.html
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
    bool operator()(const PostList *a, const PostList *b) {
	return a->get_termfreq_est() > b->get_termfreq_est();
    }
};

class Context {
  protected:
    vector<PostList*> pls;

  public:
    explicit Context(size_t reserve);

    ~Context();

    void add_postlist(PostList * pl) {
	pls.push_back(pl);
    }
};

Context::Context(size_t reserve) {
    pls.reserve(reserve);
}

Context::~Context()
{
    for_each(pls.begin(), pls.end(), delete_ptr<PostList>());
}

class OrContext : public Context {
  public:
    explicit OrContext(size_t reserve) : Context(reserve) { }

    /// Select the best set_size postlists from the last out_of added.
    void select_elite_set(size_t set_size, size_t out_of);

    PostList * postlist(QueryOptimiser* qopt);
    PostList * postlist_max(QueryOptimiser* qopt);
};

void
OrContext::select_elite_set(size_t set_size, size_t out_of)
{
    // Call recalc_maxweight() as otherwise get_maxweight()
    // may not be valid before next() or skip_to()
    vector<PostList*>::iterator begin = pls.begin() + pls.size() - out_of;
    for_each(begin, pls.end(), mem_fun(&PostList::recalc_maxweight));

    nth_element(begin, begin + set_size - 1, pls.end(), CmpMaxOrTerms());
    for_each(begin + set_size, pls.end(), delete_ptr<PostList>());
    pls.resize(pls.size() - out_of + set_size);
}

PostList *
OrContext::postlist(QueryOptimiser* qopt)
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
OrContext::postlist_max(QueryOptimiser* qopt)
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
    explicit XorContext(size_t reserve) : Context(reserve) { }

    PostList * postlist(QueryOptimiser* qopt);
};

PostList *
XorContext::postlist(QueryOptimiser* qopt)
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

  public:
    explicit AndContext(size_t reserve) : Context(reserve) { }

    void add_pos_filter(Query::op op_,
			size_t n_subqs,
			Xapian::termcount window);

    PostList * postlist(QueryOptimiser* qopt);
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
    size_t end = pls.size();
    size_t begin = end - n_subqs;
    pos_filters.push_back(PosFilter(op_, begin, end, window));
}

PostList *
AndContext::postlist(QueryOptimiser* qopt)
{
    AutoPtr<PostList> pl(new MultiAndPostList(pls.begin(), pls.end(),
					      qopt->matcher, qopt->db_size));

    // Sort the positional filters to try to apply them in an efficient order.
    // FIXME: We need to figure out what that is!  Try applying lowest cf/tf
    // first?

    // Apply any positional filters.
    list<PosFilter>::const_iterator i;
    for (i = pos_filters.begin(); i != pos_filters.end(); ++i) {
	const PosFilter & filter = *i;
	pl.reset(filter.postlist(pl.release(), pls));
    }

    // Empty pls so our destructor doesn't delete them all!
    pls.clear();
    return pl.release();
}

}

Query::Internal::~Internal() { }

size_t
Query::Internal::get_num_subqueries() const
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
Query::Internal::get_length() const
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
	case 4: case 5: case 6: case 7: { // Multi-way branch
	    size_t n_subqs = ch & 0x07;
	    if (n_subqs == 0) {
		n_subqs = decode_length(p, end, false) + 9;
	    } else {
		++n_subqs;
	    }
	    unsigned char code = (ch >> 3) & 0x0f;
	    Xapian::termcount parameter = 0;
	    if (code >= 5)
		parameter = decode_length(p, end, false);
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
		result->add_subquery(Xapian::Query(*unserialise(p, end, reg)));
	    } while (--n_subqs);
	    result->done();
	    return result;
	}
	case 2: case 3: { // Term
	    size_t len = ch & 0x0f;
	    if (len == 0)
		len = decode_length(p, end, false) + 16;
	    if (size_t(end - *p) < len)
		throw SerialisationError("Not enough data");
	    string term(*p, len);
	    *p += len;

	    int code = ((ch >> 4) & 0x03);

	    Xapian::termcount wqf = static_cast<Xapian::termcount>(code > 0);
	    if (code == 3)
		wqf = decode_length(p, end, false);

	    Xapian::termpos pos = code >= 2 ? decode_length(p, end, false) : 0;

	    return new Xapian::Internal::QueryTerm(term, wqf, pos);
	}
	case 1: { // OP_VALUE_RANGE or OP_VALUE_GE or OP_VALUE_LE
	    Xapian::valueno slot = ch & 15;
	    if (slot == 15)
		slot = decode_length(p, end, false) + 15;
	    size_t len = decode_length(p, end, true);
	    string begin(*p, len);
	    *p += len;
	    if (ch & 0x10) {
		// OP_VALUE_GE
		return new Xapian::Internal::QueryValueGE(slot, begin);
	    }

	    // OP_VALUE_RANGE
	    len = decode_length(p, end, true);
	    string end_(*p, len);
	    *p += len;
	    if (begin.empty()) // FIXME: is this right?
		return new Xapian::Internal::QueryValueLE(slot, end_);
	    return new Xapian::Internal::QueryValueRange(slot, begin, end_);
	}
	case 0: {
	    switch (ch & 0x0f) {
		case 0x0c: { // PostingSource
		    size_t len = decode_length(p, end, true);
		    string name(*p, len);
		    *p += len;

		    const PostingSource * reg_source = reg.get_posting_source(name);
		    if (!reg_source) {
			string m = "PostingSource ";
			m += name;
			m += " not registered";
			throw SerialisationError(m);
		    }

		    len = decode_length(p, end, true);
		    PostingSource * source =
			reg_source->unserialise_with_registry(string(*p, len),
							      reg);
		    *p += len;
		    return new Xapian::Internal::QueryPostingSource(source, true);
		}
		case 0x0d: {
		    using Xapian::Internal::QueryScaleWeight;
		    double scale_factor = unserialise_double(p, end);
		    return new QueryScaleWeight(scale_factor,
						Query(*unserialise(p, end, reg)));
		}
		case 0x0e: {
		    Xapian::termcount wqf = decode_length(p, end, false);
		    Xapian::termpos pos = decode_length(p, end, false);
		    return new Xapian::Internal::QueryTerm(string(), wqf, pos);
		}
		case 0x0f:
		    return Query::MatchAll.internal.get();
		default: // Others currently unused.
		    break;
	    }
	    break;
	}
    }
    throw SerialisationError("Unknown Query serialisation");
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
QueryTerm::get_type() const
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

QueryPostingSource::QueryPostingSource(PostingSource * source_, bool owned_)
    : source(source_), owned(owned_)
{
    if (!source)
	throw Xapian::InvalidArgumentError("source parameter can't be NULL");
    if (!owned) {
	PostingSource * cloned_source = source->clone();
	if (cloned_source) {
	    source = cloned_source;
	    owned = true;
	}
    }
}

QueryPostingSource::~QueryPostingSource()
{
    if (owned)
	delete source;
}

Query::op
QueryPostingSource::get_type() const
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
QueryScaleWeight::get_type() const
{
    return Query::OP_SCALE_WEIGHT;
}

size_t
QueryScaleWeight::get_num_subqueries() const
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
    bool weighted = false;
    if (factor != 0.0) {
	qopt->inc_total_subqs();
	if (!term.empty())
	    weighted = true;
    }

    AutoPtr<Xapian::Weight> wt(weighted ? qopt->make_wt(term, wqf, factor) : 0);

    AutoPtr<LeafPostList> pl(
	qopt->open_post_list(term, weighted ? wt->get_maxpart() : 0.0));

    if (weighted)
	pl->set_termweight(wt.release());
    RETURN(pl.release());
}

PostingIterator::Internal *
QueryPostingSource::postlist(QueryOptimiser * qopt, double factor) const
{
    LOGCALL(QUERY, PostingIterator::Internal *, "QueryPostingSource::postlist", qopt | factor);
    Assert(source);
    if (factor != 0.0)
	qopt->inc_total_subqs();
    Xapian::Database wrappeddb(new ConstDatabaseWrapper(&(qopt->db)));
    RETURN(new ExternalPostList(wrappeddb, source, factor, qopt->matcher));
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
	vector<pair<Xapian::termpos, string> > &terms =
	    *static_cast<vector<pair<Xapian::termpos, string> >*>(void_terms);
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
    // If lb.empty(), the backend doesn't provide value bounds.
    if (!lb.empty() && (end < lb || begin > db.get_value_upper_bound(slot))) {
	RETURN(new EmptyPostList);
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
QueryValueRange::get_type() const
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
    if (limit < db.get_value_lower_bound(slot)) {
	RETURN(new EmptyPostList);
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
QueryValueLE::get_type() const
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
    if (!lb.empty() && limit > db.get_value_upper_bound(slot)) {
	RETURN(new EmptyPostList);
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
QueryValueGE::get_type() const
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

Xapian::termcount
QueryBranch::get_length() const
{
    // Sum results from all subqueries.
    Xapian::termcount result = 0;
    QueryVector::const_iterator i;
    for (i = subqueries.begin(); i != subqueries.end(); ++i) {
	// MatchNothing subqueries should have been removed by done().
	Assert((*i).internal.get());
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
	if (subqueries.size() < 9)
	    ch |= (subqueries.size() - 1);
	result += ch;
	if (subqueries.size() >= 9)
	    result += encode_length(subqueries.size() - 9);
	if (ch >= MULTIWAY(5))
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

    AssertRel(subqueries.size(), >=, 2);

    vector<PostList *> postlists;
    postlists.reserve(subqueries.size() - first);

    QueryVector::const_iterator q;
    for (q = subqueries.begin() + first; q != subqueries.end(); ++q) {
	// MatchNothing subqueries should have been removed by done().
	Assert((*q).internal.get());
	(*q).internal->postlist_sub_or_like(ctx, qopt, factor);
    }

    if (elite_set_size && elite_set_size < subqueries.size()) {
	ctx.select_elite_set(elite_set_size, subqueries.size());
	// FIXME: not right!
    }
}

PostList *
QueryBranch::do_synonym(QueryOptimiser * qopt, double factor) const
{
    LOGCALL(MATCH, PostList *, "QueryBranch::do_synonym", qopt | factor);
    OrContext ctx(subqueries.size());
    do_or_like(ctx, qopt, 0.0);
    PostList * pl = ctx.postlist(qopt);
    if (factor == 0.0) {
	// If we have a factor of 0, we don't care about the weights, so
	// we're just like a normal OR query.
	return pl;
    }

    // We currently assume wqf is 1 for calculating the synonym's weight
    // since conceptually the synonym is one "virtual" term.  If we were
    // to combine multiple occurrences of the same synonym expansion into
    // a single instance with wqf set, we would want to track the wqf.

    // We build an OP_OR tree for OP_SYNONYM and then wrap it in a
    // SynonymPostList, which supplies the weights.
    RETURN(qopt->make_synonym_postlist(pl, factor));
}

PostList *
QueryBranch::do_max(QueryOptimiser * qopt, double factor) const
{
    LOGCALL(MATCH, PostList *, "QueryBranch::do_max", qopt | factor);
    OrContext ctx(subqueries.size());
    do_or_like(ctx, qopt, factor);
    if (factor == 0.0) {
	// If we have a factor of 0, we don't care about the weights, so
	// we're just like a normal OR query.
	return ctx.postlist(qopt);
    }

    // We currently assume wqf is 1 for calculating the OP_MAX's weight
    // since conceptually the OP_MAX is one "virtual" term.  If we were
    // to combine multiple occurrences of the same OP_MAX expansion into
    // a single instance with wqf set, we would want to track the wqf.
    return ctx.postlist_max(qopt);
}

Xapian::Query::op
QueryBranch::get_type() const
{
    return get_op();
}

size_t
QueryBranch::get_num_subqueries() const
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
	    // Single occurrence probabilistic term without position set.
	    if (len >= 16) {
		result += static_cast<char>(0x40 | 0x10);
		result += encode_length(term.size() - 16);
	    } else {
		result += static_cast<char>(0x40 | 0x10 | len);
	    }
	    result += term;
	} else {
	    // Single occurrence probabilistic term with position set.
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
    const string & s = serialise_double(scale_factor);
    result += '\x0d';
    result += s;
    subquery.internal->serialise(result);
}

struct is_matchnothing {
    bool operator()(const Xapian::Query & q) {
	return q.internal.get() == NULL;
    }
};

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
    AndContext ctx(subqueries.size());
    postlist_sub_and_like(ctx, qopt, factor);
    RETURN(ctx.postlist(qopt));
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
    // If the left side of AndNot is already MatchNothing, do nothing.
    if (subqueries.size() == 1 && subqueries[0].internal.get() == NULL)
	return;
    // Drop any 2nd or subsequent subqueries which are MatchNothing.
    if (subquery.internal.get() != NULL || subqueries.empty())
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
    OrContext ctx(subqueries.size());
    do_or_like(ctx, qopt, factor);
    RETURN(ctx.postlist(qopt));
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
    // FIXME: Combine and-like side with and-like stuff above.
    AutoPtr<PostList> l(subqueries[0].internal->postlist(qopt, factor));
    OrContext ctx(subqueries.size() - 1);
    do_or_like(ctx, qopt, 0.0, 0, 1);
    AutoPtr<PostList> r(ctx.postlist(qopt));
    RETURN(new AndNotPostList(l.release(), r.release(),
			      qopt->matcher, qopt->db_size));
}

PostingIterator::Internal *
QueryXor::postlist(QueryOptimiser * qopt, double factor) const
{
    LOGCALL(QUERY, PostingIterator::Internal *, "QueryXor::postlist", qopt | factor);
    XorContext ctx(subqueries.size());
    postlist_sub_xor(ctx, qopt, factor);
    RETURN(ctx.postlist(qopt));
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
    OrContext ctx(subqueries.size() - 1);
    do_or_like(ctx, qopt, factor, 0, 1);
    AutoPtr<PostList> r(ctx.postlist(qopt));
    RETURN(new AndMaybePostList(l.release(), r.release(),
				qopt->matcher, qopt->db_size));
}

PostingIterator::Internal *
QueryFilter::postlist(QueryOptimiser * qopt, double factor) const
{
    LOGCALL(QUERY, PostingIterator::Internal *, "QueryFilter::postlist", qopt | factor);
    // FIXME: Combine and-like stuff, like QueryOptimiser.
    AssertEq(subqueries.size(), 2);
    PostList * pls[2];
    AutoPtr<PostList> l(subqueries[0].internal->postlist(qopt, factor));
    pls[1] = subqueries[1].internal->postlist(qopt, 0.0);
    pls[0] = l.release();
    RETURN(new MultiAndPostList(pls, pls + 2, qopt->matcher, qopt->db_size));
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
    // FIXME: should has_positions() be on the combined DB (not this sub)?
    if (qopt->db.has_positions()) {
	QueryVector::const_iterator i;
	for (i = subqueries.begin(); i != subqueries.end(); ++i) {
	    // MatchNothing subqueries should have been removed by done().
	    Assert((*i).internal.get());
	    // FIXME: postlist_sub_positional?
	    ctx.add_postlist((*i).internal->postlist(qopt, factor));
	}
	// Record the positional filter to apply higher up the tree.
	ctx.add_pos_filter(op, subqueries.size(), window);
    } else {
	QueryAndLike::postlist_sub_and_like(ctx, qopt, factor);
    }
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
    OrContext ctx(subqueries.size());
    do_or_like(ctx, qopt, factor, set_size);
    RETURN(ctx.postlist(qopt));
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
    // Synonym of a single subquery should only be simplified if that subquery
    // is a term (or MatchAll).  Note that MatchNothing subqueries are dropped,
    // so we'd never get here with a single MatchNothing subquery.
    if (subqueries.size() == 1) {
	Query::op sub_type = subqueries[0].get_type();
	if (sub_type == Query::LEAF_TERM || sub_type == Query::LEAF_MATCH_ALL)
	    return subqueries[0].internal.get();
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
    return get_description_helper(" SYNONYM ");
}

string
QueryMax::get_description() const
{
    return get_description_helper(" MAX ");
}

}
}
