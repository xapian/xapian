/** @file multiandpostlist.cc
 * @brief N-way AND postlist
 */
/* Copyright (C) 2007,2009,2012 Olly Betts
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

#include "multiandpostlist.h"
#include "omassert.h"
#include "debuglog.h"

void
MultiAndPostList::allocate_plist_and_max_wt()
{
    plist = new PostList * [n_kids];
    try {
	max_wt = new Xapian::weight [n_kids];
    } catch (...) {
	delete [] plist;
	plist = NULL;
	throw;
    }
}

MultiAndPostList::~MultiAndPostList()
{
    if (plist) {
	for (size_t i = 0; i < n_kids; ++i) {
	    delete plist[i];
	}
	delete [] plist;
    }
    delete [] max_wt;
}

Xapian::doccount
MultiAndPostList::get_termfreq_min() const
{
    // The number of matching documents is minimised when we have the minimum
    // number of matching documents from each sub-postlist, and these are
    // maximally disjoint.
    Xapian::doccount sum = plist[0]->get_termfreq_min();
    if (sum) {
	for (size_t i = 1; i < n_kids; ++i) {
	    Xapian::doccount sum_old = sum;
	    sum += plist[i]->get_termfreq_min();
	    // If sum < sum_old, the calculation overflowed and the true sum
	    // must be > db_size.  Since we added a value <= db_size,
	    // subtracting db_size must un-overflow us.
	    if (sum >= sum_old && sum <= db_size) {
		// It's possible there's no overlap.
		return 0;
	    }
	    sum -= db_size;
	}
	AssertRelParanoid(sum,<=,MultiAndPostList::get_termfreq_est());
    }
    return sum;
}

Xapian::doccount
MultiAndPostList::get_termfreq_max() const
{
    // We can't match more documents than the least of our sub-postlists.
    Xapian::doccount result = plist[0]->get_termfreq_max();
    for (size_t i = 1; i < n_kids; ++i) {
	Xapian::doccount tf = plist[i]->get_termfreq_max();
	if (tf < result) result = tf;
    }
    return result;
}

Xapian::doccount
MultiAndPostList::get_termfreq_est() const
{
    if (rare(db_size == 0))
	return 0;
    // We calculate the estimate assuming independence.  With this assumption,
    // the estimate is the product of the estimates for the sub-postlists
    // divided by db_size (n_kids - 1) times.
    double result(plist[0]->get_termfreq_est());
    for (size_t i = 1; i < n_kids; ++i) {
	result = (result * plist[i]->get_termfreq_est()) / db_size;
    }
    return static_cast<Xapian::doccount>(result + 0.5);
}

TermFreqs
MultiAndPostList::get_termfreq_est_using_stats(
	const Xapian::Weight::Internal & stats) const 
{
    LOGCALL(MATCH, TermFreqs, "MultiAndPostList::get_termfreq_est_using_stats", stats);
    // We calculate the estimate assuming independence.  With this assumption,
    // the estimate is the product of the estimates for the sub-postlists
    // divided by db_size (n_kids - 1) times.
    TermFreqs freqs(plist[0]->get_termfreq_est_using_stats(stats));

    double freqest = double(freqs.termfreq);
    double relfreqest = double(freqs.reltermfreq);

    // Our caller should have ensured this.
    Assert(stats.collection_size);

    for (size_t i = 1; i < n_kids; ++i) {
	freqs = plist[i]->get_termfreq_est_using_stats(stats);

	// If the collection is empty, freqest should be 0 already, so leave
	// it alone.
	freqest = (freqest * freqs.termfreq) / stats.collection_size;

	// If the rset is empty, relfreqest should be 0 already, so leave
	// it alone.
	if (stats.rset_size != 0)
	    relfreqest = (relfreqest * freqs.reltermfreq) / stats.rset_size;
    }

    RETURN(TermFreqs(static_cast<Xapian::doccount>(freqest + 0.5),
		     static_cast<Xapian::doccount>(relfreqest + 0.5)));
}

Xapian::weight
MultiAndPostList::get_maxweight() const
{
    return max_total;
}

Xapian::docid
MultiAndPostList::get_docid() const
{
    return did;
}

Xapian::termcount
MultiAndPostList::get_doclength() const
{
    Assert(did);
    Xapian::termcount doclength = plist[0]->get_doclength();
    for (size_t i = 1; i < n_kids; ++i) {
	AssertEq(doclength, plist[i]->get_doclength());
    }
    return doclength;
}

Xapian::weight
MultiAndPostList::get_weight() const
{
    Assert(did);
    Xapian::weight result = 0;
    for (size_t i = 0; i < n_kids; ++i) {
	result += plist[i]->get_weight();
    }
    return result;
}

bool
MultiAndPostList::at_end() const
{
    return (did == 0);
}

Xapian::weight
MultiAndPostList::recalc_maxweight()
{
    max_total = 0.0;
    for (size_t i = 0; i < n_kids; ++i) {
	Xapian::weight new_max = plist[i]->recalc_maxweight();
	max_wt[i] = new_max;
	max_total += new_max;
    }
    return max_total;
}

PostList *
MultiAndPostList::find_next_match(Xapian::weight w_min)
{
advanced_plist0:
    if (plist[0]->at_end()) {
	did = 0;
	return NULL;
    }
    did = plist[0]->get_docid();
    for (size_t i = 1; i < n_kids; ++i) {
	bool valid;
	check_helper(i, did, w_min, valid);
	if (!valid) {
	    next_helper(0, w_min);
	    goto advanced_plist0;
	}
	if (plist[i]->at_end()) {
	    did = 0;
	    return NULL;
	}
	Xapian::docid new_did = plist[i]->get_docid();
	if (new_did != did) {
	    skip_to_helper(0, new_did, w_min);
	    goto advanced_plist0;
	}
    }
    return NULL;
}

PostList *
MultiAndPostList::next(Xapian::weight w_min)
{
    next_helper(0, w_min);
    return find_next_match(w_min);
}

PostList *
MultiAndPostList::skip_to(Xapian::docid did_min, Xapian::weight w_min)
{
    skip_to_helper(0, did_min, w_min);
    return find_next_match(w_min);
}

std::string
MultiAndPostList::get_description() const
{
    string desc("(");
    desc += plist[0]->get_description();
    for (size_t i = 1; i < n_kids; ++i) {
	desc += " AND ";
	desc += plist[i]->get_description();
    }
    desc += ')';
    return desc;
}

Xapian::termcount
MultiAndPostList::get_wdf() const
{
    Xapian::termcount totwdf = 0;
    for (size_t i = 0; i < n_kids; ++i) {
	totwdf += plist[i]->get_wdf();
    }
    return totwdf;
}

Xapian::termcount
MultiAndPostList::count_matching_subqs() const
{
    Xapian::termcount total = 0;
    for (size_t i = 0; i < n_kids; ++i) {
	total += plist[i]->count_matching_subqs();
    }
    return total;
}
