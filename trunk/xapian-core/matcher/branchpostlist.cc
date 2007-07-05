/** @file branchpostlist.cc
 * @brief Virtual base class for branched types of postlist.
 */
/* Copyright (C) 2007 Lemur Consulting Ltd
 * Copyright (C) 2007 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include "branchpostlist.h"

BranchPostList::~BranchPostList()
{
    delete l;
    delete r;
}

Xapian::termcount
BranchPostList::get_wdf() const
{
    return l->get_wdf() + r->get_wdf();
}

PositionList *
BranchPostList::read_position_list()
{
    throw Xapian::UnimplementedError("BranchPostList::read_position_list() not implemented");
}

PositionList *
BranchPostList::open_position_list() const
{
    throw Xapian::UnimplementedError("BranchPostList::open_position_list() not implemented");
}
