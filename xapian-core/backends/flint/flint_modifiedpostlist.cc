/** @file flint_modifiedpostlist.cc
 * @brief A FlintPostList plus pending modifications
 */
/* Copyright (C) 2006,2007,2009 Olly Betts
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
#include "flint_modifiedpostlist.h"

#include "debuglog.h"
#include "flint_database.h"

FlintModifiedPostList::~FlintModifiedPostList()
{
    delete poslist;
}

void
FlintModifiedPostList::skip_deletes(Xapian::weight w_min)
{
    while (!FlintPostList::at_end()) {
	while (it != mods.end() && it->second.first == 'D' &&
	       it->first < FlintPostList::get_docid())
	    ++it;
	if (it == mods.end()) return;
	if (it->first != FlintPostList::get_docid()) return;
	if (it->second.first != 'D') return;
	++it;
	FlintPostList::next(w_min);
    }
    while (it != mods.end() && it->second.first == 'D') ++it;
}

Xapian::doccount
FlintModifiedPostList::get_termfreq() const
{
    return this_db->get_termfreq(term);
}

Xapian::docid
FlintModifiedPostList::get_docid() const
{
    if (it == mods.end()) return FlintPostList::get_docid();
    if (FlintPostList::at_end()) return it->first;
    Assert(it->second.first != 'D');
    return min(it->first, FlintPostList::get_docid());
}

Xapian::termcount
FlintModifiedPostList::get_doclength() const
{
    LOGCALL(DB, Xapian::termcount, "FlintModifiedPostList::get_doclength", NO_ARGS);
    if (it != mods.end() && (FlintPostList::at_end() || it->first <= FlintPostList::get_docid()))
	RETURN(this_db->get_doclength(it->first));
    RETURN(FlintPostList::get_doclength());
}

Xapian::termcount
FlintModifiedPostList::get_wdf() const
{
    if (FlintPostList::at_end()) return it->second.second;
    Xapian::docid unmod_did = FlintPostList::get_docid();
    if (it != mods.end() && it->first <= unmod_did) {
	if (it->first < unmod_did) return it->second.second;
	return it->second.second;
    }
    return FlintPostList::get_wdf();
}

PositionList *
FlintModifiedPostList::read_position_list()
{
    if (it != mods.end() && (FlintPostList::at_end() || it->first <= FlintPostList::get_docid())) {
	if (poslist) {
	    delete poslist;
	    poslist = NULL;
	}
	poslist = this_db->open_position_list(it->first, term);
	return poslist;
    }
    return FlintPostList::read_position_list();
}

PositionList *
FlintModifiedPostList::open_position_list() const
{
    if (it != mods.end() && (FlintPostList::at_end() || it->first <= FlintPostList::get_docid())) {
	return this_db->open_position_list(it->first, term);
    }
    return FlintPostList::open_position_list();
}

PostList *
FlintModifiedPostList::next(Xapian::weight w_min)
{
    if (have_started) {
	if (FlintPostList::at_end()) {
	    ++it;
	    skip_deletes(w_min);
	    return NULL;
	}
	Xapian::docid unmod_did = FlintPostList::get_docid();
	if (it != mods.end() && it->first <= unmod_did) {
	    if (it->first < unmod_did && it->second.first != 'D') {
		++it;
		skip_deletes(w_min);
		return NULL;
	    }
	    ++it;
	}
    }
    FlintPostList::next(w_min);
    skip_deletes(w_min);
    return NULL;
}

PostList *
FlintModifiedPostList::skip_to(Xapian::docid desired_did, Xapian::weight w_min)
{
    if (!FlintPostList::at_end()) FlintPostList::skip_to(desired_did, w_min);
    /* FIXME: should we use lower_bound() on the map? */
    while (it != mods.end() && it->first < desired_did) ++it;
    skip_deletes(w_min);
    return NULL;
}

bool
FlintModifiedPostList::at_end() const {
    return it == mods.end() && FlintPostList::at_end();
}

std::string
FlintModifiedPostList::get_description() const
{
    std::string desc = "FlintModifiedPostList(";
    desc += FlintPostList::get_description();
    desc += ')';
    return desc;
}
