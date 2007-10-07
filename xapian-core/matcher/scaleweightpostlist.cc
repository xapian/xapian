/** @file scaleweightpostlist.cc
 * @brief Return documents from a subquery with weights multiplied by a double.
 */
/* Copyright 2007 Lemur Consulting Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include "omassert.h"
#include "scaleweightpostlist.h"

// Include branchpostlist for utility functions "next_handling_prune" and
// "skip_to_handling_prune"
#include "branchpostlist.h"

Xapian::doccount
ScaleWeightPostList::get_termfreq_min() const
{
    return source->get_termfreq_min();
}

Xapian::doccount
ScaleWeightPostList::get_termfreq_est() const
{
    return source->get_termfreq_est();
}

Xapian::doccount
ScaleWeightPostList::get_termfreq_max() const
{
    return source->get_termfreq_max();
}

Xapian::weight
ScaleWeightPostList::get_maxweight() const
{
    return source->get_maxweight() * multiplier;
}

Xapian::docid
ScaleWeightPostList::get_docid() const
{
    return source->get_docid();
}

Xapian::weight
ScaleWeightPostList::get_weight() const
{
    return source->get_weight() * multiplier;
}

Xapian::doclength
ScaleWeightPostList::get_doclength() const
{
    return source->get_doclength();
}

Xapian::weight
ScaleWeightPostList::recalc_maxweight()
{
    return source->recalc_maxweight() * multiplier;
}

PositionList *
ScaleWeightPostList::read_position_list()
{
    return source->read_position_list();
}

PositionList *
ScaleWeightPostList::open_position_list() const
{
    return source->open_position_list();
}

PostList *
ScaleWeightPostList::next(Xapian::weight w_min)
{
    Assert(multiplier != 0);
    next_handling_prune(source, w_min / multiplier, matcher);
    return NULL;
}

PostList *
ScaleWeightPostList::skip_to(Xapian::docid did, Xapian::weight w_min)
{
    Assert(multiplier != 0);
    skip_to_handling_prune(source, did, w_min / multiplier, matcher);
    return NULL;
}

bool
ScaleWeightPostList::at_end() const
{
    return source->at_end();
}

string
ScaleWeightPostList::get_description() const
{
    string desc = "ScaleWeightPostList(";
    desc += source->get_description();
    desc += " * ";
    desc += om_tostring(multiplier);
    desc += ")";
    return desc;
}
