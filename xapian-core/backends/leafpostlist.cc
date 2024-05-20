/** @file
 * @brief Abstract base class for leaf postlists.
 */
/* Copyright (C) 2007,2009,2011,2013,2014,2017,2024 Olly Betts
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

#include "xapian/weight.h"

#include "leafpostlist.h"
#include "matcher/orpositionlist.h"
#include "omassert.h"
#include "debuglog.h"

using namespace std;

LeafPostList::~LeafPostList()
{
    delete weight;
}

double
LeafPostList::get_weight(Xapian::termcount doclen,
			 Xapian::termcount unique_terms,
			 Xapian::termcount wdfdocmax) const
{
    if (!weight) return 0;
    double sumpart = weight->get_sumpart(get_wdf(), doclen,
					 unique_terms, wdfdocmax);
    AssertRel(sumpart, <=, weight->get_maxpart());
    return sumpart;
}

double
LeafPostList::recalc_maxweight()
{
    return weight ? weight->get_maxpart() : 0;
}

Xapian::termcount
LeafPostList::count_matching_subqs() const
{
    return weight ? 1 : 0;
}

void
LeafPostList::gather_position_lists(OrPositionList* orposlist)
{
    orposlist->add_poslist(read_position_list());
}

bool
LeafPostList::open_nearby_postlist(std::string_view, bool, LeafPostList*&) const
{
    return false;
}
