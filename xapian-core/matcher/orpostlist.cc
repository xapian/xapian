/** @file orpostlist.cc
 * @brief PostList class implementing Query::OP_OR
 */
/* Copyright 2017 Olly Betts
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

#include "orpostlist.h"

#include "andmaybepostlist.h"
#include "multiandpostlist.h"
#include "postlisttree.h"

#include <algorithm>

using namespace std;

PostList*
OrPostList::decay_to_and(Xapian::docid did,
			 double w_min,
			 bool* valid_ptr)
{
    l = new MultiAndPostList(l, r, l_max, r_max, pltree, db_size);
    r = NULL;
    PostList* result;
    if (valid_ptr) {
	result = l->check(did, w_min, *valid_ptr);
    } else {
	result = l->skip_to(did, w_min);
    }
    if (!result) {
	result = l;
	l = NULL;
    }
    pltree->force_recalc();
    return result;
}

PostList*
OrPostList::decay_to_andmaybe(PostList* left,
			      PostList* right,
			      Xapian::docid did,
			      double w_min,
			      bool* valid_ptr)
{
    if (l != left) swap(l_did, r_did);
    l = new AndMaybePostList(left, right, pltree, db_size, l_did, r_did);
    r = NULL;
    PostList* result;
    if (valid_ptr) {
	result = l->check(did, w_min, *valid_ptr);
    } else {
	result = l->skip_to(did, w_min);
    }
    if (!result) {
	result = l;
	l = NULL;
    }
    pltree->force_recalc();
    return result;
}

Xapian::doccount
OrPostList::get_termfreq_min() const
{
    return max(l->get_termfreq_min(), r->get_termfreq_min());
}

Xapian::docid
OrPostList::get_docid() const
{
    return min(l_did, r_did);
}

double
OrPostList::get_weight(Xapian::termcount doclen,
		       Xapian::termcount unique_terms) const
{
    if (l_did < r_did)
	return l->get_weight(doclen, unique_terms);
    if (l_did > r_did)
	return r->get_weight(doclen, unique_terms);
    return l->get_weight(doclen, unique_terms) +
	   r->get_weight(doclen, unique_terms);
}

double
OrPostList::recalc_maxweight()
{
    l_max = l->recalc_maxweight();
    r_max = r->recalc_maxweight();
    return l_max + r_max;
}

PostList*
OrPostList::next(double w_min)
{
    if (w_min > l_max) {
	if (w_min > r_max) {
	    // If l_did < r_did, r AND_MAYBE l might match at r_did.
	    Xapian::docid did = (l_did < r_did) ? r_did : r_did + 1;
	    return decay_to_and(did, w_min);
	}
	Xapian::docid did = (l_did == r_did) ? l_did + 1 : max(l_did, r_did);
	return decay_to_andmaybe(r, l, did, w_min);
    }
    if (w_min > r_max) {
	// If r_did < l_did, l AND_MAYBE r might match at l_did.
	Xapian::docid did = (r_did < l_did) ? l_did : l_did + 1;
	return decay_to_andmaybe(l, r, did, w_min);
    }

    bool advance_l = (l_did <= r_did);
    bool advance_r = (l_did >= r_did);

    if (advance_l) {
	PostList* result = l->next(w_min - r_max);
	if (result) {
	    delete l;
	    l = result;
	}
    }

    if (advance_r) {
	PostList* result = r->next(w_min - l_max);
	if (result) {
	    delete r;
	    r = result;
	}
    }

    if (advance_l) {
	if (l->at_end()) {
	    PostList* result = r;
	    r = NULL;
	    pltree->force_recalc();
	    return result;
	}
    }

    if (advance_r) {
	if (r->at_end()) {
	    PostList* result = l;
	    l = NULL;
	    pltree->force_recalc();
	    return result;
	}
    }

    if (advance_l) {
	l_did = l->get_docid();
    }

    if (advance_r) {
	r_did = r->get_docid();
    }

    return NULL;
}

PostList*
OrPostList::skip_to(Xapian::docid did, double w_min)
{
    // >= so skip_to(n) after check(n) -> !valid actually moves us on.
    bool advance_l = (did >= l_did);
    bool advance_r = (did >= r_did);
    if (!advance_l && !advance_r)
	return NULL;

    if (w_min > l_max) {
	if (w_min > r_max)
	    return decay_to_and(did, w_min);
	return decay_to_andmaybe(r, l, did, w_min);
    }
    if (w_min > r_max) {
	return decay_to_andmaybe(l, r, did, w_min);
    }

    if (advance_l) {
	PostList* result = l->skip_to(did, w_min - r_max);
	if (result) {
	    delete l;
	    l = result;
	}
    }

    if (advance_r) {
	PostList* result = r->skip_to(did, w_min - l_max);
	if (result) {
	    delete r;
	    r = result;
	}
    }

    if (advance_l) {
	if (l->at_end()) {
	    PostList* result = r;
	    r = NULL;
	    pltree->force_recalc();
	    return result;
	}
    }

    if (advance_r) {
	if (r->at_end()) {
	    PostList* result = l;
	    l = NULL;
	    pltree->force_recalc();
	    return result;
	}
    }

    if (advance_l) {
	l_did = l->get_docid();
    }

    if (advance_r) {
	r_did = r->get_docid();
    }

    return NULL;
}

PostList*
OrPostList::check(Xapian::docid did, double w_min, bool& valid)
{
    bool advance_l = (did > l_did);
    bool advance_r = (did > r_did);
    if (!advance_l && !advance_r)
	return NULL;

    if (w_min > l_max) {
	if (w_min > r_max)
	    return decay_to_and(did, w_min, &valid);
	return decay_to_andmaybe(r, l, did, w_min, &valid);
    }
    if (w_min > r_max) {
	return decay_to_andmaybe(l, r, did, w_min, &valid);
    }

    if (advance_l) {
	bool l_valid;
	PostList* result = l->check(did, w_min - r_max, l_valid);
	if (result) {
	    delete l;
	    l = result;
	} else if (!l_valid) {
	    l_did = did - 1;
	    advance_l = false;
	}
    }

    if (advance_r) {
	bool r_valid;
	PostList* result = r->check(did, w_min - l_max, r_valid);
	if (result) {
	    delete r;
	    r = result;
	} else if (!r_valid) {
	    r_did = did - 1;
	    advance_r = false;
	}
    }

    valid = advance_l || advance_r;
    if (!valid) {
	l_did = r_did = did;
	return NULL;
    }

    if (advance_l) {
	if (l->at_end()) {
	    PostList* result = r;
	    r = NULL;
	    pltree->force_recalc();
	    return result;
	}
    }

    if (advance_r) {
	if (r->at_end()) {
	    PostList* result = l;
	    l = NULL;
	    pltree->force_recalc();
	    return result;
	}
    }

    if (advance_l) {
	l_did = l->get_docid();
    }

    if (advance_r) {
	r_did = r->get_docid();
    }

    return NULL;
}

Xapian::doccount
OrPostList::get_termfreq_max() const
{
    auto l_tf_max = l->get_termfreq_max();
    auto r_tf_max = r->get_termfreq_max();
    auto tf_max = l_tf_max + r_tf_max;
    // If the sum is greater than the number of documents (or would have been
    // except it overflowed) then we can potentially match all documents.
    if (tf_max > db_size || tf_max < l_tf_max)
	tf_max = db_size;
    return tf_max;
}

template<typename T>
static void
estimate_or_assuming_indep(double a, double b, double n, T& res)
{
    if (rare(n == 0.0)) {
	res = 0;
    } else {
	res = static_cast<T>(a + b - (a * b / n) + 0.5);
    }
}

Xapian::doccount
OrPostList::get_termfreq_est() const
{
    auto l_tf_est = l->get_termfreq_est();
    auto r_tf_est = r->get_termfreq_est();
    Xapian::doccount tf_est;
    estimate_or_assuming_indep(l_tf_est, r_tf_est, db_size, tf_est);
    return tf_est;
}

TermFreqs
OrPostList::get_termfreq_est_using_stats(
	const Xapian::Weight::Internal& stats) const
{
    const TermFreqs& l_freqs = l->get_termfreq_est_using_stats(stats);
    const TermFreqs& r_freqs = r->get_termfreq_est_using_stats(stats);

    // Our caller should have ensured this.
    Assert(stats.collection_size);
    Xapian::doccount freqest;
    estimate_or_assuming_indep(l_freqs.termfreq,
			       r_freqs.termfreq,
			       stats.collection_size,
			       freqest);

    Xapian::doccount relfreqest = 0;
    if (stats.rset_size != 0) {
	estimate_or_assuming_indep(l_freqs.reltermfreq,
				   r_freqs.reltermfreq,
				   stats.rset_size,
				   relfreqest);
    }

    Xapian::termcount collfreqest = 0;
    if (stats.total_term_count != 0) {
	estimate_or_assuming_indep(l_freqs.collfreq,
				   r_freqs.collfreq,
				   stats.total_term_count,
				   collfreqest);
    }

    return TermFreqs(freqest, relfreqest, collfreqest);
}

bool
OrPostList::at_end() const
{
    // We never need to return true here - if one child reaches at_end(), we
    // prune to leave the other, and if both children reach at_end() together,
    // we prune to leave one of them which will then indicate at_end() for us.
    return false;
}

std::string
OrPostList::get_description() const
{
    string desc = "OrPostList(";
    desc += l->get_description();
    desc += ", ";
    desc += r->get_description();
    desc += ')';
    return desc;
}

Xapian::termcount
OrPostList::get_wdf() const
{
    if (l_did < r_did)
	return l->get_wdf();
    if (l_did > r_did)
	return r->get_wdf();
    return l->get_wdf() + r->get_wdf();
}

Xapian::termcount
OrPostList::count_matching_subqs() const
{
    if (l_did < r_did)
	return l->count_matching_subqs();
    if (l_did > r_did)
	return r->count_matching_subqs();
    return l->count_matching_subqs() + r->count_matching_subqs();
}

void
OrPostList::gather_position_lists(OrPositionList* orposlist)
{
    if (l_did <= r_did)
	l->gather_position_lists(orposlist);
    if (l_did >= r_did)
	r->gather_position_lists(orposlist);
}
