/** @file andnotpostlist.cc
 * @brief PostList class implementing Query::OP_AND_NOT
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

#include "andnotpostlist.h"

#include <algorithm>

using namespace std;

Xapian::doccount
AndNotPostList::get_termfreq_min() const
{
    Xapian::doccount l_tf_min = pl->get_termfreq_min();
    Xapian::doccount r_tf_max = r->get_termfreq_max();
    if (l_tf_min <= r_tf_max)
	return 0;
    return l_tf_min - r_tf_max;
}

Xapian::doccount
AndNotPostList::get_termfreq_max() const
{
    // We can't match more documents than our left-side does.
    Xapian::doccount l_tf_max = pl->get_termfreq_max();
    // We also can't more documents than our right-side *doesn't*.
    Xapian::doccount r_tf_min = r->get_termfreq_min();
    return min(db_size - r_tf_min, l_tf_max);
}

Xapian::doccount
AndNotPostList::get_termfreq_est() const
{
    if (rare(db_size == 0))
	return 0;
    // We calculate the estimate assuming independence.  With this assumption,
    // the estimate is the product of the estimates for the sub-postlists
    // (for the right side this is inverted by subtracting from db_size),
    // divided by db_size.
    double result = pl->get_termfreq_est();
    result = (result * (db_size - r->get_termfreq_est())) / db_size;
    return static_cast<Xapian::doccount>(result + 0.5);
}

TermFreqs
AndNotPostList::get_termfreq_est_using_stats(const Xapian::Weight::Internal & stats) const
{
    // We calculate the estimate assuming independence.  With this assumption,
    // the estimate is the product of the estimates for the sub-postlists
    // (for the right side this is inverted by subtracting from db_size),
    // divided by db_size.
    TermFreqs freqs(pl->get_termfreq_est_using_stats(stats));

    double freqest = double(freqs.termfreq);
    double relfreqest = double(freqs.reltermfreq);
    double collfreqest = double(freqs.collfreq);

    freqs = r->get_termfreq_est_using_stats(stats);

    // Our caller should have ensured this.
    Assert(stats.collection_size);

    freqs.termfreq = stats.collection_size - freqs.termfreq;
    freqest = (freqest * freqs.termfreq) / stats.collection_size;

    if (stats.total_length != 0) {
	freqs.collfreq = stats.total_length - freqs.collfreq;
	collfreqest = (collfreqest * freqs.collfreq) / stats.total_length;
    }

    // If the rset is empty, relfreqest should be 0 already, so leave
    // it alone.
    if (stats.rset_size != 0) {
	freqs.reltermfreq = stats.rset_size - freqs.reltermfreq;
	relfreqest = (relfreqest * freqs.reltermfreq) / stats.rset_size;
    }

    return TermFreqs(static_cast<Xapian::doccount>(freqest + 0.5),
		     static_cast<Xapian::doccount>(relfreqest + 0.5),
		     static_cast<Xapian::termcount>(collfreqest + 0.5));
}

PostList*
AndNotPostList::next(double w_min)
{
    while (true) {
	PostList* result = pl->next(w_min);
	if (result) {
	    delete pl;
	    pl = result;
	}
	if (pl->at_end()) {
	    result = pl;
	    pl = NULL;
	    return result;
	}
	Xapian::docid l_did = pl->get_docid();
	if (l_did > r_did) {
	    bool r_valid;
	    result = r->check(l_did, 0, r_valid);
	    if (result) {
		delete r;
		r = result;
	    }
	    if (!r_valid)
		return NULL;
	    if (r->at_end()) {
		result = pl;
		pl = NULL;
		return result;
	    }
	    r_did = r->get_docid();
	}
	if (l_did < r_did)
	    break;
    }
    return NULL;
}

PostList*
AndNotPostList::skip_to(Xapian::docid did, double w_min)
{
    if (did > pl->get_docid()) {
	PostList* result = pl->skip_to(did, w_min);
	if (result) {
	    delete pl;
	    pl = result;
	}
	if (pl->at_end()) {
	    result = pl;
	    pl = NULL;
	    return result;
	}
	Xapian::docid l_did = pl->get_docid();
	if (l_did > r_did) {
	    bool r_valid;
	    result = r->check(l_did, 0, r_valid);
	    if (result) {
		delete r;
		r = result;
	    }
	    if (!r_valid)
		return NULL;
	    if (r->at_end()) {
		result = pl;
		pl = NULL;
		return result;
	    }
	    r_did = r->get_docid();
	}
	if (l_did == r_did) {
	    // Advance to the next match.
	    return AndNotPostList::next(w_min);
	}
    }
    return NULL;
}

PostList*
AndNotPostList::check(Xapian::docid did, double w_min, bool& valid)
{
    PostList* result = pl->check(did, w_min, valid);
    if (result) {
	delete pl;
	pl = result;
    }
    if (valid) {
	if (pl->at_end()) {
	    result = pl;
	    pl = NULL;
	    return result;
	}
	Xapian::docid l_did = pl->get_docid();
	if (l_did > r_did) {
	    bool r_valid;
	    result = r->check(l_did, 0, r_valid);
	    if (result) {
		delete r;
		r = result;
	    }
	    if (!r_valid)
		return NULL;
	    if (r->at_end()) {
		result = pl;
		pl = NULL;
		return result;
	    }
	    r_did = r->get_docid();
	}
	if (l_did == r_did) {
	    // For check() we can simply indicate !valid.
	    valid = false;
	}
    }
    return NULL;
}

string
AndNotPostList::get_description() const
{
    string desc = "AndNotPostList(";
    desc += pl->get_description();
    desc += ", ";
    desc += r->get_description();
    desc += ')';
    return desc;
}
