/** @file extraweightpostlist.cc
 * @brief PostList which adds on a term-independent weight contribution
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

#include "extraweightpostlist.h"

#include "omassert.h"
#include "postlisttree.h"
#include "str.h"

using namespace std;

double
ExtraWeightPostList::get_weight() const
{
    /* Weight::get_sumextra() takes two parameters (document length and number
     * of unique terms) but none of the currently implemented weighting schemes
     * actually use the latter - it was added because it's likely to be wanted
     * at some point, and so that we got all the incompatible changes needed to
     * add support for the number of unique terms over with in one go.
     *
     * Currently we just pass zero for the number of unique terms, and always
     * pass the document length - an extra weight contribution which doesn't
     * depend on either can only really be a constant, and if it's constant it
     * might as well be zero (and note that if get_maxextra() returns zero,
     * then ExtraWeightPostList is not used).
     *
     * If a weighting scheme gets implemented that needs the number of unique
     * terms, then this code will need updating to hook into the "stats_needed"
     * mechanism which the Weight class has, so that we don't waste effort
     * calculating the number of unique terms for the existing weighting
     * schemes for which get_maxextra() returns non-zero.
     */
    Xapian::docid did = pl->get_docid();
    double sum_extra = weight->get_sumextra(db->get_doclength(did), 0.0);
    AssertRel(sum_extra,<=,max_extra);
    return pl->get_weight() + sum_extra;
}

double
ExtraWeightPostList::recalc_maxweight()
{
    return pl->recalc_maxweight() + max_extra;
}

PostList*
ExtraWeightPostList::next(double w_min)
{
    PostList* res = pl->next(w_min - max_extra);
    if (res) {
	delete pl;
	pl = res;
	pltree->force_recalc();
    }
    return NULL;
}

PostList*
ExtraWeightPostList::skip_to(Xapian::docid, double)
{
    // ExtraWeightPostList's parent will be either PostListTree or
    // MergePostList, neither of which use skip_to() or check().
    Assert(false);
    return NULL;
}

string
ExtraWeightPostList::get_description() const
{
    string desc = "ExtraWeightPostList(";
    desc += pl->get_description();
    desc += ", max_extra=";
    desc += str(max_extra);
    desc += ')';
    return desc;
}
