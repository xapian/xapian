/** @file
 * @brief PostList class implementing Query::OP_AND_NOT
 */
/* Copyright 2017,2022 Olly Betts
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
