/** @file
 * @brief N-way XOR postlist
 */
/* Copyright (C) 2007,2009,2010,2011,2012,2016,2017 Olly Betts
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

#include "xorpostlist.h"

#include "debuglog.h"
#include "omassert.h"

#include <algorithm>

using namespace std;

XorPostList::~XorPostList()
{
    if (plist) {
	for (size_t i = 0; i < n_kids; ++i) {
	    delete plist[i];
	}
	delete [] plist;
    }
}

Xapian::docid
XorPostList::get_docid() const
{
    return did;
}

double
XorPostList::get_weight(Xapian::termcount doclen,
			Xapian::termcount unique_terms,
			Xapian::termcount wdfdocmax) const
{
    Assert(did);
    double result = 0;
    for (size_t i = 0; i < n_kids; ++i) {
	if (plist[i]->get_docid() == did)
	    result += plist[i]->get_weight(doclen, unique_terms, wdfdocmax);
    }
    return result;
}

bool
XorPostList::at_end() const
{
    return (did == 0);
}

double
XorPostList::recalc_maxweight()
{
    LOGCALL(MATCH, double, "XorPostList::recalc_maxweight", NO_ARGS);
    double max_total = plist[0]->recalc_maxweight();
    double min_max = max_total;
    for (size_t i = 1; i < n_kids; ++i) {
	double new_max = plist[i]->recalc_maxweight();
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
XorPostList::next(double w_min)
{
    LOGCALL(MATCH, PostList*, "XorPostList::next", w_min);
    Xapian::docid old_did = did;
    did = 0;
    size_t matching_count = 0;
    for (size_t i = 0; i < n_kids; UNSIGNED_OVERFLOW_OK(++i)) {
	if (old_did == 0 || plist[i]->get_docid() <= old_did) {
	    // FIXME: calculate the min weight required here...
	    PostList * res = plist[i]->next(0);
	    if (res) {
		delete plist[i];
		plist[i] = res;
		matcher->force_recalc();
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
XorPostList::skip_to(Xapian::docid did_min, double w_min)
{
    LOGCALL(MATCH, PostList*, "XorPostList::skip_to", did_min | w_min);
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
		matcher->force_recalc();
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

void
XorPostList::get_docid_range(Xapian::docid& first, Xapian::docid& last) const
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
XorPostList::get_description() const
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
XorPostList::get_wdf() const
{
    Xapian::termcount totwdf = 0;
    for (size_t i = 0; i < n_kids; ++i) {
	if (plist[i]->get_docid() == did)
	    totwdf += plist[i]->get_wdf();
    }
    return totwdf;
}

Xapian::termcount
XorPostList::count_matching_subqs() const
{
    Xapian::termcount total = 0;
    for (size_t i = 0; i < n_kids; ++i) {
	if (plist[i]->get_docid() == did)
	    total += plist[i]->count_matching_subqs();
    }
    return total;
}
