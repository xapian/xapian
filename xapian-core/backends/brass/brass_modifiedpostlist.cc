/** @file brass_modifiedpostlist.cc
 * @brief A BrassPostList plus pending modifications
 */
/* Copyright (C) 2006,2007,2008,2009 Olly Betts
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

#include "brass_database.h"
#include "brass_modifiedpostlist.h"

BrassModifiedPostList::~BrassModifiedPostList()
{
    delete poslist;
}

void
BrassModifiedPostList::skip_deletes(Xapian::weight w_min)
{
    while (!BrassPostList::at_end()) {
	while (it != mods.end() && it->second.first == 'D' &&
	       it->first < BrassPostList::get_docid())
	    ++it;
	if (it == mods.end()) return;
	if (it->first != BrassPostList::get_docid()) return;
	if (it->second.first != 'D') return;
	++it;
	BrassPostList::next(w_min);
    }
    while (it != mods.end() && it->second.first == 'D') ++it;
}

Xapian::doccount
BrassModifiedPostList::get_termfreq() const
{
    return this_db->get_termfreq(term);
}

Xapian::docid
BrassModifiedPostList::get_docid() const
{
    if (it == mods.end()) return BrassPostList::get_docid();
    if (BrassPostList::at_end()) return it->first;
    Assert(it->second.first != 'D');
    return min(it->first, BrassPostList::get_docid());
}

Xapian::termcount
BrassModifiedPostList::get_doclength() const
{
    DEBUGCALL(DB, Xapian::termcount, "BrassModifiedPostList::get_doclength", "");
    if (it != mods.end() && (BrassPostList::at_end() || it->first <= BrassPostList::get_docid()))
	RETURN(this_db->get_doclength(it->first));
    RETURN(BrassPostList::get_doclength());
}

Xapian::termcount
BrassModifiedPostList::get_wdf() const
{
    if (BrassPostList::at_end()) return it->second.second;
    Xapian::docid unmod_did = BrassPostList::get_docid();
    if (it != mods.end() && it->first <= unmod_did) {
	if (it->first < unmod_did) return it->second.second;
	return BrassPostList::get_wdf() + it->second.second;
    }
    return BrassPostList::get_wdf();
}

PositionList *
BrassModifiedPostList::read_position_list()
{
    if (it != mods.end() && (BrassPostList::at_end() || it->first <= BrassPostList::get_docid())) {
	if (poslist) {
	    delete poslist;
	    poslist = NULL;
	}
	poslist = this_db->open_position_list(it->first, term);
	return poslist;
    }
    return BrassPostList::read_position_list();
}

PositionList *
BrassModifiedPostList::open_position_list() const
{
    if (it != mods.end() && (BrassPostList::at_end() || it->first <= BrassPostList::get_docid())) {
	return this_db->open_position_list(it->first, term);
    }
    return BrassPostList::open_position_list();
}

PostList *
BrassModifiedPostList::next(Xapian::weight w_min)
{
    if (have_started) {
	if (BrassPostList::at_end()) {
	    ++it;
	    skip_deletes(w_min);
	    return NULL;
	}
	Xapian::docid unmod_did = BrassPostList::get_docid();
	if (it != mods.end() && it->first <= unmod_did) {
	    if (it->first < unmod_did && it->second.first != 'D') {
		++it;
		skip_deletes(w_min);
		return NULL;
	    }
	    ++it;
	}
    }
    BrassPostList::next(w_min);
    skip_deletes(w_min);
    return NULL;
}

PostList *
BrassModifiedPostList::skip_to(Xapian::docid desired_did, Xapian::weight w_min)
{
    if (!BrassPostList::at_end()) BrassPostList::skip_to(desired_did, w_min);
    /* FIXME: should we use lower_bound() on the map? */
    while (it != mods.end() && it->first < desired_did) ++it;
    skip_deletes(w_min);
    return NULL;
}

bool
BrassModifiedPostList::at_end() const {
    return it == mods.end() && BrassPostList::at_end();
}

std::string
BrassModifiedPostList::get_description() const
{
    std::string desc = "BrassModifiedPostList(";
    desc += BrassPostList::get_description();
    desc += ')';
    return desc;
}
