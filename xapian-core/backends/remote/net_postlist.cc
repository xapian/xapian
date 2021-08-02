/** @file
 *  @brief Postlists for remote databases
 */
/* Copyright (C) 2007 Lemur Consulting Ltd
 * Copyright (C) 2007,2008,2009,2011,2012,2013,2015 Olly Betts
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

#include "net_postlist.h"
#include "net/length.h"
#include "omassert.h"
#include "unicode/description_append.h"

using namespace std;

Xapian::doccount
NetworkPostList::get_termfreq() const
{
    return termfreq;
}

Xapian::docid
NetworkPostList::get_docid() const
{
    return lastdocid;
}

Xapian::termcount
NetworkPostList::get_doclength() const
{
    return db->get_doclength(lastdocid);
}

Xapian::termcount
NetworkPostList::get_unique_terms() const
{
    return db->get_unique_terms(lastdocid);
}

Xapian::termcount
NetworkPostList::get_wdf() const
{
    return lastwdf;
}

PositionList *
NetworkPostList::open_position_list() const
{
    return db->open_position_list(lastdocid, term);
}

PostList *
NetworkPostList::next(double)
{
    if (!started) {
	started = true;
	pos = postings.data();
	pos_end = pos + postings.size();
	lastdocid = 0;
    }

    if (pos == pos_end) {
	pos = NULL;
    } else {
	Xapian::docid inc;
	decode_length(&pos, pos_end, inc);
	lastdocid += inc + 1;

	decode_length(&pos, pos_end, lastwdf);
    }

    return NULL;
}

PostList *
NetworkPostList::skip_to(Xapian::docid did, double min_weight)
{
    if (!started)
	next(min_weight);
    while (pos && lastdocid < did)
	next(min_weight);
    return NULL;
}

bool
NetworkPostList::at_end() const
{
    return (pos == NULL && started);
}

Xapian::termcount
NetworkPostList::get_wdf_upper_bound() const
{
    // This is only called when setting weights on PostList objects before
    // a match, which shouldn't happen to NetworkPostList objects (as remote
    // matching happens like a local match on the server).
    Assert(false);
    return 0;
}

string
NetworkPostList::get_description() const
{
    string desc = "NetworkPostList(";
    description_append(desc, term);
    desc += ')';
    return desc;
}
