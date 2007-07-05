/** @file emptysubmatch.cc
 *  @brief SubMatch class for a dead remote database.
 */
/* Copyright (C) 2006,2007 Olly Betts
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

#include "emptypostlist.h"
#include "emptysubmatch.h"

#include <xapian/enquire.h>

bool
EmptySubMatch::prepare_match(bool /*nowait*/) {
    return true;
}

void
EmptySubMatch::start_match(Xapian::doccount, Xapian::doccount) {
}

PostList *
EmptySubMatch::get_postlist_and_term_info(MultiMatch *,
	map<string, Xapian::MSet::Internal::TermFreqAndWeight> *)
{
    LeafPostList * pl = new EmptyPostList;
    pl->set_termweight(new Xapian::BoolWeight);
    return pl;
}
