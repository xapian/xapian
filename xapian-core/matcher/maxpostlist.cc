/** @file
 * @brief N-way OR postlist with wt=max(wt_i)
 */
/* Copyright (C) 2007,2009,2010,2011,2012,2013,2014,2017 Olly Betts
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

Xapian::docid
MaxPostList::get_docid() const
{
    return did;
}

double
MaxPostList::get_weight(Xapian::termcount doclen,
			Xapian::termcount unique_terms,
			Xapian::termcount wdfdocmax) const
{
    Assert(did);
    double res = 0.0;
    for (size_t i = 0; i < n_kids; ++i) {
	if (plist[i]->get_docid() == did)
	    res = max(res, plist[i]->get_weight(doclen,
						unique_terms,
						wdfdocmax));
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
    double result = plist[0]->recalc_maxweight();
    for (size_t i = 1; i < n_kids; ++i) {
	result = max(result, plist[i]->recalc_maxweight());
    }
    return result;
}

PostList *
MaxPostList::next(double w_min)
{
    Xapian::docid old_did = did;
    did = 0;
    for (size_t i = 0; i < n_kids; UNSIGNED_OVERFLOW_OK(++i)) {
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
		// erase_sublist(i) shuffles down i+1, etc down one index, so
		// the next sublist to deal with is also at index i, unless
		// this was the last index.  We deal with this by decrementing
		// i here and it'll be incremented by the loop, but this may
		// underflow (which is OK because i is an unsigned type).
		erase_sublist(UNSIGNED_OVERFLOW_OK(i--));
		continue;
	    }

	    if (res)
		matcher->force_recalc();

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
		matcher->force_recalc();

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

void
MaxPostList::get_docid_range(Xapian::docid& first, Xapian::docid& last) const
{
    plist[0]->get_docid_range(first, last);
    for (size_t i = 1; i != n_kids; ++i) {
	Xapian::docid f = 1, l = Xapian::docid(-1);
	plist[i]->get_docid_range(f, l);
	first = min(first, f);
	last = max(last, l);
    }
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
