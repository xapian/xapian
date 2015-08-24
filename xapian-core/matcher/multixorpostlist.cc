/** @file multixorpostlist.cc
 * @brief N-way XOR postlist
 */
/* Copyright (C) 2007,2009,2010,2012 Olly Betts
 * Copyright (C) 2009 Lemur Consulting Ltd
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

#include "multixorpostlist.h"

#include "debuglog.h"
#include "multimatch.h"
#include "omassert.h"

using namespace std;

MultiXorPostList::~MultiXorPostList()
{
    if (plist) {
	for (size_t i = 0; i < n_kids; ++i) {
	    delete plist[i];
	}
	delete [] plist;
    }
}

Xapian::doccount
MultiXorPostList::get_termfreq_min() const
{
    // The number of matching documents is minimised when we have the maximum
    // even overlap between the sub-postlists, but that's rather tricky to
    // calculate in general, so just return 0 for now.
    //
    // FIXME: we can certainly work out a better bound in at least some cases
    // (e.g. when tf_min(X) > sum(i!=X){ tf_max(i) } for some sub-pl X).
    return 0;
}

Xapian::doccount
MultiXorPostList::get_termfreq_max() const
{
    // Maximum is if all sub-postlists are disjoint.
    Xapian::doccount result = plist[0]->get_termfreq_max();
    bool all_exact = (result == plist[0]->get_termfreq_min());
    bool overflow = false;
    for (size_t i = 1; i < n_kids; ++i) {
	Xapian::doccount tf_max = plist[i]->get_termfreq_max();
	Xapian::doccount old_result = result;
	result += tf_max;
	// Catch overflowing the type too.
	if (result < old_result)
	    overflow = true;
	if (all_exact)
	    all_exact = (tf_max == plist[i]->get_termfreq_min());
	if (!all_exact && (overflow || result >= db_size))
	    return db_size;
    }
    if (all_exact && (overflow || result > db_size)) {
	// If the sub-postlist tfs are all exact, then if the sum of them has
	// a different odd/even-ness to db_size then max tf of the XOR can't
	// achieve db_size.
	return db_size - ((result & 1) == (db_size & 1));
    }
    return result;
}

Xapian::doccount
MultiXorPostList::get_termfreq_est() const
{
    if (rare(db_size == 0))
	return 0;
    // We calculate the estimate assuming independence.  The simplest
    // way to calculate this seems to be a series of (n_kids - 1) pairwise
    // calculations, which gives the same answer regardless of the order.
    double scale = 1.0 / db_size;
    double P_est = plist[0]->get_termfreq_est() * scale;
    for (size_t i = 1; i < n_kids; ++i) {
	double P_i = plist[i]->get_termfreq_est() * scale;
	P_est += P_i - 2.0 * P_est * P_i;
    }
    return static_cast<Xapian::doccount>(P_est * db_size + 0.5);
}

TermFreqs
MultiXorPostList::get_termfreq_est_using_stats(
	const Xapian::Weight::Internal & stats) const
{
    LOGCALL(MATCH, TermFreqs, "MultiXorPostList::get_termfreq_est_using_stats", stats);
    // We calculate the estimate assuming independence.  The simplest
    // way to calculate this seems to be a series of (n_kids - 1) pairwise
    // calculations, which gives the same answer regardless of the order.
    TermFreqs freqs(plist[0]->get_termfreq_est_using_stats(stats));

    // Our caller should have ensured this.
    Assert(stats.collection_size);
    double scale = 1.0 / stats.collection_size;
    double P_est = freqs.termfreq * scale;
    double Pr_est = freqs.reltermfreq * scale;

    for (size_t i = 1; i < n_kids; ++i) {
	double P_i = freqs.termfreq * scale;
	P_est += P_i - 2.0 * P_est * P_i;
	// If the rset is empty, Pr_est should be 0 already, so leave
	// it alone.
	if (stats.rset_size != 0) {
	    double Pr_i = freqs.reltermfreq / stats.rset_size;
	    Pr_est += Pr_i - 2.0 * Pr_est * Pr_i;
	}
    }
    RETURN(TermFreqs(Xapian::doccount(P_est * stats.collection_size + 0.5),
		     Xapian::doccount(Pr_est * stats.rset_size + 0.5)));
}

Xapian::weight
MultiXorPostList::get_maxweight() const
{
    LOGCALL(MATCH, Xapian::weight, "MultiXorPostList::get_maxweight", NO_ARGS);
    RETURN(max_total);
}

Xapian::docid
MultiXorPostList::get_docid() const
{
    return did;
}

Xapian::termcount
MultiXorPostList::get_doclength() const
{
    Assert(did);
    Xapian::termcount doclength = 0;
    bool doclength_set = false;
    for (size_t i = 0; i < n_kids; ++i) {
	if (plist[i]->get_docid() == did) {
	    if (doclength_set) {
		AssertEq(doclength, plist[i]->get_doclength());
	    } else {
		doclength = plist[i]->get_doclength();
		doclength_set = true;
	    }
	}
    }
    Assert(doclength_set);
    return doclength;
}

Xapian::weight
MultiXorPostList::get_weight() const
{
    Assert(did);
    Xapian::weight result = 0;
    for (size_t i = 0; i < n_kids; ++i) {
	if (plist[i]->get_docid() == did)
	    result += plist[i]->get_weight();
    }
    return result;
}

bool
MultiXorPostList::at_end() const
{
    return (did == 0);
}

Xapian::weight
MultiXorPostList::recalc_maxweight()
{
    LOGCALL(MATCH, Xapian::weight, "MultiXorPostList::recalc_maxweight", NO_ARGS);
    max_total = plist[0]->recalc_maxweight();
    double min_max = max_total;
    for (size_t i = 1; i < n_kids; ++i) {
	Xapian::weight new_max = plist[i]->recalc_maxweight();
	if (new_max < min_max)
	    min_max = new_max;
	max_total += new_max;
    }
    if ((n_kids & 1) == 0) {
	// If n_kids is even, we omit the child with the smallest maxweight.
	max_total -= min_max;
    }
    RETURN(max_total);
}

PostList *
MultiXorPostList::next(Xapian::weight w_min)
{
    LOGCALL(MATCH, PostList *, "MultiXorPostList::next", w_min);
    Xapian::docid old_did = did;
    did = 0;
    size_t matching_count = 0;
    for (size_t i = 0; i < n_kids; ++i) {
	if (old_did == 0 || plist[i]->get_docid() <= old_did) {
	    // FIXME: calculate the min weight required here...
	    PostList * res = plist[i]->next(0);
	    if (res) {
		delete plist[i];
		plist[i] = res;
		matcher->recalc_maxweight();
	    }

	    if (plist[i]->at_end()) {
		erase_sublist(i--);
		continue;
	    }
	}

	Xapian::docid new_did = plist[i]->get_docid();
	if (did == 0 || new_did < did) {
	    did = new_did;
	    matching_count = 1;
	} else if (new_did == did) {
	    ++matching_count;
	}
    }

    if (n_kids == 1) {
	n_kids = 0;
	RETURN(plist[0]);
    }

    // We've reached the end of all posting lists.
    if (did == 0)
	RETURN(NULL);

    // An odd number of sub-postlists match this docid, so the XOR matches.
    if (matching_count & 1)
	RETURN(NULL);

    // An even number of sub-postlists match this docid, so advance again.
    RETURN(next(w_min));
}

PostList *
MultiXorPostList::skip_to(Xapian::docid did_min, Xapian::weight w_min)
{
    LOGCALL(MATCH, PostList *, "MultiXorPostList::skip_to", did_min | w_min);
    Xapian::docid old_did = did;
    did = 0;
    size_t matching_count = 0;
    for (size_t i = 0; i < n_kids; ++i) {
	if (old_did == 0 || plist[i]->get_docid() < did_min) {
	    // FIXME: calculate the min weight required here...
	    PostList * res = plist[i]->skip_to(did_min, 0);
	    if (res) {
		delete plist[i];
		plist[i] = res;
		matcher->recalc_maxweight();
	    }

	    if (plist[i]->at_end()) {
		erase_sublist(i--);
		continue;
	    }
	}

	Xapian::docid new_did = plist[i]->get_docid();
	if (did == 0 || new_did < did) {
	    did = new_did;
	    matching_count = 1;
	} else if (new_did == did) {
	    ++matching_count;
	}
    }

    if (n_kids == 1) {
	AssertEq(matching_count, 1);
	n_kids = 0;
	RETURN(plist[0]);
    }

    // We've reached the end of all posting lists.
    if (did == 0)
	RETURN(NULL);

    // An odd number of sub-postlists match this docid, so the XOR matches.
    if (matching_count & 1)
	RETURN(NULL);

    // An even number of sub-postlists match this docid, so call next.
    RETURN(next(w_min));
}

string
MultiXorPostList::get_description() const
{
    string desc("(");
    desc += plist[0]->get_description();
    for (size_t i = 1; i < n_kids; ++i) {
	desc += " XOR ";
	desc += plist[i]->get_description();
    }
    desc += ')';
    return desc;
}

Xapian::termcount
MultiXorPostList::get_wdf() const
{
    Xapian::termcount totwdf = 0;
    for (size_t i = 0; i < n_kids; ++i) {
	if (plist[i]->get_docid() == did)
	    totwdf += plist[i]->get_wdf();
    }
    return totwdf;
}

Xapian::termcount
MultiXorPostList::count_matching_subqs() const
{
    Xapian::termcount total = 0;
    for (size_t i = 0; i < n_kids; ++i) {
	if (plist[i]->get_docid() == did)
	    total += plist[i]->count_matching_subqs();
    }
    return total;
}
