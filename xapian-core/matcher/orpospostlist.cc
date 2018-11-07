/** @file orpospostlist.cc
 * @brief Wrapper postlist providing positions for an OR
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

#include "orpospostlist.h"

OrPosPostList::~OrPosPostList()
{
    delete pl;
}

Xapian::doccount
OrPosPostList::get_termfreq_min() const
{
    return pl->get_termfreq_min();
}

Xapian::doccount
OrPosPostList::get_termfreq_max() const
{
    return pl->get_termfreq_max();
}

Xapian::doccount
OrPosPostList::get_termfreq_est() const
{
    return pl->get_termfreq_est();
}

TermFreqs
OrPosPostList::get_termfreq_est_using_stats(const Xapian::Weight::Internal & stats) const
{
    return pl->get_termfreq_est_using_stats(stats);
}

double
OrPosPostList::get_maxweight() const
{
    return pl->get_maxweight();
}

Xapian::docid
OrPosPostList::get_docid() const
{
    return pl->get_docid();
}

Xapian::termcount
OrPosPostList::get_doclength() const
{
    return pl->get_doclength();
}

Xapian::termcount
OrPosPostList::get_unique_terms() const
{
    return pl->get_unique_terms();
}

double
OrPosPostList::get_weight() const
{
    return pl->get_weight();
}

bool
OrPosPostList::at_end() const
{
    return pl->at_end();
}

double
OrPosPostList::recalc_maxweight()
{
    return pl->recalc_maxweight();
}

PositionList *
OrPosPostList::read_position_list()
{
    return position_list.gather(pl);
}

PostList*
OrPosPostList::next(double w_min)
{
    PostList* result = pl->next(w_min);
    if (result) {
	delete pl;
	pl = result;
    }
    return NULL;
}

PostList*
OrPosPostList::skip_to(Xapian::docid did, double w_min)
{
    PostList* result = pl->skip_to(did, w_min);
    if (result) {
	delete pl;
	pl = result;
    }
    return NULL;
}

std::string
OrPosPostList::get_description() const
{
    string desc = "OrPosPostList(";
    desc += pl->get_description();
    desc += ')';
    return desc;
}

Xapian::termcount
OrPosPostList::get_wdf() const
{
    return pl->get_wdf();
}

Xapian::termcount
OrPosPostList::count_matching_subqs() const
{
    return pl->count_matching_subqs();
}
