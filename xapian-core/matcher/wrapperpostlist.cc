/** @file wrapperpostlist.cc
 * @brief Base class for a PostList which wraps another PostList
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

#include "wrapperpostlist.h"

Xapian::doccount
WrapperPostList::get_termfreq_min() const
{
    return pl->get_termfreq_min();
}

Xapian::doccount
WrapperPostList::get_termfreq_max() const
{
    return pl->get_termfreq_max();
}

Xapian::doccount
WrapperPostList::get_termfreq_est() const
{
    return pl->get_termfreq_est();
}

TermFreqs
WrapperPostList::get_termfreq_est_using_stats(
	const Xapian::Weight::Internal& stats) const
{
    return pl->get_termfreq_est_using_stats(stats);
}

double
WrapperPostList::get_maxweight() const
{
    return pl->get_maxweight();
}

Xapian::docid
WrapperPostList::get_docid() const
{
    return pl->get_docid();
}

Xapian::termcount
WrapperPostList::get_doclength() const
{
    return pl->get_doclength();
}

Xapian::termcount
WrapperPostList::get_unique_terms() const
{
    return pl->get_unique_terms();
}

double
WrapperPostList::get_weight() const
{
    return pl->get_weight();
}

bool
WrapperPostList::at_end() const
{
    return pl->at_end();
}

double
WrapperPostList::recalc_maxweight()
{
    return pl->recalc_maxweight();
}

PositionList*
WrapperPostList::read_position_list()
{
    return pl->read_position_list();
}

PostList*
WrapperPostList::next(double w_min)
{
    PostList* result = pl->next(w_min);
    if (result) {
	delete pl;
	pl = result;
    }
    return NULL;
}

PostList*
WrapperPostList::skip_to(Xapian::docid did, double w_min)
{
    PostList* result = pl->skip_to(did, w_min);
    if (result) {
	delete pl;
	pl = result;
    }
    return NULL;
}

std::string
WrapperPostList::get_description() const
{
    return pl->get_description();
}

Xapian::termcount
WrapperPostList::get_wdf() const
{
    return pl->get_wdf();
}

Xapian::termcount
WrapperPostList::count_matching_subqs() const
{
    return pl->count_matching_subqs();
}
