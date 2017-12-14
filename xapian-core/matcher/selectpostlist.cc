/** @file selectpostlist.cc
 * @brief Base class for classes which filter another PostList
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

#include "selectpostlist.h"

#include "omassert.h"
#include "postlisttree.h"

bool
SelectPostList::vet(double w_min)
{
    if (pl->at_end()) {
	delete pl;
	pl = NULL;
	return true;
    }

    // We assume that test_doc() is expensive compared to calculating the
    // weight.
    if (w_min <= 0.0) {
	cached_weight = -HUGE_VAL;
    } else {
	cached_weight = pltree->get_weight();
	if (cached_weight < w_min)
	    return false;
    }
    return test_doc();
}

double
SelectPostList::get_weight(Xapian::termcount doclen,
			   Xapian::termcount unique_terms) const
{
    if (cached_weight >= 0)
	return cached_weight;
    return pl->get_weight(doclen, unique_terms);
}

bool
SelectPostList::at_end() const
{
    return pl == NULL;
}

PostList*
SelectPostList::next(double w_min)
{
    do {
	PostList* result = pl->next(w_min);
	if (result) {
	    delete pl;
	    pl = result;
	}
    } while (!vet(w_min));
    return NULL;
}

PostList*
SelectPostList::skip_to(Xapian::docid did, double w_min)
{
    if (did > pl->get_docid()) {
	PostList* result = pl->skip_to(did, w_min);
	if (result) {
	    delete pl;
	    pl = result;
	}
	if (!vet(w_min)) {
	    // Advance to the next match.
	    return SelectPostList::next(w_min);
	}
    }
    return NULL;
}

PostList*
SelectPostList::check(Xapian::docid did, double w_min, bool& valid)
{
    PostList* result = pl->check(did, w_min, valid);
    if (result) {
	delete pl;
	pl = result;
    }
    if (valid) {
	// For check() we can simply indicate !valid if the vetting fails.
	valid = vet(w_min);
    }
    return NULL;
}

Xapian::doccount
SelectPostList::get_termfreq_min() const
{
    // In general, it's possible no documents get selected.  Subclasses where
    // that's known not to be the case should provide their own implementation.
    return 0;
}
