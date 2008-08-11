/** @file leafpostlist.cc
 * @brief Abstract base class for leaf postlists.
 */
/* Copyright (C) 2007 Olly Betts
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

#include <xapian/enquire.h> // For Xapian::Weight.

#include "leafpostlist.h"
#include "omassert.h"

LeafPostList::~LeafPostList()
{
    delete weight;
}

Xapian::doccount
LeafPostList::get_termfreq_min() const
{
    return get_termfreq();
}

Xapian::doccount
LeafPostList::get_termfreq_max() const
{
    return get_termfreq();
}

Xapian::doccount
LeafPostList::get_termfreq_est() const
{
    return get_termfreq();
}

void
LeafPostList::set_termweight(const Xapian::Weight * weight_)
{
    // This method shouldn't be called more than once on the same object.
    Assert(!weight);
    weight = weight_;
    need_doclength = weight->get_sumpart_needs_doclength();
}

Xapian::weight
LeafPostList::get_maxweight() const
{
    return weight ? weight->get_maxpart() : 0;
}

Xapian::weight
LeafPostList::get_weight() const
{
    if (!weight) return 0;
    Xapian::doclength doclen = 0;
    // Fetching the document length is work we can avoid if the weighting
    // scheme doesn't use it.
    if (need_doclength) doclen = get_doclength();
    return weight->get_sumpart(get_wdf(), doclen);
}

Xapian::weight
LeafPostList::recalc_maxweight()
{
    return LeafPostList::get_maxweight();
}
