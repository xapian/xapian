/** @file
 * @brief N-way OR postlist with wt=max(wt_i)
 */
/* Copyright (C) 2007,2009,2010,2011,2012,2013,2014 Olly Betts
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

#include "maxpostlist.h"

#include "debuglog.h"
#include "omassert.h"

using namespace std;

MaxPostList::~MaxPostList()
{
    if (plist) {
	for (size_t i = 0; i < n_kids; ++i) {
	    delete plist[i];
	}
	delete [] plist;
    }
}

Xapian::doccount
MaxPostList::get_termfreq_min() const
{
    Xapian::doccount res = plist[0]->get_termfreq_min();
    for (size_t i = 1; i < n_kids; ++i) {
	res = max(res, plist[i]->get_termfreq_min());
    }
    return res;
}

Xapian::doccount
MaxPostList::get_termfreq_max() const
{
    Xapian::doccount res = plist[0]->get_termfreq_max();
    for (size_t i = 1; i < n_kids; ++i) {
	Xapian::doccount c = plist[i]->get_termfreq_max();
	if (db_size - res <= c)
	    return db_size;
	res += c;
    }
    return res;
}

Xapian::doccount
MaxPostList::get_termfreq_est() const
{
    // We shortcut an empty shard and avoid creating a postlist tree for it.
    Assert(db_size);

    // We calculate the estimate assuming independence.  The simplest
    // way to calculate this seems to be a series of (n_kids - 1) pairwise
    // calculations, which gives the same answer regardless of the order.
    double scale = 1.0 / db_size;
    double P_est = plist[0]->get_termfreq_est() * scale;
    for (size_t i = 1; i < n_kids; ++i) {
	double P_i = plist[i]->get_termfreq_est() * scale;
	P_est += P_i - P_est * P_i;
    }
    return static_cast<Xapian::doccount>(P_est * db_size + 0.5);
}

double
MaxPostList::get_maxweight() const
{
    return max_cached;
}

Xapian::docid
MaxPostList::get_docid() const
{
    return did;
}

Xapian::termcount
MaxPostList::get_doclength() const
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

Xapian::termcount
MaxPostList::get_unique_terms() const
{
    Assert(did);
    Xapian::termcount unique_terms = 0;
    bool unique_terms_set = false;
    for (size_t i = 0; i < n_kids; ++i) {
	if (plist[i]->get_docid() == did) {
	    if (unique_terms_set) {
		AssertEq(unique_terms, plist[i]->get_unique_terms());
	    } else {
		unique_terms = plist[i]->get_unique_terms();
		unique_terms_set = true;
	    }
	}
    }
    Assert(unique_terms_set);
    return unique_terms;
}

double
MaxPostList::get_weight() const
{
    Assert(did);
    double res = 0.0;
    for (size_t i = 0; i < n_kids; ++i) {
	if (plist[i]->get_docid() == did)
	    res = max(res, plist[i]->get_weight());
    }
    return res;
}

bool
MaxPostList::at_end() const
{
    return (did == 0);
}

double
MaxPostList::recalc_maxweight()
{
    max_cached = plist[0]->recalc_maxweight();
    for (size_t i = 1; i < n_kids; ++i) {
	max_cached = max(max_cached, plist[i]->recalc_maxweight());
    }
    return max_cached;
}

PostList *
MaxPostList::next(double w_min)
{
    Xapian::docid old_did = did;
    did = 0;
    for (size_t i = 0; i < n_kids; ++i) {
	Xapian::docid cur_did = 0;
	if (old_did != 0)
	    cur_did = plist[i]->get_docid();
	if (cur_did <= old_did) {
	    PostList * res;
	    if (old_did == 0 || cur_did == old_did) {
		res = plist[i]->next(w_min);
	    } else {
		res = plist[i]->skip_to(old_did + 1, w_min);
	    }
	    if (res) {
		delete plist[i];
		plist[i] = res;
	    }

	    if (plist[i]->at_end()) {
		erase_sublist(i--);
		continue;
	    }

	    if (res)
		matcher->recalc_maxweight();

	    cur_did = plist[i]->get_docid();
	}

	if (did == 0 || cur_did < did) {
	    did = cur_did;
	}
    }

    if (n_kids == 1) {
	n_kids = 0;
	return plist[0];
    }

    return NULL;
}

PostList *
MaxPostList::skip_to(Xapian::docid did_min, double w_min)
{
    Xapian::docid old_did = did;
    did = 0;
    for (size_t i = 0; i < n_kids; ++i) {
	Xapian::docid cur_did = 0;
	if (old_did != 0)
	    cur_did = plist[i]->get_docid();
	if (cur_did < did_min) {
	    PostList * res = plist[i]->skip_to(did_min, w_min);
	    if (res) {
		delete plist[i];
		plist[i] = res;
	    }

	    if (plist[i]->at_end()) {
		erase_sublist(i--);
		continue;
	    }

	    if (res)
		matcher->recalc_maxweight();

	    cur_did = plist[i]->get_docid();
	}

	if (did == 0 || cur_did < did) {
	    did = cur_did;
	}
    }

    if (n_kids == 1) {
	n_kids = 0;
	return plist[0];
    }

    return NULL;
}

string
MaxPostList::get_description() const
{
    string desc("(");
    desc += plist[0]->get_description();
    for (size_t i = 1; i < n_kids; ++i) {
	desc += " MAX ";
	desc += plist[i]->get_description();
    }
    desc += ')';
    return desc;
}

Xapian::termcount
MaxPostList::get_wdf() const
{
    Xapian::termcount totwdf = 0;
    for (size_t i = 0; i < n_kids; ++i) {
	if (plist[i]->get_docid() == did)
	    totwdf += plist[i]->get_wdf();
    }
    return totwdf;
}

Xapian::termcount
MaxPostList::count_matching_subqs() const
{
    return 1;
}
