/** @file emptypostlist.cc
 * @brief A PostList which contains no entries.
 */
/* Copyright (C) 2009,2010,2011,2015 Olly Betts
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

#include "emptypostlist.h"

#include "omassert.h"

using namespace std;

Xapian::doccount
EmptyPostList::get_termfreq_min() const
{
    return 0;
}

Xapian::doccount
EmptyPostList::get_termfreq_max() const
{
    return 0;
}

Xapian::doccount
EmptyPostList::get_termfreq_est() const
{
    return 0;
}

// OP_WILDCARD which expands to no terms becomes EmptyPostList, and this is
// needed if that is used under OP_SYNONYM.
TermFreqs
EmptyPostList::get_termfreq_est_using_stats(const Xapian::Weight::Internal &) const
{
    return TermFreqs();
}

double
EmptyPostList::get_maxweight() const
{
    return 0;
}

Xapian::docid
EmptyPostList::get_docid() const
{
    Assert(false);
    return 0;
}

Xapian::termcount
EmptyPostList::get_doclength() const
{
    return Xapian::termcount(EmptyPostList::get_docid());
}

Xapian::termcount
EmptyPostList::get_unique_terms() const
{
    return Xapian::termcount(EmptyPostList::get_docid());
}

Xapian::termcount
EmptyPostList::get_wdfdocmax() const
{
    return Xapian::termcount(EmptyPostList::get_docid());
}

double
EmptyPostList::get_weight() const
{
    Assert(false);
    return 0;
}

bool
EmptyPostList::at_end() const
{
    return true;
}

double
EmptyPostList::recalc_maxweight()
{
    return EmptyPostList::get_maxweight();
}

PostList *
EmptyPostList::next(double)
{
    return NULL;
}

PostList *
EmptyPostList::skip_to(Xapian::docid, double)
{
    return NULL;
}

string
EmptyPostList::get_description() const
{
    return "EmptyPostList";
}
