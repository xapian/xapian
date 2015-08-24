/** @file queryoptimiser.cc
 * @brief Convert a Xapian::Query::Internal tree into an optimal PostList tree.
 */
/* Copyright (C) 2007,2008,2009,2010 Olly Betts
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

#include "queryoptimiser.h"

#include "andmaybepostlist.h"
#include "andnotpostlist.h"
#include "const_database_wrapper.h"
#include "debuglog.h"
#include "emptypostlist.h"
#include "exactphrasepostlist.h"
#include "externalpostlist.h"
#include "multiandpostlist.h"
#include "multimatch.h"
#include "multixorpostlist.h"
#include "nearpostlist.h"
#include "omassert.h"
#include "omqueryinternal.h"
#include "orpostlist.h"
#include "phrasepostlist.h"
#include "postlist.h"
#include "valuegepostlist.h"
#include "valuerangepostlist.h"

#include <algorithm>
#include <functional>
#include <list>
#include <string>
#include <vector>

using namespace std;

PostList *
QueryOptimiser::do_subquery(const Xapian::Query::Internal * query, double factor)
{
    LOGCALL(MATCH, PostList *, "QueryOptimiser::do_subquery", query | factor);

    // Handle QueryMatchNothing.
    if (!query) RETURN(new EmptyPostList);

    switch (query->op) {
	case Xapian::Query::Internal::OP_LEAF:
	    if (factor != 0.0) {
		++total_subqs;
		if (query->tname.empty()) factor = 0.0;
	    }
	    RETURN(localsubmatch.postlist_from_op_leaf_query(query, factor));

	case Xapian::Query::Internal::OP_EXTERNAL_SOURCE: {
	    if (factor != 0.0)
		++total_subqs;
	    Assert(query->external_source);
	    Xapian::Database wrappeddb(new ConstDatabaseWrapper(&db));
	    RETURN(new ExternalPostList(wrappeddb,
					query->external_source, factor,
					matcher));
	}

	case Xapian::Query::OP_AND:
	case Xapian::Query::OP_FILTER:
	case Xapian::Query::OP_NEAR:
	case Xapian::Query::OP_PHRASE:
	    RETURN(do_and_like(query, factor));

	case Xapian::Query::OP_OR:
	case Xapian::Query::OP_XOR:
	case Xapian::Query::OP_ELITE_SET:
	    RETURN(do_or_like(query, factor));

	case Xapian::Query::OP_SYNONYM: {
	    // Save and restore total_subqs so we only add one for the whole
	    // OP_SYNONYM subquery (or none if we're not weighted).
	    Xapian::termcount save_total_subqs = total_subqs;
	    PostList * pl = do_synonym(query, factor);
	    total_subqs = save_total_subqs;
	    if (factor != 0.0)
		++total_subqs;
	    RETURN(pl);
	}

	case Xapian::Query::OP_AND_NOT: {
	    AssertEq(query->subqs.size(), 2);
	    PostList * l = do_subquery(query->subqs[0], factor);
	    PostList * r = do_subquery(query->subqs[1], 0.0);
	    RETURN(new AndNotPostList(l, r, matcher, db_size));
	}

	case Xapian::Query::OP_AND_MAYBE: {
	    AssertEq(query->subqs.size(), 2);
	    PostList * l = do_subquery(query->subqs[0], factor);
	    PostList * r = do_subquery(query->subqs[1], factor);
	    RETURN(new AndMaybePostList(l, r, matcher, db_size));
	}

	case Xapian::Query::OP_VALUE_RANGE: {
	    if (factor != 0.0)
		++total_subqs;
	    Xapian::valueno slot(query->parameter);
	    const string & range_begin = query->tname;
	    const string & range_end = query->str_parameter;
	    const string & lb = db.get_value_lower_bound(slot);
	    if (!lb.empty() &&
		(range_end < lb ||
		 range_begin > db.get_value_upper_bound(slot))) {
		RETURN(new EmptyPostList);
	    }
	    RETURN(new ValueRangePostList(&db, slot, range_begin, range_end));
	}

	case Xapian::Query::OP_VALUE_GE: {
	    if (factor != 0.0)
		++total_subqs;
	    Xapian::valueno slot(query->parameter);
	    const string & range_begin = query->tname;
	    const string & lb = db.get_value_lower_bound(slot);
	    if (!lb.empty() &&
		range_begin > db.get_value_upper_bound(slot)) {
		RETURN(new EmptyPostList);
	    }
	    RETURN(new ValueGePostList(&db, slot, range_begin));
	}

	case Xapian::Query::OP_VALUE_LE: {
	    if (factor != 0.0)
		++total_subqs;
	    Xapian::valueno slot(query->parameter);
	    const string & range_end = query->tname;
	    if (range_end < db.get_value_lower_bound(slot)) {
		RETURN(new EmptyPostList);
	    }
	    RETURN(new ValueRangePostList(&db, slot, string(), range_end));
	}

	case Xapian::Query::OP_SCALE_WEIGHT: {
	    AssertEq(query->subqs.size(), 1);
	    double sub_factor = factor;
	    if (sub_factor != 0.0) sub_factor *= query->get_dbl_parameter();
	    RETURN(do_subquery(query->subqs[0], sub_factor));
	}

	default:
	    Assert(false);
	    RETURN(NULL);
    }
}

struct PosFilter {
    PosFilter(Xapian::Query::Internal::op_t op_, size_t begin_, size_t end_,
	      Xapian::termcount window_)
	: op(op_), begin(begin_), end(end_), window(window_) { }

    Xapian::Query::Internal::op_t op;

    /// Start and end indices for the PostLists this positional filter uses.
    size_t begin, end;

    Xapian::termcount window;
};

PostList *
QueryOptimiser::do_and_like(const Xapian::Query::Internal *query, double factor)
{
    LOGCALL(MATCH, PostList *, "QueryOptimiser::do_and_like", query | factor);

    list<PosFilter> pos_filters;
    vector<PostList *> plists;
    do_and_like(query, factor, plists, pos_filters);
    AssertRel(plists.size(), >=, 2);

    PostList * pl;
    pl = new MultiAndPostList(plists.begin(), plists.end(), matcher, db_size);

    // Sort the positional filters to try to apply them in an efficient order.
    // FIXME: We need to figure out what that is!  Try applying lowest cf/tf
    // first?

    // Apply any positional filters.
    list<PosFilter>::iterator i;
    for (i = pos_filters.begin(); i != pos_filters.end(); ++i) {
	const PosFilter & filter = *i;

	vector<PostList *>::const_iterator terms_begin = plists.begin() + filter.begin;
	vector<PostList *>::const_iterator terms_end = plists.begin() + filter.end;

	Xapian::termcount window = filter.window;
	if (filter.op == Xapian::Query::OP_NEAR) {
	    pl = new NearPostList(pl, window, terms_begin, terms_end);
	} else if (window == filter.end - filter.begin) {
	    AssertEq(filter.op, Xapian::Query::OP_PHRASE);
	    pl = new ExactPhrasePostList(pl, terms_begin, terms_end);
	} else {
	    AssertEq(filter.op, Xapian::Query::OP_PHRASE);
	    pl = new PhrasePostList(pl, window, terms_begin, terms_end);
	}
    }

    RETURN(pl);
}

inline bool is_and_like(Xapian::Query::Internal::op_t op) {
    return op == Xapian::Query::OP_AND || op == Xapian::Query::OP_FILTER ||
	   op == Xapian::Query::OP_NEAR || op == Xapian::Query::OP_PHRASE;
}

void
QueryOptimiser::do_and_like(const Xapian::Query::Internal *query, double factor,
			    vector<PostList *> & and_plists,
			    list<PosFilter> & pos_filters)
{
    LOGCALL_VOID(MATCH, "QueryOptimiser::do_and_like", query | factor | and_plists | pos_filters);

    Xapian::Query::Internal::op_t op = query->op;
    Assert(is_and_like(op));

    bool positional = false;
    if (op == Xapian::Query::OP_PHRASE || op == Xapian::Query::OP_NEAR) {
	// If this sub-database has no positional information, change
	// OP_PHRASE/OP_NEAR into OP_AND so that we actually return some
	// matches.
	if (!db.has_positions()) {
	    op = Xapian::Query::OP_AND;
	} else {
	    positional = true;
	}
    }

    const Xapian::Query::Internal::subquery_list &queries = query->subqs;
    AssertRel(queries.size(), >=, 2);

    for (size_t i = 0; i != queries.size(); ++i) {
	// The second branch of OP_FILTER is always boolean.
	if (i == 1 && op == Xapian::Query::OP_FILTER) factor = 0.0;

	const Xapian::Query::Internal * subq = queries[i];
	if (is_and_like(subq->op)) {
	    do_and_like(subq, factor, and_plists, pos_filters);
	} else {
	    PostList * pl = do_subquery(subq, factor);
	    and_plists.push_back(pl);
	}
    }

    if (positional) {
	// Record the positional filter to apply higher up the tree.
	size_t end = and_plists.size();
	size_t begin = end - queries.size();
	Xapian::termcount window = query->parameter;

	pos_filters.push_back(PosFilter(op, begin, end, window));
    }
}

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

template<class CLASS> struct delete_ptr {
    void operator()(CLASS *p) { delete p; }
};

/// Comparison functor which orders PostList* by descending get_termfreq_est().
struct ComparePostListTermFreqAscending {
    /// Order by descending get_termfreq_est().
    bool operator()(const PostList *a, const PostList *b) {
	return a->get_termfreq_est() > b->get_termfreq_est();
    }
};

PostList *
QueryOptimiser::do_or_like(const Xapian::Query::Internal *query, double factor)
{
    LOGCALL(MATCH, PostList *, "QueryOptimiser::do_or_like", query | factor);

    // FIXME: we could optimise by merging OP_ELITE_SET and OP_OR like we do
    // for AND-like operations.
    Xapian::Query::Internal::op_t op = query->op;
    Assert(op == Xapian::Query::OP_ELITE_SET || op == Xapian::Query::OP_OR ||
	   op == Xapian::Query::OP_XOR || op == Xapian::Query::OP_SYNONYM);

    const Xapian::Query::Internal::subquery_list &queries = query->subqs;
    AssertRel(queries.size(), >=, 2);

    vector<PostList *> postlists;
    postlists.reserve(queries.size());

    Xapian::Query::Internal::subquery_list::const_iterator q;
    for (q = queries.begin(); q != queries.end(); ++q) {
	postlists.push_back(do_subquery(*q, factor));
    }

    if (op == Xapian::Query::OP_XOR) {
	RETURN(new MultiXorPostList(postlists.begin(), postlists.end(),
				    matcher, db_size));
    }

    if (op == Xapian::Query::OP_ELITE_SET) {
	// Select the best elite_set_size terms.
	Xapian::termcount elite_set_size = query->parameter;
	Assert(elite_set_size > 0);

	if (postlists.size() > elite_set_size) {
	    // Call recalc_maxweight() as otherwise get_maxweight()
	    // may not be valid before next() or skip_to()
	    for_each(postlists.begin(), postlists.end(),
		     mem_fun(&PostList::recalc_maxweight));

	    nth_element(postlists.begin(),
			postlists.begin() + elite_set_size - 1,
			postlists.end(), CmpMaxOrTerms());

	    for_each(postlists.begin() + elite_set_size, postlists.end(),
		     delete_ptr<PostList>());

	    if (elite_set_size == 1) RETURN(postlists[0]);

	    postlists.resize(elite_set_size);
	}
    }

    // Make postlists into a heap so that the postlist with the greatest term
    // frequency is at the top of the heap.
    make_heap(postlists.begin(), postlists.end(),
	      ComparePostListTermFreqAscending());

    // Now build a tree of binary OrPostList objects.
    //
    // The algorithm used to build the tree is like that used to build an
    // optimal Huffman coding tree.  If we called next() repeatedly, this
    // arrangement would minimise the number of method calls.  Generally we
    // don't actually do that, but this arrangement is still likely to be a
    // good one, and it does minimise the work in the worst case.
    AssertRel(postlists.size(), >=, 2);
    while (true) {
	// We build the tree such that at each branch:
	//
	//   l.get_termfreq_est() >= r.get_termfreq_est()
	//
	// We do this so that the OrPostList class can be optimised assuming
	// that this is the case.
	PostList * r = postlists.front();
	pop_heap(postlists.begin(), postlists.end(),
		 ComparePostListTermFreqAscending());
	postlists.pop_back();
	PostList * pl = new OrPostList(postlists.front(), r, matcher, db_size);

	if (postlists.size() == 1) RETURN(pl);

	pop_heap(postlists.begin(), postlists.end(),
		 ComparePostListTermFreqAscending());
	postlists.back() = pl;
	push_heap(postlists.begin(), postlists.end(),
		  ComparePostListTermFreqAscending());
    }
}

PostList *
QueryOptimiser::do_synonym(const Xapian::Query::Internal *query, double factor)
{
    LOGCALL(MATCH, PostList *, "QueryOptimiser::do_synonym", query | factor);
    if (factor == 0.0) {
	// If we have a factor of 0, we don't care about the weights, so
	// we're just like a normal OR query.
	RETURN(do_or_like(query, 0.0));
    }

    // We currently assume wqf is 1 for calculating the synonym's weight
    // since conceptually the synonym is one "virtual" term.  If we were
    // to combine multiple occurrences of the same synonym expansion into
    // a single instance with wqf set, we would want to use the wqf.
    AssertEq(query->get_wqf(), 0);

    // We build an OP_OR tree for OP_SYNONYM and then wrap it in a
    // SynonymPostList, which supplies the weights.
    RETURN(localsubmatch.make_synonym_postlist(do_or_like(query, 0.0),
					       matcher, factor));
}
