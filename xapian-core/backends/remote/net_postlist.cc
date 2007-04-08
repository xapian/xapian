/** @file net_postlist.cc
 *  @brief Postlists for remote databases
 */
/* Copyright (C) 2007 Lemur Consulting Ltd
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

#include "net_postlist.h"

using namespace std;

/** A postlist in a remote database.
 */
NetworkPostList::NetworkPostList(Xapian::Internal::RefCntPtr<const RemoteDatabase> db_,
				 const string & term_)
	: db(db_),
	term(term_),
	started(false),
	pos(NULL),
	pos_end(NULL),
	lastdocid(0),
	lastwdf(0),
	lastdoclen(0),
	termfreq(0)
{
    db->read_post_list(term, *this);
}

PositionList *
NetworkPostList::read_position_list()
{
    lastposlist = db->open_position_list(lastdocid, term);
    return lastposlist.get();
}

PositionList *
NetworkPostList::open_position_list() const
{
    return db->open_position_list(lastdocid, term);
}

PostList *
NetworkPostList::next(Xapian::weight)
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
	lastdocid += decode_length(&pos, pos_end, false) + 1;
	lastwdf = decode_length(&pos, pos_end, false);
	lastdoclen = unserialise_double(&pos, pos_end);
    }

    return NULL;
}

PostList *
NetworkPostList::skip_to(Xapian::docid did, Xapian::weight weight)
{
    if (pos == NULL)
	next(weight);
    while (pos != pos_end && lastdocid < did)
	next(weight);
    return NULL;
}

string
NetworkPostList::get_description() const
{
    return "NetworkPostList(" + term + ")";
}
