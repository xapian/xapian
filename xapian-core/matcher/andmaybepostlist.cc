/** @file andmaybepostlist.cc
 * @brief PostList class implementing Query::OP_AND_MAYBE
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

#include "andmaybepostlist.h"

#include "multiandpostlist.h"

using namespace std;

PostList*
AndMaybePostList::decay_to_and(Xapian::docid did,
			       double w_min,
			       bool* valid_ptr)
{
    pl = new MultiAndPostList(pl, r, pl_max, r_max, pltree, db_size);
    r = NULL;
    PostList* result;
    if (valid_ptr) {
	result = pl->check(did, w_min, *valid_ptr);
    } else {
	result = pl->skip_to(did, w_min);
    }
    if (!result) {
	result = pl;
	pl = NULL;
    }
    pltree->force_recalc();
    return result;
}

Xapian::docid
AndMaybePostList::get_docid() const
{
    return pl_did;
}

double
AndMaybePostList::get_weight() const
{
    auto res = pl->get_weight();
    if (maybe_matches())
	res += r->get_weight();
    return res;
}

double
AndMaybePostList::recalc_maxweight()
{
    pl_max = pl->recalc_maxweight();
    r_max = r->recalc_maxweight();
    return pl_max + r_max;
}

PostList*
AndMaybePostList::next(double w_min)
{
    if (w_min > pl_max)
	return decay_to_and(max(pl_did, r_did) + 1, w_min);

    PostList* result = pl->next(w_min - r_max);
    if (result) {
	delete pl;
	pl = result;
    }
    if (pl->at_end()) {
	result = pl;
	pl = NULL;
	pltree->force_recalc();
	return result;
    }

    pl_did = pl->get_docid();
    if (pl_did > r_did) {
	bool r_valid;
	result = r->check(pl_did, w_min - pl_max, r_valid);
	if (result) {
	    delete r;
	    r = result;
	}
	if (!r_valid)
	    return NULL;
	if (r->at_end()) {
	    result = pl;
	    pl = NULL;
	    pltree->force_recalc();
	    return result;
	}
	r_did = r->get_docid();
    }
    return NULL;
}

PostList*
AndMaybePostList::skip_to(Xapian::docid did, double w_min)
{
    // skip_to(pl_did) happens after decay from OR
    if (did < pl_did)
	return NULL;

    if (w_min > pl_max) {
	// We dealt with did <= pl_did just above.
	return decay_to_and(max(did, r_did), w_min);
    }

    PostList* result = pl->skip_to(did, w_min - r_max);
    if (result) {
	delete pl;
	pl = result;
    }
    if (pl->at_end()) {
	result = pl;
	pl = NULL;
	pltree->force_recalc();
	return result;
    }
    pl_did = pl->get_docid();
    if (pl_did > r_did) {
	bool r_valid;
	result = r->check(pl_did, 0, r_valid);
	if (result) {
	    delete r;
	    r = result;
	}
	if (!r_valid)
	    return NULL;
	if (r->at_end()) {
	    result = pl;
	    pl = NULL;
	    pltree->force_recalc();
	    return result;
	}
	r_did = r->get_docid();
    }
    return NULL;
}

PostList*
AndMaybePostList::check(Xapian::docid did, double w_min, bool& valid)
{
    if (w_min > pl_max)
	return decay_to_and(max({did, pl_did, r_did}), w_min, &valid);

    PostList* result = pl->check(did, w_min - r_max, valid);
    if (result) {
	delete pl;
	pl = result;
    }
    if (valid) {
	if (pl->at_end()) {
	    result = pl;
	    pl = NULL;
	    pltree->force_recalc();
	    return result;
	}
	pl_did = pl->get_docid();
	if (pl_did > r_did) {
	    bool r_valid;
	    result = r->check(pl_did, 0, r_valid);
	    if (result) {
		delete r;
		r = result;
	    }
	    if (!r_valid)
		return NULL;
	    if (r->at_end()) {
		result = pl;
		pl = NULL;
		pltree->force_recalc();
		return result;
	    }
	    r_did = r->get_docid();
	}
    }
    return NULL;
}

string
AndMaybePostList::get_description() const
{
    string desc = "AndMaybePostList(";
    desc += pl->get_description();
    desc += ", ";
    desc += r->get_description();
    desc += ')';
    return desc;
}

Xapian::termcount
AndMaybePostList::get_wdf() const
{
    auto res = pl->get_wdf();
    if (maybe_matches())
	res += r->get_wdf();
    return res;
}

Xapian::termcount
AndMaybePostList::count_matching_subqs() const
{
    auto res = pl->count_matching_subqs();
    if (maybe_matches())
	res += r->count_matching_subqs();
    return res;
}

void
AndMaybePostList::gather_position_lists(OrPositionList* orposlist)
{
    pl->gather_position_lists(orposlist);
    if (maybe_matches())
	r->gather_position_lists(orposlist);
}
